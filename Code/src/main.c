/*
 * Metal Detektor - Test Version
 *
 * Hardware:
 *   Pin 9  = TX signal (2kHz firkantbølge til sendespole)
 *   Pin A0 = RX signal (modtaget signal fra modtagerspole)
 *   Pin A4 = I2C SDA (OLED display)
 *   Pin A5 = I2C SCL (OLED display)
 *   Pin D4 = Knap (skift mellem DFT og Debug skærm)
 *
 * Hvordan det virker:
 *   1. Timer0 genererer 8kHz interrupt og toggler TX pin hver 2. gang = 2kHz
 *   2. ADC sampler automatisk ved hver Timer0 interrupt = 8kHz sampling
 *   3. DFT akkumulerer 64 samples og beregner magnitude og fase
 *   4. Resultater vises på OLED display
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>

#include "drivers/I2C.h"
#include "drivers/ssd1306.h"

/* ============ INDSTILLINGER ============ */

#define F_SAMPLE 8000       // Sample frekvens (Hz) - skal være 4x signal frekvens
#define F_SIGNAL 2000       // TX/RX signal frekvens (Hz)
#define N 64                // Antal samples per DFT vindue (flere = mere præcist, men langsommere)

/* ============ GLOBALE VARIABLE ============ */

// DFT akkumulatorer - bruges i interrupt til at summere samples
volatile int32_t Re = 0, Im = 0;

// DFT resultater - kopieres fra akkumulatorer når vindue er færdigt
volatile int32_t Re_result = 0, Im_result = 0;

// Sample tæller - holder styr på hvor vi er i DFT vinduet
volatile uint8_t sample_index = 0;

// Flag der sættes når DFT vindue er færdigt og klar til beregning
volatile uint8_t DFT_ready = 0;

// Beregnede resultater
uint16_t magnitude = 0;     // Styrke af signalet
int16_t phase_deg = 0;      // Fase i grader (-180 til +180)

// Debug variable - til at tjekke ADC værdier
volatile uint16_t adc_value = 0;
volatile uint16_t adc_min = 1023, adc_max = 0;

/* ============ HARDWARE INIT ============ */

void init_tx(void) {
    // Sæt Pin 9 (PB1) som output til TX signal
    DDRB |= (1 << PB1);

    // Timer0 i CTC mode - tæller op til OCR0A og resetter
    TCCR0A = (1 << WGM01);

    // Prescaler 8: 16MHz / 8 = 2MHz timer clock
    TCCR0B = (1 << CS01);

    // Interrupt ved 250 ticks: 2MHz / 250 = 8kHz interrupt rate
    OCR0A = 249;

    // Aktiver Timer0 Compare Match A interrupt
    TIMSK0 = (1 << OCIE0A);
}

void init_adc(void) {
    // Vælg AVCC (5V) som reference og ADC kanal 0 (pin A0)
    ADMUX = (1 << REFS0);

    // ADC indstillinger:
    // ADEN  = Aktiver ADC
    // ADIE  = Aktiver ADC interrupt når conversion er færdig
    // ADATE = Auto-trigger mode (starter automatisk ved trigger event)
    // ADPS  = Prescaler 128 (16MHz / 128 = 125kHz ADC clock)
    ADCSRA = (1 << ADEN) | (1 << ADIE) | (1 << ADATE)
           | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

    // Trigger source = Timer0 Compare Match A
    // ADC starter automatisk hver gang Timer0 matcher OCR0A
    ADCSRB = (1 << ADTS1) | (1 << ADTS0);

    // Start første conversion (derefter kører det automatisk)
    ADCSRA |= (1 << ADSC);
}

void init_button(void) {
    // Pin D4 som input
    DDRD &= ~(1 << PD4);
    // Aktiver intern pull-up modstand (knap forbinder til GND)
    PORTD |= (1 << PD4);
}

/* ============ INTERRUPTS ============ */

// TX tæller - bruges til at toggle TX pin hver 2. interrupt
static uint8_t tx_counter = 0;

// Timer0 interrupt - kører 8000 gange per sekund
ISR(TIMER0_COMPA_vect) {
    tx_counter++;
    if (tx_counter >= 2) {
        // Toggle TX pin hver 2. interrupt = 4000 toggles/sek = 2kHz firkantbølge
        PORTB ^= (1 << PB1);
        tx_counter = 0;
    }
}

