#include <reg52.h>

sbit LED = P1^0;

unsigned char pwm_count = 0;
unsigned char duty = 0;
bit increasing = 1;

/*
   Timer 0 interrupt frequency = 10 kHz

   Crystal = 11.0592 MHz
   Machine cycle = 11.0592 MHz / 12
                 = 921.6 kHz

   Timer tick = 1 / 921600
              = 1.085 us

   Required interrupt time = 100 us

   Counts = 100 us / 1.085 us
          ˜ 92

   Reload = 65536 - 92
          = 65444
          = 0xFFD4
*/

void Timer0_ISR(void) interrupt 1
{
    /* Reload Timer 0 */
    TH0 = 0xFF;
    TL0 = 0xD4;

    /*
       Software PWM

       pwm_count: 0 to 99
       duty:      0 to 100
    */

    if (pwm_count < duty)
        LED = 1;
    else
        LED = 0;

    pwm_count++;

    /*
       One complete PWM cycle = 100 interrupts
       100 interrupts × 100 us = 10 ms
    */

    if (pwm_count >= 100)
    {
        pwm_count = 0;

        /* Increase brightness */
        if (increasing)
        {
            if (duty < 100)
                duty++;
            else
                increasing = 0;
        }

        /* Decrease brightness */
        else
        {
            if (duty > 0)
                duty--;
            else
                increasing = 1;
        }
    }
}


void Timer0_Init(void)
{
    /*
       Timer 0 Mode 1
       16-bit timer
    */

    TMOD &= 0xF0;
    TMOD |= 0x01;

    /* Initial timer value */
    TH0 = 0xFF;
    TL0 = 0xD4;

    /* Enable Timer 0 interrupt */
    ET0 = 1;

    /* Enable global interrupts */
    EA = 1;

    /* Start Timer 0 */
    TR0 = 1;
}


void main(void)
{
    LED = 0;

    Timer0_Init();

    while (1)
    {
        /* Everything is handled by Timer 0 ISR */
    }
}