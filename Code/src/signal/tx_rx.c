/*
 * tx_rx.c
 * TX signal generation + ADC sampling for Arduino Nano (ATmega328P)
 *
 * Generates 2 kHz square wave on Pin 9 (PB1)
 * Samples ADC at 8 kHz (4x oversampling) on A0
 *
 * ÆNDRING: Fjernet config.h - alle pin definitioner er nu direkte i denne fil.
 *
 * ATmega328P Datasheet Reference (Rev. 7810D–AVR–01/15):
 * ======================================================
 * Timer/Counter0:
 *   - TCCR0A:      Section 14.9.1, Page 84
 *   - TCCR0B:      Section 14.9.2, Page 86
 *   - OCR0A:       Section 14.9.4, Page 87
 *   - TIMSK0:      Section 14.9.6, Page 88
 *   - CTC Mode:    Section 14.7.2, Page 79
 *   - WGM bits:    Table 14-8, Page 86
 *   - CS bits:     Table 14-9, Page 87
 *
 * ADC:
 *   - ADMUX:       Section 23.9.1, Page 217
 *   - ADCSRA:      Section 23.9.2, Page 218
 *   - ADCSRB:      Section 23.9.4, Page 220
 *   - ADC Data:    Section 23.9.3, Page 219
 *   - Timing:      Section 23.4, Page 210
 *
 * I/O Ports:
 *   - DDRB:        Section 13.4.3, Page 72
 *   - PORTB:       Section 13.4.2, Page 72
 *
 * Interrupts:
 *   - Vectors:     Section 11.1, Page 49, Table 11-1
 */

#include "tx.h"
#include "dsp.h"
#include <avr/io.h>
#include <avr/interrupt.h>

/*
 * PIN OVERSIGT - Arduino Nano (ATmega328P)
 *
 * TX Signal: Pin 9 = PB1 (PORTB bit 1)
 * RX Input:  Pin A0 = ADC kanal 0
 */

/*
 * ============================================================================
 * FEJL I ORIGINAL TIMER KONFIGURATION:
 * ============================================================================
 *
 * ORIGINAL KODE:
 *   void timer1_init() {
 *       DDRB |=(1<<PB1);
 *       TCCR0A |= (1<<WGM01);  // CTC mode
 *       TCCR0B |= (1<<CS01);   // Prescaler 8
 *       OCR0A = 124;           // Compare value
 *       TIMSK0 = (1<< TOIE0);  // Overflow interrupt
 *   }
 *
 * PROBLEMER:
 *
 * 1. FUNKTIONSNAVNET SIGER TIMER1, MEN BRUGER TIMER0:
 *    - Forvirrende og inkonsistent med dokumentation
 *
 * 2. FORKERT INTERRUPT AKTIVERING:
 *    - TIMSK0 = (1 << TOIE0) aktiverer OVERFLOW interrupt
 *    - Men i CTC mode skal vi bruge COMPARE MATCH interrupt (OCIE0A)
 *    - I CTC mode resetter TCNT0 til 0 ved compare match, IKKE ved overflow
 *    - Overflow interrupt fyrer kun når TCNT0 går fra 255 til 0 (sker aldrig i CTC!)
 *
 * 3. FORKERT FREKVENS BEREGNING:
 *    - Kommentaren siger "f_OCnA = fclk/(2*N*(1+OCRnA))"
 *    - Den formel er for TOGGLE mode på OC0A pin
 *    - Men koden bruger manuel toggle i ISR med "if(i >= 1)" divider
 *    - Lad os beregne hvad vi faktisk får:
 *
 *    Med CTC mode og OCIE0A (compare match interrupt):
 *    f_interrupt = 16MHz / (8 * (124 + 1)) = 16MHz / 1000 = 16 kHz
 *
 *    ISR toggler hver 2. gang (i >= 1):
 *    f_toggle = 16 kHz / 2 = 8 kHz
 *
 *    Square wave frekvens = toggle / 2:
 *    f_TX = 8 kHz / 2 = 4 kHz  <-- FORKERT! Vi vil have 2 kHz!
 *
 * 4. ADC SAMPLE RATE:
 *    - ADC triggers på Timer0 overflow (ADTS2 = 1)
 *    - Men i CTC mode er der INGEN overflow events!
 *    - ADC sampling virker slet ikke med denne konfiguration!
 *
 * ============================================================================
 * LØSNING:
 * ============================================================================
 *
 * For at få 2 kHz TX og 8 kHz ADC sampling:
 *
 * METODE: Brug Timer0 i CTC mode med compare match interrupt
 *
 *   f_interrupt = 16MHz / (prescaler * (OCR0A + 1))
 *
 *   Vi vil have 8 kHz interrupt (til ADC sampling):
 *   8000 = 16000000 / (prescaler * (OCR0A + 1))
 *   prescaler * (OCR0A + 1) = 2000
 *
 *   Med prescaler = 8:  OCR0A + 1 = 250  =>  OCR0A = 249
 *   Med prescaler = 64: OCR0A + 1 = 31.25 (ikke heltal, kan ikke bruges)
 *
 *   Så: Prescaler = 8, OCR0A = 249 giver 8 kHz interrupt
 *
 *   TX toggle hver 4. interrupt = 2 kHz TX signal
 *   ADC sample hver interrupt = 8 kHz sampling
 *
 * ============================================================================
 */

