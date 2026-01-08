/*
 * dsp.c
 * DFT analysis + IIR filtering
 *
 * - Optimized DFT using 4x oversampling (no trig needed)
 * - IIR low-pass for display smoothing
 */

#include "dsp.h"
#include <avr/io.h>        // inkluderer i/o funktioner
#include <stdlib.h>
#include <stdint.h>        // inkluderer uint16_t
#include <math.h>

/* TODO: Implement */
//******** ikke sikkert de her defines udover N skal bruges **********/
#define Fs 8000
#define N  64
#define f0 2000
#define ADC_middelvaerdi 512 // skal kun bruges hvis der er DC offset

//herunder har jeg udeladt at definere de Reelle og imaginære 
//dele som giver 0 da de ikke er relevante
//disse nedenfor er fasebeskrivelserne ved inkrimentering (n=0,1,2......N)af 
//90 grader mod uret fra 0 / 2Pi på enhedscirklen, ved at sample kun ved disse
// punkter bliver fasen skiftevis rent reel og rent Imaginær
// phase nummeret er værdien af n
#define RePhase1  1 //cos(0)
#define ImPhase2  1 //sin(Pi/2)
#define RePhase3 -1 //cos(Pi)
#define ImPhase4 -1 //sin(3Pi/2)

static uint8_t i = 0;
volatile uint32_t Re = 0;
volatile uint32_t Re_buff = 0;
volatile uint32_t Im = 0;
volatile uint32_t Im_buff = 0;
volatile uint16_t xn = 0;
volatile uint8_t DFT_done;

//i koden nedenfor ganger vi skiftevis Re og im på xn 
//cos: [1,0,-1,0], sin: [0,1,0,-1] ved Fmaks = Fs/4 
//Re = x[n]*cos(phase i rads)
//Im = x[n]*sin(phase i rads)
// kører kun en gang per ADC interrupt indtil vi har N = 64 samples 
void DFT_sum(uint16_t ADC_Raw){

        xn = ADC_Raw - ADC_middelvaerdi; //fjerner DC offset hvis der er et ****** skal genovervejes *********

        switch(i & 3){ // & 3 betyder at vi kun bruger de 3 nederste bits og derfor tæller 0->3 selvom i er større 

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
        i++;
        if(i>=N){ //hvis vi når N-1 starter vi forfra og gemmer i buffer 
            i = 0;
            Re_buff = Re; //vi overfører til en buffer inden vi resetter ellers vil vi miste sidste 
            Im_buff = Im;
            DFT_done = 1 ;
            Re = 0;
            Im = 0;
        }
    }

//****** her skal der så indsættes en beregner der finder fasen og magnituden ****////

void DFT_Calc(){
    // TODO: implementer beregning af fase og magnitud

    uint16_t mag = sqrt(Re_buff*Re_buff + Im_buff*Im_buff);
    uint16_t ang = (atan2(Re_buff, Im_buff)*180)/3.14;

    // nu skal vi lave IDFT

    uint16_t nigger = mag/N;  


}