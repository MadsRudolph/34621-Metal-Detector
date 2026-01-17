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
 *
 * TODO: Implementer disse features:
 *   - Start/Stop knap på Pin D2 (Krav 9a)
 *   - Kalibrering knap på Pin D3 (Krav 9b)
 *   - IIR filter til glatning af værdier (Krav 8c)
 *   - Metal klassificering - ferro/non-ferro (Krav 2)
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
#include "capture.h"

/* ============ INDSTILLINGER ============ */
//dft indstillinger
#define F_SAMPLE 8000       // Sample frekvens (Hz) - skal være 4x signal frekvens
#define F_SIGNAL 2000       // TX/RX signal frekvens (Hz)
#define N 64                // Antal samples per DFT vindue (flere = mere præcist, men langsommere)

//for samples periodisk Pi/2 ved vælgelse af k = 16, da dette sikrer at vi ikke behøver at lave udregninger
// der indeholder Realdel og Imaginærdel samtidig og ydermere betyder dette at vi kan forudregne og slippe for at regne med 
// andet en positive og negative 1 taller
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

//timer init
void timer0_init() {
// vi vælger mode 4 som er CTC for
    DDRB |=(1<<PB1);
    TCCR0A |= (1<<WGM01); //CTC mode - side 86
    TCCR0B |= (1<<CS01);  // Prescaler 8: CS01=1, CS00=0
    OCR0A = 249;          // 16MHz / 8 / 250 = 8kHz interrupt → 2kHz TX
    TIMSK0 = (1<< OCIE0A);    //Enabler Timer0 Compare match interrupt
}

//adc init
void adc_init(){
    /* Reference = AVCC internal 5v reference */
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
    // Pin D4 som input (debug skærm knap)
    DDRD &= ~(1 << PD4);
    // Aktiver intern pull-up modstand (knap forbinder til GND)
    PORTD |= (1 << PD4);

    // TODO: Pin D2 som input - Start/Stop knap (Krav 9a)
    // TODO: Pin D3 som input - Kalibrering knap (Krav 9b)
}

/* ============ INTERRUPTS ============ */

/*
 * Timer0 Compare Match A Interrupt
 *
 * Denne ISR kører med 8kHz (16MHz / 8 prescaler / 250 = 8000 Hz)
 *
 * Formål:
 *   1. Generere 2kHz TX signal til sendespolen
 *   2. Sætte flag når TX går høj (rising edge) for at synkronisere DFT
 *
 * Timing:
 *   - ISR kører hver 125µs (8kHz)
 *   - Vi toggler TX pin hver 2. gang = 250µs mellem toggles
 *   - 250µs HIGH + 250µs LOW = 500µs periode = 2kHz firkantbølge
 *
 * rising_edge_Flag bruges til at starte DFT sampling præcis når TX går høj,
 * så DFT'en er synkroniseret med TX signalet (fase 0° ved TX rising edge)
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

/*
 * ADC Conversion Complete Interrupt
 *
 * Denne ISR kører automatisk når ADC er færdig med en konvertering.
 * ADC'en er sat op til auto-trigger fra Timer0 Compare Match A,
 * så den sampler med samme 8kHz rate som Timer0.
 *
 * Formål:
 *   1. Læse ADC værdien (0-1023, 10-bit)
 *   2. Føde samples til DFT når rising_edge_Flag er sat
 *
 * Vi sampler kun til DFT når rising_edge_Flag == 1, hvilket sikrer
 * at DFT'en starter synkroniseret med TX rising edge.
 */
ISR(ADC_vect){
    ADC_Raw = ADC;              // Gem ADC værdi (10-bit, 0-1023)

    if(rising_edge_Flag == 1){
        DFT_sum(ADC_Raw);       // Send sample til DFT akkumulator
    }
}


/* ============ DFT BEREGNING ============ */
void DFT_sum(int16_t ADC_Raw){

        // Start MATLAB capture ved starten af et nyt DFT vindue (j==0)
        if(j == 0 && capture_get_state() == CAPTURE_WAITING){
            capture_set_state(CAPTURE_ACTIVE);
        }
        
        xn = ADC_Raw - ADC_middelvaerdi; //fjerner DC offset hvis der er et ****** skal genovervejes *********

        // Gem sample til MATLAB capture hvis aktiv
        capture_store_sample(j, xn);

        switch(j & 3){ // & 3 betyder at vi tæller 0->3 og så forfra igen selvom "i" er større

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

            // Marker capture som færdig hvis aktiv
            if(capture_get_state() == CAPTURE_ACTIVE){
                capture_set_state(CAPTURE_DONE);
            }
        }
    }