static volatile uint8_t tx_counter = 0;    /* Tæller for TX toggle (0-3) */
static volatile uint8_t sampling_enabled = 0;  /* Flag for ADC sampling */

void timer0_init(void) {
    /*
     * ÆNDRING: Fjernet #if/#elif conditionals for Mega/Nano.
     * Bruger nu config.h defines direkte - kun Nano support.
     *
     * ORIGINAL KODE:
     *   #if defined(__AVR_ATmega328P__)
     *       DDRB |= (1 << PB1);     // Pin 9 på Nano
     *   #elif defined(__AVR_ATmega2560__)
     *       DDRB |= (1 << PB5);     // Pin 11 på Mega
     *   #endif
     */

    /* Konfigurer TX pin som output - Pin 9 (PB1)
     * Datasheet: Section 13.4.3, Page 72 - DDRB register */
    DDRB |= (1 << PB1);

    /*
     * Timer0 i CTC mode
     * WGM01 = 1, WGM00 = 0: CTC mode (Clear Timer on Compare Match)
     * Datasheet: Section 14.9.1, Page 84 - TCCR0A register
     * Datasheet: Table 14-8, Page 86 - Waveform Generation Mode
     */
    TCCR0A = (1 << WGM01);

    /*
     * Prescaler = 8
     * CS01 = 1: clk/8
     * Datasheet: Section 14.9.2, Page 86 - TCCR0B register
     * Datasheet: Table 14-9, Page 87 - Clock Select Bit Description
     */
    TCCR0B = (1 << CS01);

    /*
     * OCR0A = 249 giver:
     * f_interrupt = 16MHz / (8 * 250) = 8 kHz
     * Datasheet: Section 14.9.4, Page 87 - OCR0A register
     * Datasheet: Section 14.7.2, Page 79 - CTC Mode frequency formula
     */
    OCR0A = 249;

    /*
     * KORREKT: Aktiver Compare Match A interrupt (IKKE overflow!)
     * OCIE0A = 1: Interrupt når TCNT0 == OCR0A
     * Datasheet: Section 14.9.6, Page 88 - TIMSK0 register
     */
    TIMSK0 = (1 << OCIE0A);
}

void adc_init(void) {
    /*
     * ========================================================================
     * FEJL I ORIGINAL ADC KONFIGURATION:
     * ========================================================================
     *
     * ORIGINAL KODE:
     *   ADCSRB |= (1 << ADTS2);  // Auto-trigger kilde = Timer0 Overflow
     *
     * PROBLEM:
     * - ADTS2:0 = 100 (binary) = Timer0 Overflow
     * - Men i CTC mode er der INGEN overflow! Timer resetter ved compare match.
     * - ADC vil ALDRIG trigge automatisk med denne opsætning.
     *
     * LØSNING:
     * - Vi bruger IKKE auto-trigger
     * - I stedet starter vi ADC manuelt i Timer0 Compare Match ISR
     * - Dette giver os præcis kontrol over sampling timing
     * ========================================================================
     */

    /*
     * Reference = AVCC (5V), kanal 0 (A0)
     * Datasheet: Section 23.9.1, Page 217 - ADMUX register
     * Datasheet: Table 23-3, Page 217 - Voltage Reference Selections
     * Datasheet: Table 23-4, Page 218 - Input Channel Selection
     */
    ADMUX = (1 << REFS0);

    /*
     * ADC Kontrol:
     * ADEN  = Aktiver ADC
     * ADIE  = Aktiver ADC interrupt
     * ADATE = DEAKTIVERET (vi trigger manuelt fra Timer ISR)
     * ADPS  = Prescaler 128 (125kHz ADC clock)
     *
     * ADC conversion tid = 13 cycles / 125kHz = 104µs
     * Sample interval = 125µs (8kHz)
     * 104µs < 125µs - vi når det!
     *
     * Datasheet: Section 23.9.2, Page 218 - ADCSRA register
     * Datasheet: Table 23-5, Page 219 - ADC Prescaler Selections
     * Datasheet: Section 23.4, Page 210 - Prescaler and Conversion Timing
     */
    ADCSRA = (1 << ADEN) | (1 << ADIE)
           | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

    /*
     * FJERNET auto-trigger konfiguration:
     * ADCSRB |= (1 << ADTS2);  // Virker ikke med CTC mode!
     *
     * ADCSRB = 0 (default) er fint når vi ikke bruger auto-trigger
     * Datasheet: Section 23.9.4, Page 220 - ADCSRB register
     */
    ADCSRB = 0;
}

