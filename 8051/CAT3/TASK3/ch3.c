#include <reg52.h>

sbit BUTTON = P2^0;
sbit P27=P2^7;


sbit LED1 = P1^0;
sbit LED2 = P1^1;
sbit LED3 = P1^2;
sbit LED4 = P1^3;



unsigned char count = 1;
unsigned char n;
unsigned int i, j;

void delay(void)
{
    for(i = 0; i < 250; i++)
    {
        for(j = 0; j < 250; j++);
    }
}

void mode1(void)
{
    LED1 = 1; delay(); LED1 = 0;
    LED2 = 1; delay(); LED2 = 0;
    LED3 = 1; delay(); LED3 = 0;
	  LED4 =1 ; delay(); LED4 = 0;
    
}

void mode2(void)
{
   for(n = 0; n < 16; n++)
{
    P1 = n;

    delay();
}
}

void mode3(void)
{
    LED1 = 1;
    LED2 = 0;
    LED3 = 1;
	  LED4 = 0;
    
    delay();

    LED1 = 0;
    LED2 = 1;
    LED3 = 0;
	  LED4 = 1;
    
    delay();
}

void mode4(void)
{
    LED1 = 1;
    LED2 = 1;
    LED3 = 1;
	  LED4 = 1;
    delay();

    LED1 = 0;
    LED2 = 0;
    LED3 = 0;
	  LED4 = 0;
	delay();
}

void main(void)
{
    P27=0;
    LED1 = 0;
    LED2 = 0;
    LED3 = 0;
	  LED4 = 0;

    while(1)
    {
        if(BUTTON == 0)
        {
            delay();

            if(BUTTON == 0)
            {
                count++;

                if(count > 4)
                    count = 1;

                while(BUTTON == 0);

                delay();
            }
        }

        switch(count)
        {
            case 1:
                mode1();
                break;

            case 2:
                mode2();
                break;

            case 3:
                mode3();
                break;

            case 4:
                mode4();
                break;
        }
    }
}