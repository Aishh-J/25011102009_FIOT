#include <reg51.h>

#define LED_PORT P3

unsigned char code seg_code[10] =
{
    0x3F,
    0x06,
    0x5B,
    0x4F,
    0x66,
    0x6D,
    0x7D,
    0x07,
    0x7F,
    0x6F
};

void delay_ms(unsigned int ms)
{
    unsigned int i, j;

    for(i = 0; i < ms; i++)
    {
        for(j = 0; j < 120; j++)
        {
            ;
        }
    }
}

void external_led(unsigned char pattern)
{
    P3 = (P3 & 0xC3) | ((pattern & 0x0F) << 2);
}

unsigned char keypad_scan(void)
{
    unsigned char col;

    while(1)
    {
        P2 = 0x7F;

        if((P2 & 0x0F) != 0x0F)
        {
            delay_ms(20);
            col = P2 & 0x0F;

            if(col != 0x0F)
            {
                while((P2 & 0x0F) != 0x0F);

                if(col == 0x07) return '1';
                if(col == 0x0B) return '2';
                if(col == 0x0D) return '3';
                if(col == 0x0E) return '4';
            }
        }

        P2 = 0xBF;

        if((P2 & 0x0F) != 0x0F)
        {
            delay_ms(20);
            col = P2 & 0x0F;

            if(col != 0x0F)
            {
                while((P2 & 0x0F) != 0x0F);

                if(col == 0x07) return '5';
                if(col == 0x0B) return '6';
                if(col == 0x0D) return '7';
                if(col == 0x0E) return '8';
            }
        }

        P2 = 0xDF;

        if((P2 & 0x0F) != 0x0F)
        {
            delay_ms(20);
            col = P2 & 0x0F;

            if(col != 0x0F)
            {
                while((P2 & 0x0F) != 0x0F);

                if(col == 0x07) return '9';
                if(col == 0x0B) return '0';
                if(col == 0x0D) return 'A';
                if(col == 0x0E) return 'B';
            }
        }

        P2 = 0xEF;

        if((P2 & 0x0F) != 0x0F)
        {
            delay_ms(20);
            col = P2 & 0x0F;

            if(col != 0x0F)
            {
                while((P2 & 0x0F) != 0x0F);

                if(col == 0x07) return 'C';
                if(col == 0x0B) return 'D';
                if(col == 0x0D) return 'E';
                if(col == 0x0E) return 'F';
            }
        }
    }
}

void display_digit(unsigned char value)
{
    if(value <= 9)
    {
        P0 = seg_code[value];
    }
}

void running_led(void)
{
    unsigned char i;

    for(i = 0; i < 3; i++)
    {
        external_led(0x01);
        delay_ms(300);

        external_led(0x02);
        delay_ms(300);

        external_led(0x04);
        delay_ms(300);

        external_led(0x08);
        delay_ms(300);
    }

    external_led(0x00);
}

void binary_counter(void)
{
    unsigned char count;

    for(count = 0; count <= 15; count++)
    {
        external_led(count);
        delay_ms(500);
    }

    external_led(0x00);
}

void alternate_led(void)
{
    unsigned char i;

    for(i = 0; i < 6; i++)
    {
        external_led(0x0A);
        delay_ms(400);

        external_led(0x05);
        delay_ms(400);
    }

    external_led(0x00);
}

void blink_all(void)
{
    unsigned char i;

    for(i = 0; i < 5; i++)
    {
        external_led(0x0F);
        delay_ms(400);

        external_led(0x00);
        delay_ms(400);
    }
}

void main(void)
{
    unsigned char key;

    P0 = seg_code[0];
    P2 = 0xFF;
    P3 = 0x00;

    while(1)
    {
        key = keypad_scan();

        if(key >= '0' && key <= '9')
        {
            display_digit(key - '0');
        }

        switch(key)
        {
            case '1':
                external_led(0x01);
                break;

            case '2':
                external_led(0x02);
                break;

            case '3':
                external_led(0x04);
                break;

            case '4':
                external_led(0x08);
                break;

            case '5':
                running_led();
                break;

            case '6':
                alternate_led();
                break;

            case '7':
                binary_counter();
                break;

            case '8':
                blink_all();
                break;

            case '9':
                external_led(0x00);
                break;

            case '0':
                external_led(0x00);
                P0 = seg_code[0];
                break;

            default:
                break;
        }
    }
}