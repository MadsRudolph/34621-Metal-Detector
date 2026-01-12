/*
 * VLF Metal Detector Firmware
 * DTU 34621 - Arduino Nano (ATmega328P)
 *
 * Pins: D9=TX, A0=RX, A4/A5=I2C, D4=Button
 *
 * Saet CAPTURE_SAMPLES=1 for MATLAB DFT verifikation
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

/* Capture mode: Når CAPTURE_SAMPLES=1 gemmes samples og sendes via UART til MATLAB */
#define CAPTURE_SAMPLES 1
#if CAPTURE_SAMPLES
#include "capture.h"
#endif


/* ============================================================================
 * KONFIGURATION
 * ============================================================================
 * F_SAMPLE: ADC sample frekvens (8000 Hz)
 * F_SIGNAL: TX sender frekvens (2000 Hz) - dette er metaldetektorens arbejdsfrekvens
 * N:        Antal samples per DFT beregning (64 samples = 8ms ved 8kHz)
 *
 * Forhold: F_SAMPLE / F_SIGNAL = 8000/2000 = 4 samples per TX periode
 *          Dette giver præcis 90 graders faseforskydning mellem samples
 *          hvilket udnyttes i DFT beregningen (se dft_sum)
 */
#define F_SAMPLE 8000
#define F_SIGNAL 2000
#define N 64
#define ADC_OFFSET 512      /* Midtpunkt for 10-bit ADC (0-1023) -> centrerer signal omkring 0 */
#define UART_BAUD 115200


/* ============================================================================
 * GLOBALE VARIABLE
 * ============================================================================
 * 'volatile' bruges når variablen ændres i interrupts - fortæller compileren
 * at værdien kan ændre sig uventet, så den ikke optimerer læsninger væk
 */

/* TX/RX Synkronisering */
static uint8_t tx_count = 0;      /* Tæller til at generere 2kHz fra 8kHz timer */
volatile int16_t adc_raw = 0;     /* Seneste ADC aflæsning (0-1023) */
volatile uint8_t sync_flag = 0;   /* Flag: 1 = start ny DFT ved næste rising edge af TX */

/* DFT Akkumulatorer
 * Re/Im: Løbende summer for real og imaginær del
 * Re_buf/Im_buf: Færdige resultater fra sidste DFT blok (kopieres når N samples er talt)
 */
volatile uint8_t dft_index = 0;           /* Sample nummer i aktuel DFT blok (0 til N-1) */
volatile int32_t Re = 0, Im = 0;          /* Akkumulatorer under beregning */
volatile int32_t Re_buf = 0, Im_buf = 0;  /* Færdige DFT resultater */
volatile uint8_t dft_done = 0;            /* Flag: 1 = ny DFT klar til beregning */

/* Beregnede Resultater */
volatile uint16_t mag = 0;    /* Magnitude (styrke) af 2kHz komponenten */
volatile int16_t phase = 0;   /* Fase i grader (-180 til +180) */

/* Display */
static char buf[20];          /* Buffer til sprintf formatering */
static uint8_t show_debug = 0;/* Toggle mellem DFT visning og debug visning */


/* ============================================================================
 * FUNKTIONS PROTOTYPER
 * ============================================================================ */
void dft_sum(int16_t raw);
void dft_calc(void);
void display_dft(void);
void display_debug(void);


/* ============================================================================
 * HARDWARE INITIALISERING
 * ============================================================================ */

/*
 * Timer0 Initialisering - Genererer 8kHz interrupt til sampling og 2kHz TX signal
 *
 * Timer0 er en 8-bit tæller der kører i CTC mode (Clear Timer on Compare match)
 *
 * Beregning af timing:
 *   F_CPU = 16 MHz (Arduino Nano krystal)
 *   Prescaler = 8 (CS01 bit sat)
 *   OCR0A = 249
 *
 *   Interrupt frekvens = F_CPU / (Prescaler * (OCR0A + 1))
 *                      = 16,000,000 / (8 * 250)
 *                      = 8000 Hz
 *
 *   TX frekvens = 8000 Hz / 4 = 2000 Hz (toggle hver 2. interrupt, se ISR)
 */
