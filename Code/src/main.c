/*
 * ============================================================================
 *                        VLF METALDETEKTOR - FIRMWARE
 * ============================================================================
 *
 * Kursus:  DTU 34621 - Indlejrede Systemer
 * Version: Test Version (før hardware integration)
 * Target:  Arduino Nano (ATmega328P @ 16MHz)
 *
 * ============================================================================
 *                              PIN KONFIGURATION
 * ============================================================================
 *
 *   Pin 9  (PB1) = TX signal output    - 2kHz firkantbølge til sendespole
 *   Pin A0 (PC0) = RX signal input     - Modtaget signal fra modtagerspole
 *   Pin A4 (PC4) = I2C SDA             - OLED display data
 *   Pin A5 (PC5) = I2C SCL             - OLED display clock
 *   Pin D4 (PD4) = Debug knap          - Skift mellem DFT og Debug skærm
 *
 *   TODO: Pin D2/D3 = Start/Stop knap  - Krav 9a
 *   TODO: Pin D3    = Kalibrering knap - Krav 9b
 *
 * ============================================================================
 *                              SIGNALFLOW
 * ============================================================================
 *
 *   1. Timer0 genererer 8kHz interrupt
 *   2. TX pin toggles hver 2. interrupt = 2kHz firkantbølge
 *   3. ADC auto-trigger sampler ved 8kHz (synkroniseret med Timer0)
 *   4. DFT akkumulerer 64 samples (8ms vindue, 16 TX cykler)
 *   5. Magnitude og fase beregnes fra Re/Im komponenter
 *   6. Resultater vises på OLED display
 *
 * ============================================================================
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "drivers/I2C.h"
#include "drivers/ssd1306.h"


/* ============================================================================
 *                              KONFIGURATION
 * ============================================================================ */

/* ----- Sampling & Frekvens ----- */
#define F_SAMPLE 8000           // Sample frekvens (Hz) - skal være 4x signal frekvens
#define F_SIGNAL 2000           // TX/RX signal frekvens (Hz)
#define N 64                    // Antal samples per DFT vindue

/* ----- DFT Koefficienter (4x oversampling) ----- */
#define RePhase1  1             // cos(0)
#define ImPhase2  1             // sin(Pi/2)
#define RePhase3 -1             // cos(Pi)
#define ImPhase4 -1             // sin(3Pi/2)

/* ----- ADC ----- */
#define ADC_middelvaerdi 512    // DC offset (mid-skala for 10-bit ADC)

/* ----- TODO: Metalklassificering Tærskler (Krav 2) ----- */
// #define FERRO_THRESHOLD -10  // Grader relativt til baseline
// #define NON_FERRO 0
// #define FERRO 1


/* ============================================================================
 *                           GLOBALE VARIABLE
 * ============================================================================ */

/* ----- TX/RX Variabler ----- */
static uint8_t i = 0;                       // Timer0 toggle tæller
volatile int16_t ADC_Raw = 0;               // Seneste ADC værdi (volatile: ISR → main)
volatile uint8_t rising_edge_Flag = 0;      // Flag: TX rising edge detekteret

/* ----- DFT Akkumulatorer ----- */
volatile uint8_t j = 0;                     // Sample tæller (0 til N-1)
volatile int32_t Re = 0;                    // Realdel akkumulator
volatile int32_t Im = 0;                    // Imaginærdel akkumulator
volatile int32_t Re_buff = 0;               // Realdel buffer (færdigt vindue)
volatile int32_t Im_buff = 0;               // Imaginærdel buffer (færdigt vindue)
volatile int16_t xn = 0;                    // Nuværende sample (DC fjernet)
volatile uint8_t DFT_done = 0;              // Flag: DFT vindue færdigt

/* ----- Resultater ----- */
volatile uint16_t mag = 0;                  // Magnitude
volatile int16_t ang = 0;                   // Fase i grader (-180 til +180)

/* ----- TODO: Kalibrering Variabler (Krav 9b) ----- */
// static int16_t baseline_mag = 0;
// static int16_t baseline_phase = 0;

/* ----- TODO: Detektor Tilstand (Krav 9a) ----- */
// static uint8_t detector_running = 1;


/* ============================================================================
 *                          FUNKTIONSPROTOTYPER
 * ============================================================================ */

void DFT_sum(int16_t ADC_Raw);


/* ============================================================================
 *                         HARDWARE INITIALISERING
 * ============================================================================ */

/*
 * Timer0 Initialisering
 * - CTC mode med 8kHz interrupt rate
 * - Bruges til TX signalgenerering og ADC trigger
 */
void timer0_init() {
    DDRB |= (1 << PB1);             // TX pin som output
    TCCR0A |= (1 << WGM01);         // CTC mode
    TCCR0B |= (1 << CS01);          // Prescaler 8
    OCR0A = 249;                    // 16MHz / 8 / 250 = 8kHz interrupt
    TIMSK0 = (1 << OCIE0A);         // Enable Timer0 Compare Match A interrupt
}

