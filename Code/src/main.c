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
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "drivers/I2C.h"
#include "drivers/ssd1306.h"

/* ============ INDSTILLINGER ============ */
//dft indstillinger
#define F_SAMPLE 8000       // Sample frekvens (Hz) - skal være 4x signal frekvens
#define F_SIGNAL 2000       // TX/RX signal frekvens (Hz)
#define N 64                // Antal samples per DFT vindue (flere = mere præcist, men langsommere)

#define RePhase1  1 //cos(0)
#define ImPhase2  1 //sin(Pi/2)
#define RePhase3 -1 //cos(Pi)
#define ImPhase4 -1 //sin(3Pi/2)
#define ADC_middelvaerdi 512 // skal kun bruges hvis der er DC offset

/* ============ GLOBALE VARIABLE ============ */


//tx og rx variabler
static uint8_t i = 0;
int16_t ADC_Raw = 0; // signed ADCRAW værdi 
volatile uint8_t rising_edge_Flag = 0; // flag der indikerer at en DFT er kørt

//dft variabler
volatile uint8_t j = 0;
volatile int32_t Re = 0;
volatile int32_t Re_buff = 0;
volatile int32_t Im = 0;
volatile int32_t Im_buff = 0;
volatile int16_t xn = 0; //det fourrier transformerede tidssignal (nu i frekvensdomænet)
volatile uint8_t DFT_done;
volatile uint16_t mag = 0;
volatile int16_t ang = 0;  // Fase i grader (-180 til +180)

/* ============ FUNCTION PROTOTYPES ============ */
void DFT_sum(int16_t ADC_Raw);  // Forward declaration - bruges i ADC ISR

/* ============ HARDWARE INIT ============ */

/**
 * Initialize Timer0 to generate periodic compare-match interrupts and configure the TX pin.
 *
 * Configures PB1 as an output, sets Timer0 to CTC mode with a prescaler of 8, sets OCR0A
 * to produce compare-match interrupts at 8 kHz, and enables the Timer0 Compare Match A interrupt.
 *
 */
void timer0_init() {
// vi vælger mode 4 som er CTC for 
    DDRB |=(1<<PB1); 
    TCCR0A |= (1<<WGM01); //CTC mode - side 86
    TCCR0B |= (1<<CS01);  // Prescaler 8: CS01=1, CS00=0
    OCR0A = 249;          // 16MHz / 8 / 250 = 8kHz interrupt → 2kHz TX
    TIMSK0 = (1<< OCIE0A);    //Enabler Timer0 Compare match interrupt
}

/**
 * Initialize the ADC to use AVcc as reference, enable the ADC with interrupt-driven
 * conversions triggered by Timer0 Compare Match A, set the clock prescaler to 64,
 * and start the first conversion.
 *
 * Configured behavior:
 * - Reference selection: AVcc (REFS0).
 * - ADC enabled and ADC Conversion Complete interrupt enabled.
 * - Auto-triggering enabled with trigger source = Timer0 Compare Match A.
 * - ADC clock prescaler set to 64 (ADPS2|ADPS1).
 * - Starts the first ADC conversion.
 */
void adc_init(){
    /* Reference = AVCC (5V taget fra 5V pin til vores REF pin) */
    ADMUX = (1 << REFS0);
    /*
     * ADC Kontrol:
     * ADEN  = Aktiver ADC
     * ADIE  = Aktiver ADC interrupt
     * ADATE = Aktiver auto-trigger
     * ADPS  = Prescaler 128 (125kHz ADC clock)
     */
    ADCSRA = (1 << ADEN) | (1 << ADIE) | (1 << ADATE)
           | (1 << ADPS2) | (1 << ADPS1); //denne linje er prescaler på 64 ** denne skal genovervejes**
    /* Auto-trigger kilde = Timer0 compare match A */
    ADCSRB |=  (1<<ADTS1)|(1<<ADTS0); // Starter en ADC konvertering på Compare match A

       /* Start første konvertering */
    ADCSRA |= (1 << ADSC);
}

/**
 * Configure PD4 as an input with the internal pull-up enabled for an active-low button.
 *
 * Sets the DDRD bit for PD4 to input and enables PORTD pull-up so the button can be
 * wired to ground (press pulls the pin low).
 */
void init_button(void) {
    // Pin D4 som input
    DDRD &= ~(1 << PD4);
    // Aktiver intern pull-up modstand (knap forbinder til GND)
    PORTD |= (1 << PD4);
}

/* ============ INTERRUPTS ============ */

/**
 * Timer0 Compare Match A interrupt handler that generates the TX waveform and
 * signals DFT window synchronization.
 *
 * This ISR runs at 8 kHz and toggles the TX output pin (PB1) every second
 * interrupt to produce a 2 kHz square wave. It increments the internal
 * interrupt counter and sets `rising_edge_Flag` when the TX pin transitions
 * from low to high to indicate the start of a DFT sampling window.
 *
 * Side effects:
 *   - Toggles PORTB PB1 (TX output).
 *   - Modifies the global interrupt counter `i`.
 *   - Sets `rising_edge_Flag` when a rising edge on TX is detected.
 */
ISR(TIMER0_COMPA_vect){
    i++;                        // Tæl antal interrupts
    if(i >= 2){                 // Hver 2. interrupt (4kHz toggle rate)
        PORTB ^=(1<<PB1);       // Toggle TX pin (XOR flipper bit)
        i = 0;                  // Nulstil tæller

        // Tjek om TX lige gik høj (rising edge)
        if(PORTB & (1<<PB1)){
            rising_edge_Flag = 1;  // Signal til ADC ISR: start ny DFT periode
        }
    }
}