void timer0_init(void) {
    DDRB |= (1 << PB1);       /* PB1 (Arduino D9) som output til TX signal */
    TCCR0A = (1 << WGM01);    /* CTC mode: nulstil timer når TCNT0 == OCR0A */
    TCCR0B = (1 << CS01);     /* Prescaler = 8 */
    OCR0A = 249;              /* Compare match værdi (0-249 = 250 ticks) */
    TIMSK0 = (1 << OCIE0A);   /* Enable Compare Match A interrupt */
}

/*
 * ADC Initialisering - Automatisk triggered sampling synkroniseret med Timer0
 *
 * ADC'en samples automatisk hver gang Timer0 Compare Match A sker (8kHz)
 * Dette sikrer præcis timing mellem TX signal og RX sampling
 *
 * ADC Clock: 16MHz / 64 = 250kHz (ADPS2:1 = 110)
 * En conversion tager 13 ADC clock cycles = 52us
 * Ved 8kHz sampling (125us mellem samples) er der god margin
 */
void adc_init(void) {
    ADMUX = (1 << REFS0);     /* Reference = AVcc (5V), input = ADC0 (Arduino A0) */

    ADCSRA = (1 << ADEN)      /* Enable ADC */
           | (1 << ADIE)      /* Enable ADC interrupt (kald ISR når conversion done) */
           | (1 << ADATE)     /* Auto Trigger Enable - start ny conversion automatisk */
           | (1 << ADPS2) | (1 << ADPS1);  /* Prescaler = 64 -> 250kHz ADC clock */

    ADCSRB = (1 << ADTS1) | (1 << ADTS0);  /* Trigger source = Timer0 Compare Match A */

    ADCSRA |= (1 << ADSC);    /* Start første conversion (derefter sker det automatisk) */
}

/*
 * Knap Initialisering - D4 som input med intern pull-up
 *
 * Knappen forbinder D4 til GND når den trykkes
 * Pull-up modstanden holder D4 høj (1) når knappen IKKE er trykket
 * Når knappen trykkes: D4 = 0
 */
void button_init(void) {
    DDRD &= ~(1 << PD4);      /* PD4 (Arduino D4) som input (clear bit) */
    PORTD |= (1 << PD4);      /* Enable intern pull-up modstand */
}


/* ============================================================================
 * INTERRUPT SERVICE ROUTINES (ISR)
 * ============================================================================
 * Interrupts afbryder hovedprogrammet og kører denne kode med det samme
 * Vigtig: Hold ISR'er korte! Lang kode her blokerer andre interrupts
 */

/*
 * Timer0 Compare Match Interrupt - Kører 8000 gange per sekund
 *
 * Opgave 1: Generer 2kHz TX signal på D9
 *   - tx_count tæller 0,1,0,1,0,1...
 *   - Toggle D9 hver 2. gang -> 8000/4 = 2000 Hz firkantbølge
 *
 * Opgave 2: Synkroniser DFT med TX rising edge
 *   - sync_flag sættes når D9 går HØJ
 *   - Dette starter DFT akkumulering ved starten af TX periode
 */
ISR(TIMER0_COMPA_vect) {
    if (++tx_count >= 2) {              /* Hver 2. interrupt (4000 Hz toggle rate) */
        PORTB ^= (1 << PB1);            /* Toggle PB1 (D9) - XOR med 1 flipper bit */
        tx_count = 0;
        if (PORTB & (1 << PB1)) {       /* Hvis D9 lige gik HØJ (rising edge) */
            sync_flag = 1;              /* Signal til ADC ISR: start DFT */
        }
    }
}

/*
 * ADC Conversion Complete Interrupt - Kører når hver ADC sampling er færdig
 *
 * Kaldes automatisk 8000 gange per sekund (triggered af Timer0)
 * Læs ADC værdi og send til DFT hvis synkroniseret
 */
ISR(ADC_vect) {
    adc_raw = ADC;                      /* Læs 10-bit ADC resultat (0-1023) */
    if (sync_flag) {                    /* Kun akkumuler hvis synkroniseret med TX */
        dft_sum(adc_raw);
    }
}


