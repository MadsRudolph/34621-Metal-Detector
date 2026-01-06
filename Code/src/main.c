#include <avr/io.h>
#include "signal/tx.h"

int main(void) {
    timer1_init();

    while (1) {
        /* PWM runs in hardware */
    }
    return 0;
}
