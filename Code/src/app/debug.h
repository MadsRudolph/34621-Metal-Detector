/*
 * debug.h
 * Debug og diagnostik system til metal detektor
 *
 * Giver 4 debug skærme til at verificere:
 * 1. ADC sampling
 * 2. DFT akkumulator
 * 3. DFT resultat (magnitude/fase)
 * 4. Timing synkronisering
 */

#ifndef DEBUG_H
#define DEBUG_H

#include <stdint.h>

/* ===== Debug Screen Enumeration ===== */
typedef enum {
    DEBUG_SCREEN_ADC,        /* Rå ADC værdier og range */
    DEBUG_SCREEN_DFT_ACCUM,  /* DFT akkumulator Re/Im */
    DEBUG_SCREEN_DFT_RESULT, /* Magnitude, fase, klassifikation */
    DEBUG_SCREEN_TIMING,     /* Timing og synkronisering */
    DEBUG_SCREEN_COUNT       /* Antal skærme (til cycling) */
} debug_screen_t;

/* ===== Debug State Variables ===== */

/* ADC debug data */
extern volatile uint16_t debug_adc_last;      /* Seneste ADC værdi */
extern volatile uint16_t debug_adc_min;       /* Minimum set værdi */
extern volatile uint16_t debug_adc_max;       /* Maximum set værdi */
extern volatile uint32_t debug_adc_count;     /* Antal samples */

/* DFT debug data */
extern volatile int32_t debug_dft_re;         /* Seneste Re akkumulator */
extern volatile int32_t debug_dft_im;         /* Seneste Im akkumulator */
extern volatile uint32_t debug_dft_count;     /* Antal færdige DFT */
extern volatile uint8_t debug_dft_index;      /* Nuværende sample index */

/* Timing debug data */
extern volatile uint32_t debug_isr_count;     /* ISR kald tæller */

/* Nuværende debug skærm */
extern debug_screen_t debug_current_screen;

/* Debug mode aktiv */
extern uint8_t debug_enabled;

/* ===== Function Prototypes ===== */

/*
 * debug_init
 * Initialiser debug system
 * Kald én gang ved opstart
 */
void debug_init(void);

/*
 * debug_update
 * Opdater debug display
 * Kald fra main loop (~10-20 Hz)
 */
void debug_update(void);

/*
 * debug_next_screen
 * Skift til næste debug skærm
 * Kald ved knaptryk
 */
void debug_next_screen(void);

/*
 * debug_toggle
 * Slå debug mode til/fra
 */
void debug_toggle(void);

/*
 * debug_log_adc_sample
 * Log ADC sample til debug statistik
 * Kald fra ADC ISR
 */
void debug_log_adc_sample(uint16_t sample);

/*
 * debug_log_dft_done
 * Log færdig DFT resultat
 * Kald fra dsp.c når DFT er færdig
 */
void debug_log_dft_done(int32_t re, int32_t im);

/*
 * debug_log_dft_index
 * Log nuværende DFT sample index
 * Kald fra dsp.c i DFT_sum
 */
void debug_log_dft_index(uint8_t index);

/* ===== Integer Math Utilities ===== */

/*
 * isqrt
 * Heltal kvadratrod (Newton's metode)
 * Input: n (32-bit unsigned)
 * Output: floor(sqrt(n))
 */
uint16_t isqrt(uint32_t n);

/*
 * fast_atan2_deg
 * Hurtig atan2 approksimation
 * Input: y, x (32-bit signed)
 * Output: vinkel i grader (-180 til +180)
 */
int16_t fast_atan2_deg(int32_t y, int32_t x);

/* ===== Debug Pin Configuration ===== */

/* Pin 13 (PB7) bruges til timing verifikation med oscilloskop */
#define DEBUG_PIN_PORT  PORTB
#define DEBUG_PIN_DDR   DDRB
#define DEBUG_PIN_BIT   PB7

/*
 * debug_pin_init
 * Konfigurer debug pin som output
 */
void debug_pin_init(void);

/*
 * debug_pin_toggle
 * Toggle debug pin (til oscilloskop måling)
 */
#define debug_pin_toggle() (DEBUG_PIN_PORT ^= (1 << DEBUG_PIN_BIT))

#endif /* DEBUG_H */