/* ============================================================================
 * DFT (Diskret Fourier Transform) BEREGNING
 * ============================================================================
 *
 * Vi beregner DFT KUN ved 2kHz (vores TX frekvens) - ikke fuld FFT
 * Dette er meget hurtigere og giver os præcis det vi skal bruge:
 *   - Magnitude: Hvor stærkt er 2kHz signalet?
 *   - Fase: Hvad er faseforskydningen i forhold til TX?
 *
 * Matematisk baggrund:
 *   DFT ved bin k: X[k] = sum(x[n] * e^(-j*2*pi*k*n/N))
 *                       = sum(x[n] * (cos(2*pi*k*n/N) - j*sin(2*pi*k*n/N)))
 *
 *   For k=16 (2kHz bin ved N=64, Fs=8kHz):
 *   cos/sin værdierne gentager sig med periode 4 samples:
 *     n=0: cos=1,  sin=0   -> Re += x, Im += 0
 *     n=1: cos=0,  sin=1   -> Re += 0, Im -= x
 *     n=2: cos=-1, sin=0   -> Re -= x, Im += 0
 *     n=3: cos=0,  sin=-1  -> Re += 0, Im += x
 *
 *   Dette mønster gentages for alle 64 samples!
 *   Derfor kan vi undgå sin/cos beregninger helt.
 */

/*
 * dft_sum - Akkumuler en sample til DFT beregning
 *
 * Kaldes fra ADC interrupt for hver sample (8000 Hz)
 * Bruger switch på (index mod 4) til at vælge +/- Re/Im
 *
 * Parameter: raw = ADC værdi (0-1023)
 */
void dft_sum(int16_t raw) {
    /* Centrer omkring 0: ADC giver 0-1023, vi vil have -512 til +511 */
    int16_t x = raw - ADC_OFFSET;

#if CAPTURE_SAMPLES
    /* Capture mode: Gem samples til MATLAB analyse */
    uint8_t state = capture_get_state();
    if (dft_index == 0 && state == CAPTURE_WAITING) {
        capture_set_state(CAPTURE_ACTIVE);  /* Start capture ved DFT blok start */
    }
    capture_store_sample(dft_index, x);
#endif

    /*
     * Akkumuler til Re og Im baseret på sample position
     * (dft_index & 3) er hurtig modulo 4 operation (bitwise AND)
     *
     * Dette implementerer: sum(x[n] * cos(pi*n/2)) for Re
     *                      sum(x[n] * -sin(pi*n/2)) for Im
     */
    switch (dft_index & 3) {
        case 0: Re += x;  break;  /* cos=+1, sin=0  */
        case 1: Im -= x;  break;  /* cos=0,  sin=+1 -> -sin=-1 */
        case 2: Re -= x;  break;  /* cos=-1, sin=0  */
        case 3: Im += x;  break;  /* cos=0,  sin=-1 -> -sin=+1 */
    }

    /* Check om vi har samlet N samples */
    if (++dft_index >= N) {
        /* Kopier resultater til buffer (main loop læser derfra) */
        Re_buf = Re;
        Im_buf = Im;

        /* Nulstil akkumulatorer til næste blok */
        Re = Im = 0;
        dft_index = 0;

        /* Signal til main loop at nye data er klar */
        dft_done = 1;

        /* Stop akkumulering indtil næste TX rising edge */
        sync_flag = 0;

#if CAPTURE_SAMPLES
        if (capture_get_state() == CAPTURE_ACTIVE) {
            capture_set_state(CAPTURE_DONE);  /* Marker capture som færdig */
        }
#endif
    }
}

/*
 * dft_calc - Beregn magnitude og fase fra Re/Im
 *
 * Kaldes fra main loop når dft_done flag er sat
 * Bruger floating point - dette er langsomt på AVR men OK da det
 * kun sker 125 gange per sekund (8000/64)
 *
 * Magnitude = sqrt(Re^2 + Im^2) / N
 * Fase = atan2(Im, Re) * (180/pi)
 */
void dft_calc(void) {
    double re = Re_buf, im = Im_buf;     /* Konverter til double for beregning */
    mag = sqrt(re*re + im*im) / N;       /* Normaliser med N for at få amplitude */
    phase = atan2(im, re) * 57.2957795;  /* Konverter radianer til grader (180/pi) */
}


