
#include <REGX51.H>

void uart_init(void);
void uart_putchar(unsigned char c);
void uart_putstring(char *s);
void delay_ms(unsigned int ms);

void main(void)
{
    uart_init();

    while (1)
    {
        uart_putstring("Hello from 8051\r\n");
        delay_ms(500);
    }
}

void uart_init(void)
{
    TMOD &= 0x0F;   
    TMOD |= 0x20;   
    TH1 = 0xFD;      
    TL1 = 0xFD;
    TR1 = 1;         
    SCON = 0x50;     
    TI = 0;
    RI = 0;
}

void uart_putchar(unsigned char c)
{
    SBUF = c;
    while (TI == 0);
    TI = 0;
}

void uart_putstring(char *s)
{
    while (*s)
    {
        uart_putchar(*s);
        s++;
    }
}

void delay_ms(unsigned int ms)
{
    unsigned int i, j;
    for (i = 0; i < ms; i++)
        for (j = 0; j < 120; j++);
}