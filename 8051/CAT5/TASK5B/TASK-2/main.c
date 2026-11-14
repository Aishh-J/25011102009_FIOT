#include <reg52.h> 

unsigned char pwm_count = 0; 
unsigned char duty = 0;
unsigned char fade_count = 0;

// Default Timer 1 reload values (1 kHz output)
unsigned char reload_high = 0xFE;
unsigned char reload_low  = 0x33;

sbit LED    = P1^0;
sbit OUTPUT = P1^1;

// Timer 0 ISR for LED fading
void timer0_ISR(void) interrupt 1 {
    TH0 = 0xFF;
    TL0 = 0xF7;

    if(pwm_count < duty) {
        LED = 1;
    } else {
        LED = 0;
    }

    pwm_count++;
    if(pwm_count >= 100) {
        pwm_count = 0;
        fade_count++;

        if(fade_count >= 10) {
            duty++;
            fade_count = 0;
            if(duty > 100) {
                duty = 0;
            }
        }
    }
}

// Keypad matrix scanner on Port P2
int keypad_scan(void)
{
    static const int keypad[4][4] =
    {
        {1, 2, 3, 4},
        {5, 6, 7, 8},
        {9, -1, -1, -1},
        {-1, -1, -1, -1}
    };

    static const unsigned char rowPin[4] = {7, 6, 5, 4};
    static const unsigned char colPin[4] = {3, 2, 1, 0};

    int row, col;

    for(row = 0; row < 4; row++)
    {
        // Set all rows HIGH
        P2 |= 0xF0;

        // Set current row LOW
        P2 &= ~(1 << rowPin[row]);

        // Check columns
        for(col = 0; col < 4; col++)
        {
            if((P2 & (1 << colPin[col])) == 0)
            {
                return keypad[row][col];
            }
        }
    }

    return -1;
}

// Frequency selection based on keypad press
void set_frequency(unsigned char mode)
{
    switch(mode)
    {
        case 1: // Mode 1: 100 Hz
            reload_high = 0xEE;
            reload_low  = 0x00;
            break;

        case 2: // Mode 2: 500 Hz
            reload_high = 0xFC;
            reload_low  = 0x66;
            break;

        case 3: // Mode 3: 1 kHz
            reload_high = 0xFE;
            reload_low  = 0x33;
            break;

        case 4: // Mode 4: 2 kHz
            reload_high = 0xFF;
            reload_low  = 0x1A;
            break;

        case 5: // Mode 5: 5 kHz
            reload_high = 0xFF;
            reload_low  = 0xA4;
            break;

        default:
            break;
    }
}

// Timer 1 ISR for square wave toggling on P1^1
void timer1_ISR(void) interrupt 3 {
    TH1 = reload_high;
    TL1 = reload_low;

    OUTPUT = ~OUTPUT;
}

void main() {
    int key;

    TMOD = 0x11; // Timer 0 (Mode 1, 16-bit) and Timer 1 (Mode 1, 16-bit)
    
    TH0 = 0xFF;
    TL0 = 0xF7;

    TH1 = reload_high;
    TL1 = reload_low;
    
    ET0 = 1; // Enable Timer 0 Interrupt
    ET1 = 1; // Enable Timer 1 Interrupt
    EA  = 1; // Enable Global Interrupts

    TR0 = 1; // Start Timer 0
    TR1 = 1; // Start Timer 1

    while(1) {
        key = keypad_scan();
        if(key != -1) {
            set_frequency(key);
        }
    }
}