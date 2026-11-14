#include <reg52.h>

// Global variables for Timer 0 (LED Fading)
unsigned char pwm_count = 0;
unsigned char duty = 0;
unsigned char fade_count = 0;
unsigned char direction = 1;

// Global variables for Timer 1 (Waveform Generator)
unsigned char waveform_mode = 1; // 1: Square, 2: Var Duty (25%), 3: Pulse Train, 4: Burst Signal
unsigned char wave_tick = 0;

sbit LED    = P1^0; // Output pin for LED Fading
sbit OUTPUT = P1^1; // Output pin for Waveforms

// Timer 0 ISR: Software PWM LED Fading (100 us tick base)
void timer0_ISR(void) interrupt 1 
{
    TH0 = 0xFF;
    TL0 = 0xF7;

    if (pwm_count < duty) {
        LED = 1;
    } else {
        LED = 0;
    }

    pwm_count++;
    if (pwm_count >= 100) {
        pwm_count = 0;
        fade_count++;

        if (fade_count >= 10) {
            fade_count = 0;
            if (direction == 1) {
                duty += 10;
                if (duty >= 100) direction = 0;
            } else {
                duty -= 10;
                if (duty == 0) direction = 1;
            }
        }
    }
}

// Timer 1 ISR: Multi-Waveform Signal Generator (1 ms tick base)
void timer1_ISR(void) interrupt 3 
{
    TH1 = 0xFC;
    TL1 = 0x66;

    switch (waveform_mode)
    {
        case 1: // 1. Standard Square Wave (50% Duty Cycle)
            OUTPUT = ~OUTPUT;
            break;

        case 2: // 2. Variable Duty-Cycle Wave (25% HIGH, 75% LOW over 8 ticks)
            if (wave_tick < 2) {
                OUTPUT = 1;
            } else {
                OUTPUT = 0;
            }
            wave_tick++;
            if (wave_tick >= 8) wave_tick = 0;
            break;

        case 3: // 3. Pulse Train (10000000 pattern over 8 ticks)
            if (wave_tick == 0) {
                OUTPUT = 1;
            } else {
                OUTPUT = 0;
            }
            wave_tick++;
            if (wave_tick >= 8) wave_tick = 0;
            break;

        case 4: // 4. Burst Signal (10 toggling pulses followed by 10 silent ticks)
            if (wave_tick < 10) {
                OUTPUT = (wave_tick % 2 == 0) ? 1 : 0;
            } else {
                OUTPUT = 0;
            }
            wave_tick++;
            if (wave_tick >= 20) wave_tick = 0;
            break;

        default:
            OUTPUT = 0;
            break;
    }
}

// Non-blocking 4x4 Keypad Scan on Port P2
int keypad_scan(void)
{
    static const int keypad[4][4] = {
        {1, 2, 3, 4},
        {5, 6, 7, 8},
        {9, -1, -1, -1},
        {-1, -1, -1, -1}
    };
    static const unsigned char rowPin[4] = {7, 6, 5, 4};
    static const unsigned char colPin[4] = {3, 2, 1, 0};
    int row, col;

    for (row = 0; row < 4; row++)
    {
        P2 |= 0xF0;
        P2 &= ~(1 << rowPin[row]);

        for (col = 0; col < 4; col++)
        {
            if ((P2 & (1 << colPin[col])) == 0)
            {
                return keypad[row][col];
            }
        }
    }
    return -1;
}

void main(void)
{
    int key;

    // Configure Timer 0 (Mode 1, 16-bit) and Timer 1 (Mode 1, 16-bit)
    TMOD = 0x11;

    // Preload Timer 0
    TH0 = 0xFF;
    TL0 = 0xF7;

    // Preload Timer 1 (1 ms ticks)
    TH1 = 0xFC;
    TL1 = 0x66;

    ET0 = 1; // Enable Timer 0 Interrupt
    ET1 = 1; // Enable Timer 1 Interrupt
    EA  = 1; // Enable Global Interrupts

    TR0 = 1; // Start Timer 0
    TR1 = 1; // Start Timer 1

    while (1)
    {
        key = keypad_scan();
        if (key >= 1 && key <= 4)
        {
            waveform_mode = key;
            wave_tick = 0; // Reset state counter on mode change
        }
    }
}