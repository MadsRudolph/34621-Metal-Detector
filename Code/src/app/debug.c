/*
 * debug.c
 * Debug og diagnostik system til metal detektor
 *
 * Fire debug skærme:
 * 1. ADC: Rå værdier, min/max, sample count
 * 2. DFT Akkumulator: Re, Im, completion count
 * 3. DFT Resultat: Magnitude, fase, klassifikation
 * 4. Timing: Frekvenser, ISR count, sync status
 */

#include "debug.h"
#include "display.h"
#include "../drivers/ssd1306.h"
#include <avr/io.h>
#include <string.h>

/* ===== Debug State Variables ===== */

/* ADC debug data */
volatile uint16_t debug_adc_last = 0;
volatile uint16_t debug_adc_min = 1023;
volatile uint16_t debug_adc_max = 0;
volatile uint32_t debug_adc_count = 0;

/* DFT debug data */
volatile int32_t debug_dft_re = 0;
volatile int32_t debug_dft_im = 0;
volatile uint32_t debug_dft_count = 0;
volatile uint8_t debug_dft_index = 0;

/* Timing debug data */
volatile uint32_t debug_isr_count = 0;

/* Nuværende debug skærm */
debug_screen_t debug_current_screen = DEBUG_SCREEN_ADC;

/* Debug mode aktiv */
uint8_t debug_enabled = 0;

/* ===== Intern Hjælpefunktioner ===== */

/* Buffer til tal-til-string konvertering */
static char num_buf[12];

/*
 * int_to_str
 * Konverter signed integer til string
 */
static char* int_to_str(int32_t value)
{
    char *p = num_buf + 11;
    uint32_t abs_val;
    uint8_t negative = 0;

    *p = '\0';

    if (value < 0) {
        negative = 1;
        abs_val = (uint32_t)(-value);
    } else {
        abs_val = (uint32_t)value;
    }

    if (abs_val == 0) {
        *(--p) = '0';
    } else {
        while (abs_val > 0) {
            *(--p) = '0' + (abs_val % 10);
            abs_val /= 10;
        }
    }

    if (negative) {
        *(--p) = '-';
    }

    return p;
}

/*
 * uint_to_str
 * Konverter unsigned integer til string
 */
static char* uint_to_str(uint32_t value)
{
    char *p = num_buf + 11;
    *p = '\0';

    if (value == 0) {
        *(--p) = '0';
    } else {
        while (value > 0) {
            *(--p) = '0' + (value % 10);
            value /= 10;
        }
    }

    return p;
}

/*
 * pad_string
 * Højrejuster string med mellemrum
 */
static void pad_right(char *dest, const char *src, uint8_t width)
{
    uint8_t len = strlen(src);
    uint8_t pad = (len < width) ? (width - len) : 0;

    /* Fyld med mellemrum */
    for (uint8_t i = 0; i < pad; i++) {
        dest[i] = ' ';
    }

    /* Kopier string */
    strcpy(dest + pad, src);
}

/* ===== Integer Math Implementations ===== */

/*
 * isqrt
 * Integer kvadratrod med Newton-Raphson iteration
 * Konvergerer typisk i 5-6 iterationer for 32-bit input
 */
uint16_t isqrt(uint32_t n)
{
    if (n == 0) return 0;
    if (n == 1) return 1;

    uint32_t x = n;
    uint32_t y = (x + 1) / 2;

    while (y < x) {
        x = y;
        y = (x + n / x) / 2;
    }

    return (uint16_t)x;
}

/*
 * fast_atan2_deg
 * Hurtig atan2 approksimation der returnerer grader
 *
 * Bruger lineær approksimation:
 * - For |y| <= |x|: atan(y/x) ≈ 45° * (y/x)
 * - For |y| > |x|: atan(y/x) ≈ 90° - 45° * (x/y)
 *
 * Derefter justeres for korrekt kvadrant
 */
int16_t fast_atan2_deg(int32_t y, int32_t x)
{
    /* Håndter special cases */
    if (x == 0 && y == 0) return 0;
    if (x == 0) return (y > 0) ? 90 : -90;
    if (y == 0) return (x > 0) ? 0 : 180;

    int16_t angle;
    int32_t abs_y = (y < 0) ? -y : y;
    int32_t abs_x = (x < 0) ? -x : x;

    /*
     * Beregn vinkel i første oktant (0-45°)
     * Skaler med 64 for bedre præcision
     */
    if (abs_x >= abs_y) {
        /* |vinkel| <= 45° */
        int32_t ratio = (abs_y * 64) / abs_x;
        angle = (int16_t)((ratio * 45) / 64);
    } else {
        /* |vinkel| > 45° */
        int32_t ratio = (abs_x * 64) / abs_y;
        angle = 90 - (int16_t)((ratio * 45) / 64);
    }

    /* Juster for kvadrant */
    if (x < 0) {
        angle = 180 - angle;
    }
    if (y < 0) {
        angle = -angle;
    }

    return angle;
}

