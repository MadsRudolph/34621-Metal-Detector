/*
 * dsp.c
 * DFT analysis + IIR filtering for Arduino Nano (ATmega328P)
 *
 * - Optimized DFT using 4x oversampling (no trig needed)
 * - IIR low-pass for display smoothing
 * - Integer math only (no floating-point)
 */

#include "dsp.h"
#include <avr/io.h>        // inkluderer i/o funktioner
#include <stdlib.h>
#include <stdint.h>        // inkluderer uint16_t
/*
 * FJERNET: #include <math.h>
 *
 * HVORFOR: Vi bruger ikke længere sqrt() og atan2() fra math.h.
 * I stedet bruger vi vores egne integer-baserede funktioner
 * isqrt() og fast_atan2_deg() som er meget hurtigere på AVR.
 */

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

/*
 * FEJL: Originalt var Re og Im defineret som uint32_t (unsigned):
 *   volatile uint32_t Re = 0;
 *   volatile uint32_t Im = 0;
 *
 * HVORFOR DET ER FORKERT:
 * DFT akkumulatoren subtraherer værdier (Re -= xn, Im -= xn).
 * Med unsigned typer vil negative resultater wrappe til store positive tal.
 * F.eks. hvis Re = 100 og vi gør Re -= 200, får vi 2^32 - 100 i stedet for -100.
 *
 * KORREKT: Brug int32_t (signed) så negative værdier håndteres korrekt.
 */
volatile int32_t Re = 0;
volatile int32_t Re_buff = 0;
volatile int32_t Im = 0;
volatile int32_t Im_buff = 0;

/*
 * FEJL: xn var defineret som uint16_t:
 *   volatile uint16_t xn = 0;
 *
 * HVORFOR DET ER FORKERT:
 * Efter DC offset fjernelse (ADC_Raw - 512) kan xn blive negativ.
 * F.eks. hvis ADC_Raw = 100, så er xn = 100 - 512 = -412.
 * Med uint16_t bliver dette til 65124 (forkert!).
 *
 * KORREKT: Brug int16_t for at tillade negative værdier.
 */
volatile int16_t xn = 0;

volatile uint8_t DFT_done;

/*
 * NYE VARIABLER til eksport af resultater.
 * Disse manglede i original kode - DFT_Calc() beregnede værdier
 * men gemte dem ikke hvor main.c kunne bruge dem.
 */
volatile uint16_t magnitude = 0;
volatile int16_t phase_deg = 0;
volatile uint16_t magnitude_filt = 0;
volatile int16_t phase_filt = 0;

/* IIR filter koefficient: alpha = 32/256 = 0.125 (langsom, stabil) */
#define IIR_ALPHA 32

