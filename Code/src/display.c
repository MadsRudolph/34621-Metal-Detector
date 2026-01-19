/*
 * display.c - Grafisk HUD display for Metal Detektor
 *
 * Rent UI med grafisk signal bar, metaltype ikoner og værdier
 * Optimeret til at reducere flimmer ved kun at opdatere ændrede elementer
 */

#include <stdio.h>
#include "include/display.h"
#include "drivers/ssd1306.h"
#include "include/dft.h"
#include "include/filter.h"
#include "include/detection.h"
#include "include/adc.h"

// Eksternt ikon data (defineret i data.h, inkluderet af ssd1306.c)
extern const char icon_ferro[];
extern const char icon_nonferro[];
extern const char icon_search[];

/* ============ GLOBALE VARIABLE ============ */
uint8_t show_debug = 0;

// Lokal buffer til tekstformatering
static char buf[20];

// Tilstandssporing for at reducere flimmer (tegn kun når ændret)
static uint8_t last_metal = 255;      // Sidst viste metaltype
static uint8_t last_calibrated = 255; // Sidst viste kalibreringstilstand
static uint8_t last_active = 255;     // Sidst viste detection_active tilstand

/* ============ HOVED HUD DISPLAY ============ */
void display_dft(void) {
    // Tjek om detektionstilstand er ændret - kræver fuld gentegning
    if (last_active != detection_active) {
        last_active = detection_active;
        last_metal = 255;  // Tving ikon gentegning
        clear_display();
    }

    if (!detection_active) {
        // Pausetilstand - tegn kun én gang
        if (last_metal != 254) {  // Brug 254 som "pause" tilstand
            last_metal = 254;
            sendStrXY("================", 1, 0);
            sendStrXY("     PAUSE      ", 3, 0);
            sendStrXY("   [Tryk D2]    ", 5, 0);
            sendStrXY("================", 6, 0);
        }
        return;
    }

    // Beregn signalstyrke
    uint16_t signal = 0;
    uint8_t percent = 0;
    uint16_t bar_max = 50;

    if (is_calibrated && mag_filtered > cal_mag) {
        signal = mag_filtered - cal_mag;
        uint32_t pct = (signal * 100) / bar_max;
        percent = (pct > 100) ? 100 : pct;
    } else if (!is_calibrated) {
        signal = mag_filtered / 4;
        bar_max = 100;
        percent = (signal > 100) ? 100 : signal;
    }

    // Række 0: Grafisk fremgangsbar (opdater altid - den ændres ofte)
    draw_hbar(0, 0, 128, percent);

    // Række 1-4: Stor procent til venstre, ikon til højre
    // Store tal er 24px brede x 32px høje (4 sider), starter ved side 1
    // Formatér procent som 3 cifre (venstrejusteret)
    char pct_str[4];
    sprintf(pct_str, "%-3d", percent);

    // Tegn store procent cifre (side 1-4, kol 0-71)
    printBigNumber(pct_str[0], 1, 0);  // Hundreder ved kol 0
    printBigNumber(pct_str[1], 1, 3);  // Tiere ved kol 24
    printBigNumber(pct_str[2], 1, 6);  // Enere ved kol 48

    // Lille % symbol efter de store tal
    sendStrXY("%", 3, 9);  // Ved side 3, kol 9 (72px)

    // Række 1-3: Metaltype ikon til højre - tegn kun hvis ændret
    uint8_t metal = classify_metal();
    uint8_t cal_state = is_calibrated ? 1 : 0;

    // Tjek om vi skal gentegne ikonområdet
    if (metal != last_metal || cal_state != last_calibrated) {
        last_metal = metal;
        last_calibrated = cal_state;

        // Ryd ikonområde til højre (kol 88-120)
        fill_rect(1, 88, 40, 0x00);
        fill_rect(2, 88, 40, 0x00);
        fill_rect(3, 88, 40, 0x00);

        if (metal == METAL_FERRO) {
            draw_icon((const uint8_t *)icon_ferro, 1, 92);
            sendStrXY("  FERRO ", 4, 10);
        } else if (metal == METAL_NONFERRO) {
            draw_icon((const uint8_t *)icon_nonferro, 1, 92);
            sendStrXY("NON-FER ", 4, 10);
        } else {
            if (!is_calibrated) {
                draw_icon((const uint8_t *)icon_search, 1, 92);
                sendStrXY("[D3]CAL ", 4, 10);
            } else {
                sendStrXY(" SOGER  ", 4, 10);
            }
        }
    }

    // Række 5: Tom/mellemrum
    fill_rect(5, 0, 128, 0x00);

    // Række 6: Kun faseværdi (procent vises stort ovenover)
    sprintf(buf, "      Fase:%+4d", ang_filtered);
    sendStrXY(buf, 6, 0);

    // Række 7: Separator linje (tegn én gang efter tilstandsændring)
    static uint8_t separator_drawn = 0;
    if (!separator_drawn || last_active == detection_active) {
        sendStrXY("----------------", 7, 0);
        separator_drawn = 1;
    }
}

/* ============ DEBUG DISPLAY ============ */
void display_debug(void) {
    sendStrXY("---- DEBUG ---- ", 0, 0);

    sprintf(buf, "ADC:  %-9d", ADC_Raw);
    sendStrXY(buf, 1, 0);

    sprintf(buf, "Mag:  %-9u", mag);
    sendStrXY(buf, 2, 0);

    sprintf(buf, "Fase: %-9d", ang);
    sendStrXY(buf, 3, 0);

    if (is_calibrated) {
        sprintf(buf, "Cal: M%-4u A%-3d", cal_mag, cal_ang);
        sendStrXY(buf, 4, 0);

        int16_t d_mag = (int16_t)mag_filtered - (int16_t)cal_mag;
        int16_t d_ang = ang_filtered - cal_ang;
        sprintf(buf, "dM:%-4d dA:%-4d", d_mag, d_ang);
        sendStrXY(buf, 5, 0);
    } else {
        sendStrXY("Cal:  ---       ", 4, 0);
        sendStrXY("Ikke kalibreret ", 5, 0);
    }

    sprintf(buf, "%s  %s         ",
            detection_active ? "RUN" : "PAU",
            is_calibrated ? "CAL" : "---");
    sendStrXY(buf, 6, 0);

    sendStrXY("----------------", 7, 0);
}

/* ============ KALIBRERINGS BESKED ============ */
void display_calibrated(void) {
    clear_display();
    last_metal = 255;  // Tving gentegning efter kalibreringsskærm
    sendStrXY("================", 2, 0);
    sendStrXY("  KALIBRERET!   ", 3, 0);
    sendStrXY("================", 4, 0);
}

/* ============ SKIFT DISPLAY TILSTAND ============ */
void toggle_display_mode(void) {
    show_debug = !show_debug;
    last_metal = 255;  // Tving gentegning ved tilstandsskift
    last_active = 255;
}

/* ============ SPLASH SKÆRM ============ */
void display_splash(void) {
    clear_display();

    // Række 0: Øverste kant
    sendStrXY("################", 0, 0);

    // Række 1: Tom
    sendStrXY("#              #", 1, 0);

    // Række 2-3: Titel
    sendStrXY("#    METAL     #", 2, 0);
    sendStrXY("#   DETEKTOR   #", 3, 0);

    // Række 4: Tom
    sendStrXY("#              #", 4, 0);

    // Række 5: Version/DTU
    sendStrXY("#   DTU 34621  #", 5, 0);

    // Række 6: Tom
    sendStrXY("#              #", 6, 0);

    // Række 7: Nederste kant
    sendStrXY("################", 7, 0);
}
