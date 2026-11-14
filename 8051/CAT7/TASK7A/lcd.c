#include <reg51.h>

sbit RS = P3^2;              
sbit EN = P3^3;              

#define LCD_PORT  P3         


void delay_us(unsigned int us)
{
    while (us--);
}

void delay_ms(unsigned int ms)
{
    unsigned int i, j;
    for (i = 0; i < ms; i++)
        for (j = 0; j < 120; j++);
}

void lcd_nibble(unsigned char nib)
{
    LCD_PORT = (LCD_PORT & 0x0F) | (nib & 0xF0);
    EN = 1;
    delay_us(2);
    EN = 0;                  
    delay_us(50);
}


void lcd_cmd(unsigned char cmd)
{
    RS = 0;
    lcd_nibble(cmd & 0xF0);
    lcd_nibble(cmd << 4);

    if (cmd == 0x01 || cmd == 0x02)  
        delay_ms(2);
    else
        delay_us(50);                
}


void lcd_data(unsigned char dat)
{
    RS = 1;
    lcd_nibble(dat & 0xF0);
    lcd_nibble(dat << 4);
    delay_us(50);
}


void lcd_init(void)
{
    RS = 0;
    EN = 0;
    delay_ms(50);                    

    lcd_nibble(0x30);  delay_ms(5);      
    lcd_nibble(0x30);  delay_us(200);
    lcd_nibble(0x30);  delay_us(200);

    lcd_nibble(0x20);  delay_us(200);    

    lcd_cmd(0x28);   
    lcd_cmd(0x08);   
    lcd_cmd(0x01);  
    lcd_cmd(0x06);   
    lcd_cmd(0x0F);  
}


void main(void)
{
    lcd_init();

    lcd_cmd(0x04);
		lcd_cmd(0x85);
	
		lcd_data('F');
		lcd_data('I');
		lcd_data('O');
		lcd_data('T');
		lcd_data('-');
		lcd_data('A');
		
    

    while (1);           
}