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
 
/* ---------------- LCD low-level (same as 7A/7B) ---------------- */
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
 
    lcd_cmd(0x28);
    lcd_cmd(0x0C);
    lcd_cmd(0x06);
    lcd_cmd(0x01);
    delay_ms(2);
}
 
void lcd_goto(unsigned char row, unsigned char col)
{
    unsigned char addr = (row == 0) ? (0x80 + col) : (0xC0 + col);
    lcd_cmd(addr);
}
 
/* ===================== Task 7C: keypad scan ========================= */
 
/* keymap[row][col]: row0=R1, row1=R2, row2=R3, row3=R4
                     col0=C1, col1=C2, col2=C3, col3=C4  */
code char keymap[4][4] = {
    {'1', '2', '3', 'A'},   /* R1: P2.7 */
    {'4', '5', '6', 'B'},   /* R2: P2.6 */
    {'7', '8', '9', 'C'},   /* R3: P2.5 */
    {'*', '0', '#', 'D'}    /* R4: P2.4 */
};
 
code unsigned char row_mask[4] = {0x80, 0x40, 0x20, 0x10};  /* R1..R4 */
 
/* Returns the pressed key character, or 0 if none is pressed.
   Rows (P2.4-P2.7) are driven low one at a time; columns (P2.0-P2.3)
   are read back (8051 quasi-bidirectional pins act as inputs when
   written 1, using their internal weak pull-ups). */
char keypad_scan(void)
{
    unsigned char row;
    unsigned char colval;
 
    for (row = 0; row < 4; row++)
    {
        P2 = 0xFF;                 /* all rows/cols high first */
        P2 &= ~row_mask[row];      /* drive only this row low */
 
        colval = P2 & 0x0F;        /* read column nibble (C1-C4) */
        if (colval != 0x0F)        /* some column pulled low -> key pressed */
        {
            delay_ms(15);           /* debounce */
            colval = P2 & 0x0F;
            if (colval != 0x0F)
            {
                if (!(colval & 0x08)) return keymap[row][0];  /* C1 */
                if (!(colval & 0x04)) return keymap[row][1];  /* C2 */
                if (!(colval & 0x02)) return keymap[row][2];  /* C3 */
                if (!(colval & 0x01)) return keymap[row][3];  /* C4 */
            }
        }
    }
    return 0;   /* no key pressed */
}
 
/* Blocks until the currently-held key is released (simple debounce) */
void wait_key_release(void)
{
    while (keypad_scan() != 0);
    delay_ms(15);
}
 
/* ---------------- Main: move a character with 2/8/4/6 -------------- */
void main(void)
{
    unsigned char row = 0, col = 0;
    char key;
 
    lcd_init();
 
    lcd_goto(row, col);
    lcd_data('*');              /* initial marker */
 
    while (1)
    {
        key = keypad_scan();
 
        if (key != 0)
        {
            /* erase marker from current position */
            lcd_goto(row, col);
            lcd_data(' ');
 
            switch (key)
            {
                case '2':                    /* Up */
                    if (row > 0) row--;
                    break;
                case '8':                    /* Down */
                    if (row < 1) row++;
                    break;
                case '4':                    /* Left */
                    if (col > 0) col--;
                    break;
                case '6':                    /* Right */
                    if (col < 15) col++;
                    break;
                default:
                    break;                   /* ignore other keys */
            }
 
            /* redraw marker at new position */
            lcd_goto(row, col);
            lcd_data('*');
 
            wait_key_release();
        }
    }
}