/* ===== Debug Screen Implementations ===== */

/*
 * draw_screen_adc
 * Vis ADC debug information
 */
static void draw_screen_adc(void)
{
    char line[17];

    /* Titel */
    sendStrXY("== ADC DEBUG ==", 0, 0);

    /* Rå ADC værdi */
    sendStrXY("Raw:      ", 1, 0);
    pad_right(line, uint_to_str(debug_adc_last), 5);
    sendStrXY(line, 1, 5);

    /* Minimum værdi */
    sendStrXY("Min:      ", 2, 0);
    pad_right(line, uint_to_str(debug_adc_min), 5);
    sendStrXY(line, 2, 5);

    /* Maximum værdi */
    sendStrXY("Max:      ", 3, 0);
    pad_right(line, uint_to_str(debug_adc_max), 5);
    sendStrXY(line, 3, 5);

    /* Range (max - min) */
    sendStrXY("Range:    ", 4, 0);
    uint16_t range = debug_adc_max - debug_adc_min;
    pad_right(line, uint_to_str(range), 5);
    sendStrXY(line, 4, 7);

    /* Sample counter (vis kun laveste 6 cifre) */
    sendStrXY("Count:    ", 5, 0);
    uint32_t count_display = debug_adc_count % 1000000UL;
    pad_right(line, uint_to_str(count_display), 6);
    sendStrXY(line, 5, 7);

    /* Forventet range info */
    sendStrXY("10-bit: 0-1023", 7, 0);
}

/*
 * draw_screen_dft_accum
 * Vis DFT akkumulator debug information
 */
static void draw_screen_dft_accum(void)
{
    char line[17];

    /* Titel */
    sendStrXY("= DFT ACCUM =", 0, 0);

    /* Real komponent */
    sendStrXY("Re:         ", 1, 0);
    pad_right(line, int_to_str(debug_dft_re), 7);
    sendStrXY(line, 1, 4);

    /* Imaginær komponent */
    sendStrXY("Im:         ", 2, 0);
    pad_right(line, int_to_str(debug_dft_im), 7);
    sendStrXY(line, 2, 4);

    /* DFT completion count */
    sendStrXY("DFT#:       ", 3, 0);
    uint32_t dft_display = debug_dft_count % 100000UL;
    pad_right(line, uint_to_str(dft_display), 6);
    sendStrXY(line, 3, 6);

    /* Nuværende sample index */
    sendStrXY("Index:      ", 4, 0);
    pad_right(line, uint_to_str(debug_dft_index), 3);
    sendStrXY(line, 4, 7);
    sendStrXY("/64", 4, 10);

    /* Status indikator */
    sendStrXY("Window: 64 samp", 6, 0);
    sendStrXY("4x oversample", 7, 0);
}

/*
 * draw_screen_dft_result
 * Vis DFT resultat: magnitude, fase, klassifikation
 */
static void draw_screen_dft_result(void)
{
    char line[17];

    /* Titel */
    sendStrXY("= DFT RESULT =", 0, 0);

    /* Beregn magnitude fra Re og Im */
    int32_t re_scaled = debug_dft_re / 16;
    int32_t im_scaled = debug_dft_im / 16;
    uint32_t mag_sq = (uint32_t)(re_scaled * re_scaled + im_scaled * im_scaled);
    uint16_t magnitude = isqrt(mag_sq);

    /* Vis magnitude */
    sendStrXY("Mag:        ", 1, 0);
    pad_right(line, uint_to_str(magnitude), 5);
    sendStrXY(line, 1, 5);

    /* Beregn fase */
    int16_t phase = fast_atan2_deg(debug_dft_im, debug_dft_re);

    /* Vis fase */
    sendStrXY("Phase:      ", 2, 0);
    pad_right(line, int_to_str(phase), 4);
    sendStrXY(line, 2, 7);
    sendStrXY("deg", 2, 12);

    /* Klassifikation baseret på fase og magnitude */
    sendStrXY("Type:           ", 4, 0);

    #define MAG_THRESHOLD 20

    if (magnitude < MAG_THRESHOLD) {
        sendStrXY("NO SIGNAL", 4, 6);
    } else if (phase < 65) {
        sendStrXY("FERROUS  ", 4, 6);
    } else {
        sendStrXY("NON-FERRO", 4, 6);
    }

    /* Vis threshold info */
    sendStrXY("Thresh: 65 deg", 6, 0);
    sendStrXY("MinMag: 20", 7, 0);
}