/*
 * ADC Initialisering
 * - Auto-trigger fra Timer0 Compare Match A
 * - 8kHz sampling rate synkroniseret med TX
 */
void adc_init() {
    /* Reference = AVCC (5V) */
    ADMUX = (1 << REFS0);

    /* ADC Kontrol: Enable, Interrupt, Auto-trigger, Prescaler 64 */
    ADCSRA = (1 << ADEN) | (1 << ADIE) | (1 << ADATE)
           | (1 << ADPS2) | (1 << ADPS1);   // TODO: Verificer prescaler

    /* Auto-trigger kilde = Timer0 Compare Match A */
    ADCSRB |= (1 << ADTS1) | (1 << ADTS0);

    /* Start første konvertering */
    ADCSRA |= (1 << ADSC);
}

/*
 * Knap Initialisering
 * - D4: Debug skærm toggle (eksisterende)
 * - TODO: D2/D3 til Start/Stop og Kalibrering
 */
void init_button(void) {
    /* Debug knap på D4 */
    DDRD &= ~(1 << PD4);            // Input
    PORTD |= (1 << PD4);            // Pull-up aktiveret

    /* TODO: Start/Stop knap (Krav 9a) */
    // DDRD &= ~(1 << PD2);
    // PORTD |= (1 << PD2);

    /* TODO: Kalibrering knap (Krav 9b) */
    // DDRD &= ~(1 << PD3);
    // PORTD |= (1 << PD3);
}


/* ============================================================================
 *                              INTERRUPTS
 * ============================================================================ */

/*
 * Timer0 Compare Match A Interrupt (8kHz)
 *
 * Formål:
 *   1. Generere 2kHz TX signal (toggle hver 2. interrupt)
 *   2. Sætte rising_edge_Flag for DFT synkronisering
 *
 * Timing:
 *   - ISR kører hver 125µs (8kHz)
 *   - TX toggle hver 250µs = 2kHz firkantbølge
 */
ISR(TIMER0_COMPA_vect) {
    i++;
    if (i >= 2) {
        PORTB ^= (1 << PB1);        // Toggle TX pin
        i = 0;

        /* Rising edge detektion */
        if (PORTB & (1 << PB1)) {
            rising_edge_Flag = 1;   // Start ny DFT periode
        }
    }
}

/*
 * ADC Konvertering Færdig Interrupt (8kHz)
 *
 * Formål:
 *   1. Læse ADC værdi (10-bit, 0-1023)
 *   2. Føde samples til DFT når synkroniseret
 */
ISR(ADC_vect) {
    ADC_Raw = ADC;

    if (rising_edge_Flag == 1) {
        DFT_sum(ADC_Raw);
    }
}


/* ============================================================================
 *                             DFT BEREGNING
 * ============================================================================ */

/*
 * DFT Sample Akkumulering
 *
 * Single-bin DFT ved 2kHz (bin 16 af 64-punkt DFT)
 * Udnytter 4x oversampling: koefficienter er kun +1, -1, 0
 *
 * Re = Σ x[n] * cos(2π * 16 * n / 64)
 * Im = Σ x[n] * (-sin(2π * 16 * n / 64))
 */
void DFT_sum(int16_t ADC_Raw) {

    xn = ADC_Raw - ADC_middelvaerdi;    // Fjern DC offset

    switch (j & 3) {    // Cykler 0,1,2,3,0,1,2,3...
        case 0:         // cos(0) = +1
            Re += RePhase1 * xn;
            break;
        case 1:         // -sin(π/2) = -1
            Im += -ImPhase2 * xn;
            break;
        case 2:         // cos(π) = -1
            Re += RePhase3 * xn;
            break;
        case 3:         // -sin(3π/2) = +1
            Im += -ImPhase4 * xn;
            break;
    }
    j++;

    /* Vindue færdigt - gem resultater */
    if (j >= N) {
        j = 0;
        Re_buff = Re;
        Im_buff = Im;
        DFT_done = 1;
        Re = 0;
        Im = 0;
        rising_edge_Flag = 0;
    }
}

/*
 * DFT Resultat Beregning
 *
 * Beregner magnitude og fase fra Re/Im komponenter
 * Kaldes fra hovedløkke når DFT_done == 1
 */
void DFT_Calc() {
    /* Magnitude: sqrt(Re² + Im²) / N */
    mag = sqrt((double)Re_buff * Re_buff + (double)Im_buff * Im_buff) / N;

    /* Fase: atan2(Im, Re) konverteret til grader */
    ang = (atan2(Im_buff, Re_buff)) * 57.2957795131;     // RAD_TO_DEG
}


/* ============================================================================
 *                          TODO: METALKLASSIFICERING
 * ============================================================================ */