/*
 * Start ADC sampling
 * Kaldes fra main() efter initialisering
 */
void sampling_start(void) {
    sampling_enabled = 1;
    /* Start første konvertering
     * Datasheet: Section 23.9.2, Page 218 - ADSC bit starter konvertering */
    ADCSRA |= (1 << ADSC);
}

/*
 * Stop ADC sampling
 */
void sampling_stop(void) {
    sampling_enabled = 0;
}

/*
 * ============================================================================
 * FEJL I ORIGINAL ISR:
 * ============================================================================
 *
 * ORIGINAL KODE:
 *   ISR(TIMER0_OVF_vect) {
 *       if(i >= 1) {
 *           PORTB ^= (1<<PB1);
 *           i = 0;
 *       }
 *       i++;
 *   }
 *
 * PROBLEMER:
 *
 * 1. FORKERT ISR VEKTOR:
 *    - TIMER0_OVF_vect er overflow interrupt
 *    - Vi aktiverede OCIE0A (compare match), IKKE TOIE0 (overflow)
 *    - Denne ISR vil ALDRIG blive kaldt!
 *    - KORREKT: Brug TIMER0_COMPA_vect
 *
 * 2. FORKERT TOGGLE LOGIK:
 *    - "if(i >= 1)" toggler hver 2. gang = 8 kHz toggle
 *    - 8 kHz toggle = 4 kHz square wave
 *    - Vi vil have 2 kHz, så vi skal toggle hver 4. gang
 *
 * 3. MANGLENDE ADC TRIGGER:
 *    - Original kode antog auto-trigger, men det virker ikke
 *    - Vi skal starte ADC manuelt i ISR
 *
 * ============================================================================
 */

/*
 * Timer0 Compare Match A ISR
 * Kaldes 8000 gange per sekund (8 kHz)
 *
 * Datasheet: Section 11.1, Page 49, Table 11-1 - TIMER0_COMPA interrupt vector
 */
ISR(TIMER0_COMPA_vect) {
    /*
     * TX signal generation:
     * Toggle hver 4. gang = 2 kHz square wave
     *
     * 8 kHz / 4 = 2 kHz toggle rate
     * 2 kHz toggle = 1 kHz square wave? NEJ!
     * Toggle rate = 2 * frekvens, så 2 kHz toggle = 1 kHz... VENT
     *
     * Lad os tænke igen:
     * - Vi vil have 2 kHz square wave (periode = 500µs)
     * - Half-period = 250µs (high) + 250µs (low)
     * - Ved 8 kHz interrupt (125µs interval) skal vi toggle hver 2. gang
     * - 125µs * 2 = 250µs half-period = 2 kHz ✓
     */
    tx_counter++;
    if (tx_counter >= 2) {
        tx_counter = 0;
        /* Toggle Pin 9 - hardware toggle via PINx write
         * Datasheet: Section 13.2.2, Page 60 - Writing to PINx toggles PORTx */
        PINB = (1 << PB1);
    }

    /*
     * Start ADC konvertering (hvis sampling er aktiveret)
     * ADC_vect ISR vil blive kaldt når konvertering er færdig
     * Datasheet: Section 23.9.2, Page 218 - ADSC bit
     */
    if (sampling_enabled) {
        ADCSRA |= (1 << ADSC);
    }
}

/*
 * ADC Conversion Complete ISR
 * Kaldes når ADC konvertering er færdig (~104µs efter ADSC sat)
 *
 * Datasheet: Section 11.1, Page 49, Table 11-1 - ADC interrupt vector
 */
ISR(ADC_vect) {
    /* Læs 10-bit ADC resultat
     * Datasheet: Section 23.9.3, Page 219 - ADCL og ADCH registre */
    uint16_t sample = ADC;
    DFT_sum(sample);
}

/*
 * FJERNET: timer1_init() wrapper funktion
 *
 * ORIGINAL KODE:
 *   void timer1_init(void) {
 *       timer0_init();
 *   }
 *
 * HVORFOR FJERNET:
 * - Forvirrende navngivning: Funktionen hedder "timer1_init" men bruger Timer0
 * - Unødvendig indirektion: main.c skal bare kalde timer0_init() direkte
 * - Nu er koden konsistent: Timer0 initialiseres med timer0_init()
 */
