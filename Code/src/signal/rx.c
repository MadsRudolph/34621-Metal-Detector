/*
 * rx.c - Receiver Signal Processing
 */
#include <avr/io.h>

volatile uint16_t ADC_Raw = 0; // skal trækkes fra ADC når den er sat op

//*****autotrigger skal fjernes og der skal justeres så vi har en sampling på 8kHz**************************
void init_ADC_kanal0() // ADC0 er på Analog 0 pin på arduino board
{
    ADMUX = (1 << REFS0); // ref på 5V for ADC: S.289
                          // Alle MUXn bits er sat lav dette vælger kanal 0 som input: S.290 Table 26-4

    ADCSRA |= (1 << ADEN); // dette enabler adc se på datablad side: S.292
    ADCSRA |= (1 << ADSC); // ADSC starter sample på ADC'en den første sample tager længere,
                           // så derfor at det godt at køre den en gang i initialisering: s.293

    ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // prescale på 128, hvilket bliver til 125kHz med F_CPU = 16MHz: S.293
                                                          // Det tager 13 clock cykler(S.289) at konvertere indgangs spændingen
                                                          // med 125KHz: 125KHz/13 = 9.6KHz, dette er den maksimale sample rate

    ADCSRA |= (1 << ADIE); // ADIE aktivere ADC interrupt, sådan at når et sample er færdig aktivere et interrupt: S.293

    ADCSRA |= (1 << ADATE);                // ADATE lukker op for auto trigger af ADC'en, hvad triggeren er vælges i ADCSRB: S.293
    ADCSRB |= (1 << ADTS2) | (1 << ADTS0); // Disse to bits vælger ADC auto trigger til at være TIMER1 compare match B: S.295
}


ISR(ADC_vect){
   //indsæt her sampling kode
    ADC_Raw = ADC; //vi indlæser ADC målingen i 
    DFT_sum(ADC_Raw); //på en eller anden måde skal denne hive ADC Raw med ind i DSP boksen
}



