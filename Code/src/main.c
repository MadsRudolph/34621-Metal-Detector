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
//dft indstillinger
#define F_SAMPLE 8000       // Sample frekvens (Hz) - skal være 4x signal frekvens
#define F_SIGNAL 2000       // TX/RX signal frekvens (Hz)
#define N 64                // Antal samples per DFT vindue (flere = mere præcist, men langsommere)

#define RePhase1  1 //cos(0)
#define ImPhase2  1 //sin(Pi/2)
#define RePhase3 -1 //cos(Pi)
#define ImPhase4 -1 //sin(3Pi/2)
/* ============ GLOBALE VARIABLE ============ */


//tx og rx variabler
static uint8_t i = 0;
int16_t ADC_Raw = 0; // signed ADCRAW værdi 
extern volatile uint8_t rising_edge_Flag = 0; // flag der indikerer at en DFT er kørt

//dft variabler
volatile uint8_t j = 0;
volatile int32_t Re = 0;
volatile int32_t Re_buff = 0;
volatile int32_t Im = 0;
volatile int32_t Im_buff = 0;
volatile int16_t xn = 0; //det fourrier transformerede tidssignal (nu i frekvensdomænet)
volatile uint8_t DFT_done;


/* ============ HARDWARE INIT ============ */

//timer init 
void timer0_init() {
// vi vælger mode 4 som er CTC for 
    DDRB |=(1<<PB1); 
    TCCR0A |= (1<<WGM01); //CTC mode - side 86
    TCCR0B |= (1<<CS01);  //Prescaler på 8 - side 86
    OCR0A = 124;  // med formel: f_OCnA = fclk/(2*N*(1+OCRnA)) = 124 ticks med prescaler på 8
    /****************CHAT SIGER AT OVENSTÅENDE GIVER 16 KHZ WTFFFFFFF*************/
    TIMSK0 = (1<< OCIE0A);    //Enabler Timer0 Compare match interrupt
}

//adc init 
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

void init_button(void) {
    // Pin D4 som input
    DDRD &= ~(1 << PD4);
    // Aktiver intern pull-up modstand (knap forbinder til GND)
    PORTD |= (1 << PD4);
}

/* ============ INTERRUPTS ============ */

// Timer0 interrupt - kører 8000 gange per sekund
ISR(TIMER0_COMPA_vect){
    if(i >= 1 ){    //vi sikrer at vi toggler med 4kHz som giver 2kHz firkant pulser
        PORTB ^=(1<<PB1); //toggler PB1 som er tx 
        i = 0; //resetter counter hvis vi har togglet
    if(PORTB &=(1<<PB1)){
        rising_edge_Flag = 1;
    }
    }
    i++; //ittererer tæller
}
// ADC interrupt - kører hver gang en ADC conversion er færdig (8kHz)
ISR(ADC_vect){
    ADC_Raw = ADC; // buffer til at holde ADC værdi
      if(rising_edge_Flag == 1){
        DFT_sum(ADC_Raw); //DFT 
        }
    }

   

    


/* ============ DFT BEREGNING ============ */
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

void DFT_Calc(){
    // TODO: implementer beregning af fase og magnitude

    uint16_t mag = sqrt(Re_buff*Re_buff + Im_buff*Im_buff);//vi vil kun have magnituden i heltal derfor ingen float
    uint8_t ang = (atan2(Im_buff, Re_buff)*180)/3.14159265; //vi vil kun have fasen i heltal derfor ingen float
   
    // nu skal vi lave IDFT
    uint16_t Xk = mag/N;  
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