/**
 * Handle ADC conversion completion and forward samples for DFT accumulation.
 *
 * Invoked on ADC conversion complete (ADC_vect). Reads the 10-bit ADC result into the global
 * `ADC_Raw` variable and, if `rising_edge_Flag` is set, forwards the sample to `DFT_sum`
 * for accumulation. The ADC is expected to be auto-triggered (Timer0 Compare Match A) so
 * this ISR effectively services the 8 kHz sampling rate used by the DFT.
 */
ISR(ADC_vect){
    ADC_Raw = ADC;              // Gem ADC værdi (10-bit, 0-1023)

    if(rising_edge_Flag == 1){
        DFT_sum(ADC_Raw);       // Send sample til DFT akkumulator
    }
}

   

    


/**
     * Accumulates a single ADC sample into the N-point DFT accumulators and finalizes the DFT buffer when the window completes.
     *
     * The provided raw ADC sample has its DC offset removed (using ADC_middelvaerdi) before being added to the real and imaginary accumulators
     * according to the current phase step. Advances the sample index and, when N samples have been accumulated, copies accumulators to
     * Re_buff/Im_buff, sets DFT_done, resets Re and Im and the sample index, and clears the rising_edge_Flag.
     *
     * @param ADC_Raw Raw ADC sample (0–1023) to be incorporated into the DFT accumulation.
     */
void DFT_sum(int16_t ADC_Raw){
        
        xn = ADC_Raw - ADC_middelvaerdi; //fjerner DC offset hvis der er et ****** skal genovervejes *********

        switch(j & 3){ // & 3 betyder at vi kun bruger de 3 nederste bits og derfor tæller 0->3 selvom "i" er større 

            case 0: // cos(0)*xn = 1*xn
                Re += RePhase1*xn;
            break;

            case 1: // -sin(pi/2)*xn = -1*xn
                Im += -ImPhase2*xn; //negativt fortegn da imaginærdel 
            break;

            case 2: // cos(Pi)*xn = 1*xn
                Re += RePhase3*xn;
            break;

            case 3: // -sin(3Pi/2)*xn = -(-1)*xn
                Im += -ImPhase4*xn; //negativt fortegn da imaginærdel 
            break;
        }
        j++;

        if(j>=N){ //hvis vi når N-1 starter vi forfra og gemmer i buffer 
            j = 0;
            Re_buff = Re; //vi overfører til en buffer inden vi resetter ellers vil vi miste sidste 
            Im_buff = Im;
            DFT_done = 1 ;
            Re = 0;
            Im = 0;
            rising_edge_Flag = 0; //resetter flag efter DFT er kørt
        }
    }

/**
 * Compute magnitude and phase from the buffered DFT accumulators.
 *
 * Reads Re_buff and Im_buff, computes magnitude as sqrt(Re^2 + Im^2) divided by N,
 * and computes phase in degrees using atan2(Im_buff, Re_buff). Results are stored
 * in the global variables `mag` (magnitude) and `ang` (angle in degrees).
 */
void DFT_Calc(){
    mag = sqrt(Re_buff*Re_buff + Im_buff*Im_buff)/N;
    ang = atan2(Im_buff, Re_buff)*57.2957795131;
}

/* ============ DISPLAY ============ */

static char buf[20];
static uint8_t show_debug = 0;

/**
 * Display the latest DFT results on the OLED.
 *
 * Shows buffered real and imaginary components, computed magnitude, and phase
 * at fixed positions on the screen.
 */
void display_dft(void) {
    sendStrXY("=== DFT ===     ", 0, 0);

    sprintf(buf, "Re:   %-9ld", Re_buff);
    sendStrXY(buf, 2, 0);
    
    sprintf(buf, "Im:   %-9ld", Im_buff);
    sendStrXY(buf, 3, 0);

    sprintf(buf, "Mag:  %d", mag);
    sendStrXY(buf, 5, 0);

    sprintf(buf, "Fase: %d", ang);
    sendStrXY(buf, 6, 0);
}

/**
 * Render a debug screen on the OLED showing a header and the current raw ADC sample.
 *
 * Writes a static "=== DEBUG ===" header and a formatted "ADC: <value>" line using the global
 * `ADC_Raw` value and the shared string buffer `buf`, positioned on the display.
 */
void display_debug(void) {
    sendStrXY("=== DEBUG ===   ", 0, 0);

    sprintf(buf, "ADC:  %-9u", ADC_Raw);
    sendStrXY(buf, 2, 0);
}

/**
 * Initialize peripherals, enable interrupts, and run the main processing loop that handles DFT completion and display mode toggling.
 *
 * Initializes I2C and OLED, configures Timer0, ADC, and the user button, then enables global interrupts. Shows a brief startup message and enters an infinite loop that:
 * - processes completed DFT windows (computes magnitude/phase and updates the OLED), and
 * - monitors the button (PD4) for falling-edge presses to toggle between DFT and debug displays with debounce.
 *
 * @returns Program exit status (not used; main does not return in normal embedded operation).
 */

int main(void) {
    // Initialiser hardware
    I2C_Init();
    InitializeDisplay();
    clear_display();
    timer0_init();
    adc_init();
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
        if (DFT_done) {
            DFT_done = 0;
            DFT_Calc();  // Beregn magnitude og fase fra Re_buff/Im_buff

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