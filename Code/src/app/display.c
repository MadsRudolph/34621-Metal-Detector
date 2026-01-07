/*
 * display.c
 * Simpel wrapper til OLED display
 *
 * Wrapper funktioner der kalder SSD1306 driveren.
 * Håndterer I2C initialisering og giver simpel tekst API.
 */

#include "display.h"
#include "../drivers/I2C.h"
#include "../drivers/ssd1306.h"

/* Brug interne SSD1306 funktioner til bar grafik */
extern void setXY(unsigned char row, unsigned char col);
extern void SendChar(unsigned char data);

/*
 * display_init
 * Initialiserer I2C bus og OLED display
 */
void display_init(void)
{
    I2C_Init();           /* Start I2C bus */
    InitializeDisplay();  /* Konfigurer OLED */
    clear_display();      /* Start med blank skærm */
}

/*
 * display_clear
 * Rydder hele displayet
 */
void display_clear(void)
{
    clear_display();
}

/*
 * display_text
 * Udskriver tekst på given position
 * sendStrXY forventer: (string, række, kolonne)
 */
void display_text(uint8_t row, uint8_t col, const char *text)
{
    sendStrXY((char *)text, row, col);
}

/*
 * display_number
 * Konverterer tal til string og udskriver på display
 * Bruger baglæns konvertering (højre-til-venstre)
 */
void display_number(uint8_t row, uint8_t col, uint16_t value)
{
    char buf[6];  /* Max "65535" + null terminator */
    char *p = buf + 5;

    *p = '\0';  /* Null terminator sidst */

    /* Konverter cifre baglæns */
    do {
        *(--p) = '0' + (value % 10);  /* Udtræk sidste ciffer */
        value /= 10;                   /* Fjern sidste ciffer */
    } while (value > 0);

    display_text(row, col, p);
}

/*
 * display_solid_block
 * Tegner en solid blok (8x8 pixels fyldt)
 */
static void display_solid_block(uint8_t row, uint8_t col)
{
    setXY(row, col);
    for (uint8_t i = 0; i < 8; i++) {
        SendChar(0xFF);  /* Alle pixels tændt */
    }
}

/*
 * display_empty_block
 * Tegner en tom blok (8x8 pixels slukket)
 */
static void display_empty_block(uint8_t row, uint8_t col)
{
    setXY(row, col);
    for (uint8_t i = 0; i < 8; i++) {
        SendChar(0x00);  /* Alle pixels slukket */
    }
}

/*
 * display_bar
 * Tegner en horisontal bar med given værdi (0-100)
 * Bruger solide blokke for fyldt, tomme for resten
 */
void display_bar(uint8_t row, uint8_t col, uint8_t width, uint8_t value)
{
    if (width > 16) width = 16;
    if (value > 100) value = 100;

    /* Beregn antal fyldte positioner */
    uint8_t filled = (value * width) / 100;

    for (uint8_t i = 0; i < width; i++) {
        if (i < filled) {
            display_solid_block(row, col + i);
        } else {
            display_empty_block(row, col + i);
        }
    }
}

/*
 * screen_splash
 * Viser velkomst skærm ved opstart
 */
void screen_splash(void)
{
    display_clear();
    display_text(1, 1, "METAL DETEKTOR");
    display_text(3, 3, "DTU 34621");
    display_text(5, 2, "VLF 2kHz");
    display_text(7, 1, "Initialiserer...");
}

/*
 * screen_idle
 * Viser idle skærm når detektor er stoppet
 */
void screen_idle(void)
{
    display_clear();
    display_text(0, 0, "METAL DETEKTOR");
    display_text(1, 0, "----------------");
    display_text(3, 0, "Status: IDLE");
    display_text(5, 0, "Pin2: Start");
    display_text(6, 0, "Pin3: Kalibrer");
}

/*
 * screen_calibrating
 * Vises under kalibrering
 */
void screen_calibrating(void)
{
    display_clear();
    display_text(0, 0, "KALIBRERING");
    display_text(1, 0, "----------------");
    display_text(3, 0, "Hold spolen vaek");
    display_text(4, 0, "fra metal...");
    display_text(6, 0, "Vent venligst");
}

/*
 * screen_detection
 * Hoved detektion skærm med live data
 * metal_type: 0=NONE, 1=FERRO, 2=NON-FERRO
 */
void screen_detection(uint8_t strength, int16_t phase, uint8_t metal_type)
{
    /* Række 0: Titel */
    display_text(0, 0, "== DETEKTION ==");

    /* Række 2: Styrke bar */
    display_text(2, 0, "Styrke:");
    display_bar(2, 8, 8, strength);

    /* Række 3: Styrke % */
    display_text(3, 0, "       ");
    display_number(3, 0, strength);
    display_text(3, 4, "%");

    /* Række 4: Fase */
    display_text(4, 0, "Fase:        ");
    if (phase < 0) {
        display_text(4, 6, "-");
        display_number(4, 7, -phase);
    } else {
        display_number(4, 6, phase);
    }
    display_text(4, 11, "deg");

    /* Række 6: Metal type */
    display_text(6, 0, "Type:           ");
    switch (metal_type) {
        case 0:
            display_text(6, 6, "---");
            break;
        case 1:
            display_text(6, 6, "FERRO");
            break;
        case 2:
            display_text(6, 6, "NON-FERRO");
            break;
    }
}