/* ============================================================================
 * DISPLAY FUNKTIONER
 * ============================================================================
 * Viser DFT resultater på SSD1306 OLED display (128x64 pixels)
 * Display er forbundet via I2C (A4=SDA, A5=SCL)
 */

/* Vis DFT resultater: Re, Im, Magnitude, Fase */
void display_dft(void) {
    sendStrXY("=== DFT ===     ", 0, 0);
    sprintf(buf, "Re:  %-10ld", Re_buf);  sendStrXY(buf, 2, 0);
    sprintf(buf, "Im:  %-10ld", Im_buf);  sendStrXY(buf, 3, 0);
    sprintf(buf, "Mag: %-10u", mag);      sendStrXY(buf, 5, 0);
    sprintf(buf, "Pha: %-10d", phase);    sendStrXY(buf, 6, 0);
}

/* Vis debug info: rå ADC værdi */
void display_debug(void) {
    sendStrXY("=== DEBUG ===   ", 0, 0);
    sprintf(buf, "ADC: %-10d", adc_raw);  sendStrXY(buf, 2, 0);
}


/* ============================================================================
 * HOVEDPROGRAM
 * ============================================================================ */

int main(void) {
    /* ---- Initialisering ---- */
    I2C_Init();               /* Start I2C bus til OLED kommunikation */
    InitializeDisplay();      /* Konfigurer SSD1306 OLED */
    clear_display();          /* Ryd skærm */

    timer0_init();            /* Start 8kHz timer og 2kHz TX signal */
    adc_init();               /* Start ADC med auto-trigger */
    button_init();            /* Konfigurer knap på D4 */

#if CAPTURE_SAMPLES
    /* Capture mode: Initialiser UART til PC kommunikation */
    capture_init(UART_BAUD);
    sendStrXY("CAPTURE MODE", 2, 2);
    _delay_ms(500);
    capture_send_ready();     /* Send "READY" til PC så MATLAB ved vi er klar */
#else
    sendStrXY("Starting...", 3, 2);
    _delay_ms(1000);
#endif

    clear_display();
    sei();                    /* Enable globale interrupts - nu kører systemet! */

    uint8_t btn_prev = 1;     /* Forrige knap tilstand (1 = ikke trykket pga pull-up) */

    /* ---- Hovedløkke ---- */
    while (1) {
        /*
         * Behandl DFT resultat når en ny blok er klar
         * dft_done sættes i ISR, vi nulstiller det her for at undgå race condition
         */
        if (dft_done) {
            dft_done = 0;                 /* Nulstil FØR beregning */
            dft_calc();                   /* Beregn magnitude og fase */
            show_debug ? display_debug() : display_dft();  /* Opdater display */
        }

#if CAPTURE_SAMPLES
        /* Check om PC har sendt kommando via UART */
        capture_check_serial();

        /* Send captured data til PC når capture er færdig */
        if (capture_get_state() == CAPTURE_DONE) {
            capture_output(Re_buf, Im_buf, mag, phase);
            capture_set_state(CAPTURE_IDLE);
            sendStrXY("Sent!           ", 7, 0);
        }
#endif

        /*
         * Knap håndtering med edge detection
         * Vi reagerer kun på FALLING edge (knap trykkes ned)
         * btn_prev && !btn: forrige gang høj OG nu lav = netop trykket
         */
        uint8_t btn = (PIND >> PD4) & 1;  /* Læs D4 (0=trykket, 1=ikke trykket) */
        if (btn_prev && !btn) {           /* Falling edge detected */
            _delay_ms(50);                /* Simpel debounce */
#if CAPTURE_SAMPLES
            /* I capture mode: Tryk starter ny capture */
            if (capture_get_state() == CAPTURE_IDLE) {
                capture_set_state(CAPTURE_WAITING);
            }
#else
            /* Normal mode: Tryk skifter mellem DFT og debug visning */
            show_debug = !show_debug;
            clear_display();
#endif
        }
        btn_prev = btn;

        _delay_ms(50);  /* Reducer CPU load - display opdateres max 20 gange/sek */
    }
}