/*
 * TODO: Implementer metaltype klassificering (Krav 2)
 *
 * void detect_metal_type() {
 *     int16_t delta_phase = ang - baseline_phase;
 *
 *     if (delta_phase < FERRO_THRESHOLD) {
 *         // Ferromagnetisk (jern/stål)
 *     } else {
 *         // Ikke-ferromagnetisk (kobber/messing/aluminium)
 *     }
 * }
 */


/* ============================================================================
 *                          TODO: IIR FILTER
 * ============================================================================ */

/*
 * TODO: Implementer IIR lavpas filter til display (Krav 8c)
 *
 * static float mag_filtered = 0;
 * static float ang_filtered = 0;
 * #define ALPHA 0.1
 *
 * void apply_iir_filter() {
 *     mag_filtered = ALPHA * mag + (1 - ALPHA) * mag_filtered;
 *     ang_filtered = ALPHA * ang + (1 - ALPHA) * ang_filtered;
 * }
 */


/* ============================================================================
 *                               DISPLAY
 * ============================================================================ */

static char buf[20];
static uint8_t show_debug = 0;

/*
 * Vis DFT resultater på OLED
 */
void display_dft(void) {
    sendStrXY("=== DFT ===     ", 0, 0);

    sprintf(buf, "Re:   %-9ld", Re_buff);
    sendStrXY(buf, 2, 0);

    sprintf(buf, "Im:   %-9ld", Im_buff);
    sendStrXY(buf, 3, 0);

    sprintf(buf, "Mag:  %-9d", mag);
    sendStrXY(buf, 5, 0);

    sprintf(buf, "Fase: %-9d", ang);
    sendStrXY(buf, 6, 0);

    /* TODO: Vis metaltype (Krav 2) */
    // sprintf(buf, "Metal: %s", metal_type == FERRO ? "FERRO" : "NON-FE");
    // sendStrXY(buf, 7, 0);
}

/*
 * Vis debug info på OLED
 */
void display_debug(void) {
    sendStrXY("=== DEBUG ===   ", 0, 0);

    sprintf(buf, "ADC:  %-9d", ADC_Raw);
    sendStrXY(buf, 2, 0);

    /* TODO: Vis flere debug værdier */
    // sprintf(buf, "Base: %-9d", baseline_phase);
    // sendStrXY(buf, 4, 0);
}


/* ============================================================================
 *                                 MAIN
 * ============================================================================ */

int main(void) {

    /* -------------------- Initialisering -------------------- */

    I2C_Init();
    InitializeDisplay();
    clear_display();
    timer0_init();
    adc_init();
    init_button();

    sei();      // VIGTIGT: Aktiver globale interrupts!

    /* Opstarts besked */
    sendStrXY("Starter...", 3, 2);
    _delay_ms(1000);
    clear_display();

    /* -------------------- Variable til hovedløkke -------------------- */

    uint8_t btn_prev = 1;           // Debug knap tidligere tilstand

    /* TODO: Start/Stop knap tilstand (Krav 9a) */
    // uint8_t start_btn_prev = 1;

    /* TODO: Kalibrering knap tilstand (Krav 9b) */
    // uint8_t cal_btn_prev = 1;

    /* -------------------- Hovedløkke -------------------- */

    while (1) {

        /* ----- DFT Resultat Håndtering ----- */
        if (DFT_done) {
            DFT_done = 0;
            DFT_Calc();

            /* TODO: Anvend IIR filter (Krav 8c) */
            // apply_iir_filter();

            /* TODO: Detekter metaltype (Krav 2) */
            // detect_metal_type();

            /* Opdater display */
            if (show_debug)
                display_debug();
            else
                display_dft();
        }

        /* ----- Debug Knap (D4) ----- */
        uint8_t btn = (PIND >> PD4) & 1;
        if (btn_prev && !btn) {             // Falling edge
            _delay_ms(50);                  // Debounce
            show_debug = !show_debug;
            clear_display();
        }
        btn_prev = btn;

        /* ----- TODO: Start/Stop Knap (Krav 9a) ----- */
        /*
        uint8_t start_btn = (PIND >> PD2) & 1;
        if (start_btn_prev && !start_btn) {
            _delay_ms(50);
            detector_running = !detector_running;
            // TODO: Enable/disable TX
            clear_display();
        }
        start_btn_prev = start_btn;
        */

        /* ----- TODO: Kalibrering Knap (Krav 9b) ----- */
        /*
        uint8_t cal_btn = (PIND >> PD3) & 1;
        if (cal_btn_prev && !cal_btn) {
            _delay_ms(50);
            baseline_mag = mag;
            baseline_phase = ang;
            sendStrXY("KALIBRERET!     ", 7, 0);
        }
        cal_btn_prev = cal_btn;
        */

        _delay_ms(50);
    }
}