// ADC interrupt - kører hver gang en ADC conversion er færdig (8kHz)
ISR(ADC_vect) {
    // Læs ADC værdi (0-1023) og fjern DC offset (512 = midtpunkt)
    // Dette giver en signed værdi fra -512 til +511
    int16_t sample = ADC - 512;

    // Debug: gem ADC værdier til debug skærm
    adc_value = ADC;
    if (ADC < adc_min) adc_min = ADC;
    if (ADC > adc_max) adc_max = ADC;

    /*
     * DFT BEREGNING - dette er kernen i metal detektoren!
     *
     * Vi sampler 4 gange per TX periode (8kHz / 2kHz = 4).
     * Ved 4x oversampling bliver DFT koefficienterne simple:
     *
     *   Sample 0: fase =   0° → cos = +1, sin =  0 → Re += sample
     *   Sample 1: fase =  90° → cos =  0, sin = +1 → Im -= sample
     *   Sample 2: fase = 180° → cos = -1, sin =  0 → Re -= sample
     *   Sample 3: fase = 270° → cos =  0, sin = -1 → Im += sample
     *
     * Efter N samples har vi akkumuleret Re og Im komponenterne.
     * Disse bruges til at beregne magnitude og fase.
     */
    switch (sample_index % 4) {
        case 0: Re += sample; break;
        case 1: Im -= sample; break;
        case 2: Re -= sample; break;
        case 3: Im += sample; break;
    }

    sample_index++;

    // Når vi har samlet N samples, er DFT vinduet færdigt
    if (sample_index >= N) {
        Re_result = Re;
        Im_result = Im;
        DFT_ready = 1;
        Re = 0;
        Im = 0;
        sample_index = 0;
    }
}

/* ============ DFT BEREGNING ============ */

void calc_magnitude_phase(void) {
    double re = (double)Re_result;
    double im = (double)Im_result;

    magnitude = (uint16_t)(sqrt(re*re + im*im) / 16.0);
    phase_deg = (int16_t)(atan2(im, re) * 57.2957795131);
}

/* ============ DISPLAY ============ */

static char buf[20];
static uint8_t show_debug = 0;

void display_dft(void) {
    sendStrXY("=== DFT ===     ", 0, 0);

    sprintf(buf, "Re:   %-9ld", Re_result);
    sendStrXY(buf, 2, 0);

    sprintf(buf, "Im:   %-9ld", Im_result);
    sendStrXY(buf, 3, 0);

    sprintf(buf, "Mag:  %-9u", magnitude);
    sendStrXY(buf, 5, 0);

    sprintf(buf, "Fase: %d", phase_deg);
    sendStrXY(buf, 6, 0);
}

void display_debug(void) {
    sendStrXY("=== DEBUG ===   ", 0, 0);

    sprintf(buf, "ADC:  %-9u", adc_value);
    sendStrXY(buf, 2, 0);

    sprintf(buf, "Min:  %-9u", adc_min);
    sendStrXY(buf, 3, 0);

    sprintf(buf, "Max:  %-9u", adc_max);
    sendStrXY(buf, 4, 0);

    sprintf(buf, "Vpp:  %-9u", adc_max - adc_min);
    sendStrXY(buf, 5, 0);
}

/* ============ MAIN ============ */

int main(void) {
    // Initialiser hardware
    I2C_Init();
    InitializeDisplay();
    clear_display();
    init_tx();
    init_adc();
    init_button();

    // VIGTIGT: Aktiver globale interrupts!
    sei();

    // Vis opstarts besked
    sendStrXY("Starter...", 3, 2);
    _delay_ms(1000);
    clear_display();

    // Knap state til edge detection
    uint8_t btn_prev = 1;

    // Hovedløkke
    while (1) {
        // Når DFT vindue er færdigt, beregn og vis resultater
        if (DFT_ready) {
            calc_magnitude_phase();
            DFT_ready = 0;

            if (show_debug)
                display_debug();
            else
                display_dft();
        }

        // Tjek knap D4 - skift mellem DFT og Debug skærm
        uint8_t btn = (PIND >> PD4) & 1;
        if (btn_prev && !btn) {         // Falling edge = knap trykket
            _delay_ms(50);              // Debounce
            show_debug = !show_debug;
            clear_display();
        }
        btn_prev = btn;

        _delay_ms(50);
    }
}