/*
 * draw_screen_timing
 * Vis timing og synkronisering information
 */
static void draw_screen_timing(void)
{
    char line[17];

    /* Titel */
    sendStrXY("== TIMING ==", 0, 0);

    /* TX frekvens (konstant) */
    sendStrXY("TX: 2000 Hz", 1, 0);

    /* Sample rate (konstant) */
    sendStrXY("Fs: 8000 Hz", 2, 0);

    /* Samples per TX cycle */
    sendStrXY("Samp/cyc: 4", 3, 0);

    /* ISR execution count */
    sendStrXY("ISR#:       ", 4, 0);
    uint32_t isr_display = debug_isr_count % 1000000UL;
    pad_right(line, uint_to_str(isr_display), 6);
    sendStrXY(line, 4, 6);

    /* Sync status (baseret på om DFT producerer resultater) */
    sendStrXY("Sync: ", 5, 0);
    if (debug_dft_count > 0 && debug_isr_count > 0) {
        sendStrXY("OK    ", 5, 6);
    } else {
        sendStrXY("CHECK ", 5, 6);
    }

    /* Debug pin info */
    sendStrXY("Pin13: Toggle", 6, 0);
    sendStrXY("Scope: 4kHz", 7, 0);
}

/* ===== Public API Functions ===== */

/*
 * debug_init
 * Initialiser debug system
 */
void debug_init(void)
{
    /* Reset alle debug variabler */
    debug_adc_last = 0;
    debug_adc_min = 1023;
    debug_adc_max = 0;
    debug_adc_count = 0;

    debug_dft_re = 0;
    debug_dft_im = 0;
    debug_dft_count = 0;
    debug_dft_index = 0;

    debug_isr_count = 0;

    debug_current_screen = DEBUG_SCREEN_ADC;
    debug_enabled = 0;

    /* Konfigurer debug pin */
    debug_pin_init();
}

/*
 * debug_pin_init
 * Konfigurer Pin 13 (PB7) som output for timing verifikation
 */
void debug_pin_init(void)
{
    DEBUG_PIN_DDR |= (1 << DEBUG_PIN_BIT);
    DEBUG_PIN_PORT &= ~(1 << DEBUG_PIN_BIT);
}

/*
 * debug_update
 * Opdater debug display baseret på aktuel skærm
 */
void debug_update(void)
{
    if (!debug_enabled) return;

    switch (debug_current_screen) {
        case DEBUG_SCREEN_ADC:
            draw_screen_adc();
            break;

        case DEBUG_SCREEN_DFT_ACCUM:
            draw_screen_dft_accum();
            break;

        case DEBUG_SCREEN_DFT_RESULT:
            draw_screen_dft_result();
            break;

        case DEBUG_SCREEN_TIMING:
            draw_screen_timing();
            break;

        default:
            debug_current_screen = DEBUG_SCREEN_ADC;
            break;
    }
}

/*
 * debug_next_screen
 * Skift til næste debug skærm
 */
void debug_next_screen(void)
{
    debug_current_screen++;
    if (debug_current_screen >= DEBUG_SCREEN_COUNT) {
        debug_current_screen = DEBUG_SCREEN_ADC;
    }

    /* Ryd display ved skærmskift */
    clear_display();
}

/*
 * debug_toggle
 * Slå debug mode til/fra
 */
void debug_toggle(void)
{
    debug_enabled = !debug_enabled;

    if (debug_enabled) {
        /* Nulstil min/max ved aktivering */
        debug_adc_min = 1023;
        debug_adc_max = 0;
        clear_display();
    }
}

/*
 * debug_log_adc_sample
 * Log ADC sample til debug statistik
 * BEMÆRK: Kaldt fra ISR - skal være hurtig!
 */
void debug_log_adc_sample(uint16_t sample)
{
    debug_adc_last = sample;
    debug_adc_count++;
    debug_isr_count++;

    /* Opdater min/max */
    if (sample < debug_adc_min) {
        debug_adc_min = sample;
    }
    if (sample > debug_adc_max) {
        debug_adc_max = sample;
    }
}

/*
 * debug_log_dft_done
 * Log færdig DFT resultat
 */
void debug_log_dft_done(int32_t re, int32_t im)
{
    debug_dft_re = re;
    debug_dft_im = im;
    debug_dft_count++;
}

/*
 * debug_log_dft_index
 * Log nuværende DFT sample index
 */
void debug_log_dft_index(uint8_t index)
{
    debug_dft_index = index;
}