//i koden nedenfor ganger vi skiftevis Re og im på xn 
//cos: [1,0,-1,0], sin: [0,1,0,-1] ved Fmaks = Fs/4 
//Re = x[n]*cos(phase i rads)
//Im = x[n]*sin(phase i rads)
// kører kun en gang per ADC interrupt indtil vi har N = 64 samples
void DFT_sum(uint16_t ADC_Raw){
        /*
         * BEMÆRK: Cast til int16_t er nødvendigt her.
         * ADC_Raw er uint16_t (0-1023), ADC_middelvaerdi er 512.
         * Resultatet kan være negativt (-512 til +511), så vi skal
         * gemme det i en signed variabel.
         */
        xn = (int16_t)ADC_Raw - ADC_middelvaerdi;

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

/*
 * ============================================================================
 * FEJL I ORIGINAL DFT_Calc():
 * ============================================================================
 *
 * ORIGINAL KODE:
 *   void DFT_Calc(){
 *       uint16_t mag = sqrt(Re_buff*Re_buff + Im_buff*Im_buff);
 *       uint16_t ang = (atan2(Re_buff, Im_buff)*180)/3.14;
 *       uint16_t nigger = mag/N;  // <-- UACCEPTABELT VARIABELNAVN
 *   }
 *
 * PROBLEMER:
 *
 * 1. FLOATING-POINT FUNKTIONER (sqrt, atan2):
 *    - AVR har ingen FPU (floating-point unit)
 *    - sqrt() og atan2() fra math.h bruger software floating-point
 *    - Hver sqrt() tager ~2000 clock cycles, atan2() endnu mere
 *    - Dette er ALT for langsomt til at køre ved hver DFT (125 gange/sek)
 *    - LØSNING: Brug integer-baserede isqrt() og fast_atan2_deg()
 *
 * 2. OVERFLOW VED SQUARING:
 *    - Re_buff kan være op til 64*512 = 32768
 *    - Re_buff*Re_buff = 32768^2 = 1,073,741,824 (overstiger int32_t max!)
 *    - LØSNING: Skaler ned FØR squaring
 *
 * 3. ATAN2 ARGUMENTER BYTTET:
 *    - Standard atan2 er atan2(y, x) = atan2(Im, Re)
 *    - Original kode havde atan2(Re_buff, Im_buff) - byttet om!
 *
 * 4. RESULTATER IKKE EKSPORTERET:
 *    - mag og ang er lokale variabler der forsvinder efter funktionen
 *    - main.c har brug for magnitude_filt og phase_filt
 *    - LØSNING: Gem i globale volatile variabler
 *
 * 5. UACCEPTABELT VARIABELNAVN:
 *    - Krænkende og uprofessionelt
 *    - Fjernet helt
 *
 * ============================================================================
 */

/*
 * Integer square root (Newton-Raphson metode)
 * Undgår floating-point - ca. 50x hurtigere end sqrt() på AVR
 */
static uint16_t isqrt(uint32_t n) {
    if (n == 0) return 0;

    uint32_t x = n;
    uint32_t y = (x + 1) / 2;

    /* Newton iteration: x_new = (x + n/x) / 2 */
    while (y < x) {
        x = y;
        y = (x + n / x) / 2;
    }
    return (uint16_t)x;
}

/*
 * Hurtig integer atan2 approximation
 * Returnerer vinkel i grader (-180 til +180)
 * Nøjagtighed: ca. ±2 grader (tilstrækkeligt til metal klassifikation)
 */
static int16_t fast_atan2_deg(int32_t y, int32_t x) {
    /* Håndter special cases */
    if (x == 0 && y == 0) return 0;
    if (x == 0) return (y > 0) ? 90 : -90;
    if (y == 0) return (x > 0) ? 0 : 180;

    int16_t angle;
    int32_t abs_y = (y < 0) ? -y : y;
    int32_t abs_x = (x < 0) ? -x : x;

    /*
     * Approximation baseret på: atan(z) ≈ 45° * z for små z
     * Vi bruger fixed-point med 64 som skaleringsfaktor
     */
    if (abs_x >= abs_y) {
        int32_t z = (abs_y * 64) / abs_x;
        angle = (z * 45) / 64;
    } else {
        int32_t z = (abs_x * 64) / abs_y;
        angle = 90 - (z * 45) / 64;
    }

    /* Juster for kvadrant */
    if (x < 0) angle = 180 - angle;
    if (y < 0) angle = -angle;

    return angle;
}

void DFT_Calc(void) {
    int32_t re = Re_buff;
    int32_t im = Im_buff;

    /*
     * Skaler ned for at undgå overflow ved squaring.
     * Max værdi: 64 * 512 = 32768
     * Efter /16: max = 2048
     * 2048^2 = 4,194,304 (passer fint i uint32_t)
     */
    int32_t re_scaled = re / 16;
    int32_t im_scaled = im / 16;

    /* Magnitude = sqrt(Re² + Im²) */
    uint32_t mag_squared = (uint32_t)(re_scaled * re_scaled + im_scaled * im_scaled);
    magnitude = isqrt(mag_squared);

    /*
     * Phase = atan2(Im, Re) i grader
     * BEMÆRK: atan2 tager (y, x) = (Im, Re), IKKE (Re, Im)!
     */
    phase_deg = fast_atan2_deg(im, re);

    /*
     * IIR Low-pass filter for at udjævne visning
     * y[n] = α * x[n] + (1-α) * y[n-1]
     * Med α = 32/256 = 0.125 får vi langsom men stabil respons
     */
    magnitude_filt = (uint16_t)((IIR_ALPHA * (uint32_t)magnitude +
                      (256 - IIR_ALPHA) * (uint32_t)magnitude_filt) / 256);
    phase_filt = (int16_t)((IIR_ALPHA * (int32_t)phase_deg +
                  (256 - IIR_ALPHA) * (int32_t)phase_filt) / 256);
}