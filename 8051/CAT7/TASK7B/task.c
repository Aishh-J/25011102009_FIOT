#include <reg52.h>
 
sbit RS = P3^2;
sbit E  = P3^3;
sbit D4 = P3^4;
sbit D5 = P3^5;
sbit D6 = P3^6;
sbit D7 = P3^7;
 
/* ---------------- Delays ---------------- */
void delay_ms(unsigned int ms)
{
    unsigned int i, j;
    for (i = 0; i < ms; i++)
        for (j = 0; j < 120; j++);
}
 
void delay_us(unsigned int us)
{
    while (us--);
}
 
/* ---------------- Nibble write ---------------- */
void lcd_write_nibble(unsigned char nibble)
{
    D4 = (nibble >> 0) & 0x01;
    D5 = (nibble >> 1) & 0x01;
    D6 = (nibble >> 2) & 0x01;
    D7 = (nibble >> 3) & 0x01;
 
    E = 1;
    delay_us(2);
    E = 0;
    delay_us(50);
}
 
/* ---------------- Command / Data ---------------- */
void lcd_cmd(unsigned char cmd)
{
    RS = 0;
    lcd_write_nibble(cmd >> 4);
    lcd_write_nibble(cmd & 0x0F);
 
    if (cmd == 0x01 || cmd == 0x02)
        delay_ms(2);
    else
        delay_us(50);
}
 
void lcd_data(unsigned char dat)
{
    RS = 1;
    lcd_write_nibble(dat >> 4);
    lcd_write_nibble(dat & 0x0F);
    delay_us(50);
}
 
/* ---------------- Initialization ---------------- */
void lcd_init(void)
{
    delay_ms(20);
 
    RS = 0;
    lcd_write_nibble(0x03);
    delay_ms(5);
    lcd_write_nibble(0x03);
    delay_us(150);
    lcd_write_nibble(0x03);
    delay_us(150);
    lcd_write_nibble(0x02);
    delay_us(150);
 
    lcd_cmd(0x28);   /* 4-bit, 2-line, 5x8 font */
    lcd_cmd(0x0C);   /* display ON, cursor OFF, blink OFF */
    lcd_cmd(0x06);   /* entry mode: increment cursor */
    lcd_cmd(0x01);   /* clear display */
    delay_ms(2);
}
 
/* ---------------- Go to any row/column ---------------- */
void lcd_goto(unsigned char row, unsigned char col)
{
    unsigned char addr = (row == 0) ? (0x80 + col) : (0xC0 + col);
    lcd_cmd(addr);
}
 
/* ================================================================
   MAIN -- change these three values to whatever you're asked live
   ================================================================ */
void main(void)
{
    char CHAR_TO_DISPLAY = 'A';   /* ---> EDIT HERE: character to show   */
    unsigned char ROW     = 0;    /* ---> EDIT HERE: row    (0 or 1)     */
    unsigned char COL     = 3;    /* ---> EDIT HERE: column (0 to 15)    */
 
    lcd_init();
 
    lcd_goto(ROW, COL);
    lcd_data(CHAR_TO_DISPLAY);
 
    while (1);   /* halt */
}