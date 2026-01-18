/*
 * display.c - OLED display funktioner for Metal Detektor
 *
 * Viser DFT resultater og debug information på SSD1306 OLED display
 */

#include <stdio.h>
#include "display.h"
#include "drivers/ssd1306.h"
#include "dft.h"
#include "filter.h"
#include "detection.h"
#include "adc.h"

/* ============ GLOBALE VARIABLE ============ */
uint8_t show_debug = 0;         // Start med DFT visning

// Lokal buffer til tekstformatering
static char buf[20];

/* ============ HELPER: SIGNAL BAR ============ */
static void make_signal_bar(char *out, uint16_t value, uint16_t max_val) {
    // Lav en 10-tegn bargraf: [========  ]
    uint8_t filled = 0;
    if (max_val > 0 && value > 0) {
        filled = (value * 10) / max_val;
        if (filled > 10) filled = 10;
    }

    out[0] = '[';
    for (uint8_t i = 0; i < 10; i++) {
        out[i + 1] = (i < filled) ? '=' : ' ';
    }
    out[11] = ']';
    out[12] = '\0';
}

/* ============ DFT DISPLAY ============ */
void display_dft(void) {
    // Vis pause-besked hvis detektor er stoppet
    if (!detection_active) {
        sendStrXY("                ", 0, 0);
        sendStrXY("    PAUSED      ", 2, 0);
        sendStrXY("                ", 3, 0);
        sendStrXY("  Tryk D2 for   ", 5, 0);
        sendStrXY("  at fortsaette ", 6, 0);
        sendStrXY("                ", 7, 0);
        return;
    }

    // Header
    sendStrXY("METAL DETEKTOR  ", 0, 0);

    // Signal styrke bar (baseret på magnitude relativt til kalibrering)
    uint16_t signal_strength = 0;
    uint16_t bar_max = 200;  // Max værdi for fuld bar

    if (is_calibrated && mag_filtered > cal_mag) {
        signal_strength = mag_filtered - cal_mag;
    } else if (!is_calibrated) {
        signal_strength = mag_filtered / 4;  // Skalér ned før kalibrering
        bar_max = 100;
    }

    char bar[14];
    make_signal_bar(bar, signal_strength, bar_max);
    sprintf(buf, "Sig: %s", bar);
    sendStrXY(buf, 2, 0);

    // Magnitude og fase på samme linje
    sprintf(buf, "M:%-5lu  F:%-4d", mag_filtered, ang_filtered);
    sendStrXY(buf, 3, 0);

    // Separator
    sendStrXY("----------------", 4, 0);

    // Metal type - stor og tydelig visning
    uint8_t metal = classify_metal();

    if (metal == METAL_FERRO) {
        sendStrXY("                ", 5, 0);
        sendStrXY("  ** FERRO **   ", 6, 0);
        sendStrXY("   (Magnetisk)  ", 7, 0);
    } else if (metal == METAL_NONFERRO) {
        sendStrXY("                ", 5, 0);
        sendStrXY(" ** NON-FERRO **", 6, 0);
        sendStrXY(" (Ikke-magnet.) ", 7, 0);
    } else {
        // Intet metal - vis kalibreringsstatus
        if (!is_calibrated) {
            sendStrXY("  Tryk D3 for   ", 5, 0);
            sendStrXY("  kalibrering   ", 6, 0);
            sendStrXY("                ", 7, 0);
        } else {
            sendStrXY("                ", 5, 0);
            sendStrXY("   [ Soeger ]   ", 6, 0);
            sendStrXY("                ", 7, 0);
        }
    }
}

/* ============ DEBUG DISPLAY ============ */
void display_debug(void) {
    sendStrXY("=== DEBUG ===   ", 0, 0);

    // Rå ADC værdi (0-1023)
    sprintf(buf, "ADC:  %-9d", ADC_Raw);
    sendStrXY(buf, 2, 0);

    // Ufiltreret magnitude og fase
    sprintf(buf, "Mag:  %-9u", mag);
    sendStrXY(buf, 3, 0);

    sprintf(buf, "Fase: %-9d", ang);
    sendStrXY(buf, 4, 0);

    // Kalibrerings baseline værdier
    if (is_calibrated) {
        sprintf(buf, "Cal:  M%-4u A%-3d", cal_mag, cal_ang);
        sendStrXY(buf, 5, 0);

        // Delta fra kalibrering (til fejlfinding)
        int16_t d_mag = (int16_t)mag_filtered - (int16_t)cal_mag;
        int16_t d_ang = ang_filtered - cal_ang;
        sprintf(buf, "dM:%-5d dA:%-4d", d_mag, d_ang);
        sendStrXY(buf, 6, 0);
    } else {
        sendStrXY("Cal:  ---       ", 5, 0);
        sendStrXY("Ikke kalibreret ", 6, 0);
    }

    // Status linje
    sprintf(buf, "%s %s      ",
            detection_active ? "RUN" : "PAU",
            is_calibrated ? "CAL" : "---");
    sendStrXY(buf, 7, 0);
}

/* ============ KALIBRERINGS BESKED ============ */
void display_calibrated(void) {
    clear_display();
    sendStrXY("KALIBRERET!     ", 3, 0);
}

/* ============ TOGGLE DISPLAY MODE ============ */
void toggle_display_mode(void) {
    show_debug = !show_debug;
}
