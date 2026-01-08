/*
 * tx.c
 * TX signal generation using Timer1
 *
 * Generates 2 kHz square wave on Timer1 OC1A output:
 *   - ATmega2560: Pin 11 (PB5)
 *   - ATmega328P: Pin 9 (PB1)
 */

#include "tx.h"
#include "../config.h"
#include <avr/io.h>
#include <avr/interrupt.h>

static uint8_t i = 0;
uint16_t ADC_Raw = 0; 

void timer1_init() {
// vi vælger mode 4 som er CTC for 
    DDRB |=(1<<PB1); 
    TCCR1B |= (1<<WGM12) | (1<<CS10);  //CTC (WGM)  og ingen prescaler (CS10) side 86
    OCR1A = 999;  // med formel: f_OCnA = fclk/(2*N*(1+OCRnA)) = 999 ticks med prescaler på 1
    TIMSK1 = (1<<TOIE1);          //Enabler Timer1 Overflow interrupt
}

void adc_init(){
    /* Reference = AVCC (5V), kanal 0 (A0) */
    ADMUX = (1 << REFS0);
    /*
     * ADC Kontrol:
     * ADEN  = Aktiver ADC
     * ADIE  = Aktiver ADC interrupt
     * ADATE = Aktiver auto-trigger
     * ADPS  = Prescaler 128 (125kHz ADC clock)
     */
    ADCSRA = (1 << ADEN) | (1 << ADIE) | (1 << ADATE)
           | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); //denne linje er prescaler på 128
    /* Auto-trigger kilde = Timer1 Overflow */
    ADCSRB = (1 << ADTS2) | (1 << ADTS1);

       /* Start første konvertering */
    ADCSRA |= (1 << ADSC);
}
ISR(OVF1_vect){
    if(i >= 1 ){
        PORTB ^=(1<<PB1); 
        i = 0;
    }
    i++; 

}
ISR(ADC_vect){
    ADC_Raw = ADC; 
    DFT_sum(ADC_Raw);
}
