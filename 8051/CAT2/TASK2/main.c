#include <reg51.h>

sbit LED = P1^0;

void delay(void) {
    unsigned int i, j;
    for (i = 0; i < 500; i++) {
        for (j = 0; j < 500; j++) {
        }
    }
}

void main(void) {
    P1 = 0x00; // Initialize Port 1 pins to LOW state

    while (1) {
        LED = 1;      // Turn LED ON 
        delay();
        LED = 0;      // Turn LED OFF 
        delay();
    }
}