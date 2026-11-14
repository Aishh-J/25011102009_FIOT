
#include <REGX52.H>

void uart_init(void);
void uart_putchar(unsigned char c);
unsigned char uart_getchar(void);

void main(void)
{
    unsigned char received;

    uart_init();

    while (1)
    {
        received = uart_getchar();   /* wait for a character from PuTTY */
        uart_putchar(received);      /* echo it straight back           */
    }
}

/* Configure UART: Mode 1 (8-bit, variable baud), Timer 1 Mode 2
 * (auto-reload) generating 9600 baud at 11.0592 MHz. */
void uart_init(void)
{
    TMOD &= 0x0F;   /* clear Timer1 bits, keep Timer0 bits untouched */
    TMOD |= 0x20;   /* Timer1 -> Mode 2 (8-bit auto-reload)          */
    TH1 = 0xFD;      /* reload value for 9600 baud @ 11.0592 MHz     */
    TL1 = 0xFD;      /* initial counter value                        */
    TR1 = 1;         /* start Timer1                                 */
    SCON = 0x50;     /* Mode 1, REN=1 (reception enabled)            */
    TI = 0;
    RI = 0;
}

/* Send one byte and wait until the hardware finishes shifting it out. */
void uart_putchar(unsigned char c)
{
    SBUF = c;
    while (TI == 0);
    TI = 0;
}

/* Wait until a byte has been received, then return it. */
unsigned char uart_getchar(void)
{
    while (RI == 0);   /* wait until a full byte has arrived */
    RI = 0;             /* clear receive flag for next byte   */
    return SBUF;
}