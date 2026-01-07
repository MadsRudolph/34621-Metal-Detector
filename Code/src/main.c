#include <avr/io.h>
#include "signal/tx.h"
#include "drivers/I2C.h"
#include "drivers/ssd1306.h"


int main(void) {
    // TX 2kHz PWM init (Pin 11 / OC1A)
    timer1_init();

    // OLED init (I2C: SDA=Pin20, SCL=Pin21)
    I2C_Init();
    InitializeDisplay();
    clear_display();

    // Splash screen
    sendStrXY("Metal Detector", 0, 0);
    sendStrXY("DTU 34621", 2, 3);
    sendStrXY("Initializing...", 4, 0);

    while (1) {
        /* PWM runs in hardware */
        /* OLED shows splash screen */



    }
    return 0;
}