void DFT_Calc(){
    mag = sqrt((float)Re_buff*Re_buff + (float)Im_buff*Im_buff)/N;
    ang = atan2((float)Im_buff, (float)Re_buff)*57.2957795131;
}

// IIR filter variable, sat til 1 for at vi ikke ganger med 0 første gang og får forsinkelse i startup
    volatile uint32_t mag_filtered; //filtreret magnitude, 32 bit for at kunne holde multiplikation
    volatile int16_t ang_filtered; //filtreret fase i grader
    static uint8_t filt_init = 0; // bruges til at sikre at vi ikke ganger med 0 i første udregning
    //IIR filter som sikrer at vi holder stabile displayværdier for at gøre læsning af denne nemmere
    void IIR_Filt(){
        //opstarts funktion
        if(!filt_init){ //for at undgå forsinkelse ved startup sættes filterresultatet til det målte på den første måling
        mag_filtered = mag;
        ang_filtered = ang;  
        filt_init = 1; // flag sættes og vi udregner nedenfor
        // ellers kører udregningen
        }else{
        //filtreret magnitude regnet som 32 bit for at undgå overflow
        mag_filtered = (mag_filtered*9 + (uint32_t)mag*1)/10; // vi dividerer med 10 (alfa = 1/10) for at undgå floats 
        //filtreret fase
        ang_filtered = (ang_filtered*9 + ang*1)/10; // vi dividerer med 10 (alfa = 1/10) for at undgå floats
        }
}

/* ============ DISPLAY ============ */

static char buf[20];
static uint8_t show_debug = 0;

void display_dft(void) {
    sendStrXY("=== DFT ===     ", 0, 0);

    sprintf(buf, "Re:   %-9ld", Re_buff);
    sendStrXY(buf, 2, 0);

    sprintf(buf, "Im:   %-9ld", Im_buff);
    sendStrXY(buf, 3, 0);

    sprintf(buf, "Mag:  %-9u", mag_filtered);
    sendStrXY(buf, 5, 0);

    sprintf(buf, "Fase: %-9d", ang_filtered);
    sendStrXY(buf, 6, 0);

    // TODO: Vis metal type (Krav 2)
}

void display_debug(void) {
    sendStrXY("=== DEBUG ===   ", 0, 0);

    sprintf(buf, "ADC:  %-9u", ADC_Raw);
    sendStrXY(buf, 2, 0);
}

/* ============ MAIN ============ */

int main(void) {
    // Initialiser hardware
    I2C_Init();
    InitializeDisplay();
    clear_display();
    timer0_init();
    adc_init();
    init_button();
    capture_init(115200);  // UART til MATLAB kommunikation (skal matche platformio.ini)

    // VIGTIGT: Aktiver globale interrupts!
    sei();

    // Vis opstarts besked
    sendStrXY("Starter...", 3, 2);
    _delay_ms(1000);
    clear_display();

    // Knap state til edge detection
    uint8_t btn_prev = 1;

#if DFT_VERIFICATION_MODE
    uint16_t ready_counter = 0;  // Tæller til periodisk READY signal
    // Send initial READY efter reset
    _delay_ms(100);  // Vent lidt på UART stabilisering
    capture_send_ready();
#endif

    // Hovedløkke
    while (1) {
        // Når DFT vindue er færdigt, beregn og vis resultater
        if (DFT_done) {
            DFT_done = 0;
            DFT_Calc();  // Beregn magnitude og fase fra Re_buff/Im_buff
            IIR_Filt(); // filtrerede værdier
            // TODO: Anvend IIR lavpas filter (Krav 8c)
            // TODO: Detekter metal type (Krav 2)

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

        // TODO: Tjek knap D2 - Start/Stop detektor (Krav 9a)
        // TODO: Tjek knap D3 - Kalibrering (Krav 9b)

#if DFT_VERIFICATION_MODE
        // MATLAB capture - tjek for seriel kommando
        capture_check_serial();

        // Hvis capture er færdig, send data til MATLAB
        if(capture_get_state() == CAPTURE_DONE){
            capture_output(Re_buff, Im_buff, mag, ang);
            capture_set_state(CAPTURE_IDLE);
            ready_counter = 0;  // Reset så READY sendes med det samme
        }

        // Send READY periodisk når idle (så MATLAB kan forbinde når som helst)
        if(capture_get_state() == CAPTURE_IDLE){
            ready_counter++;
            if(ready_counter >= 20){  // Ca. hvert sekund (20 * 50ms)
                capture_send_ready();
                ready_counter = 0;
            }
        }
#endif

        _delay_ms(50);
    }
}
