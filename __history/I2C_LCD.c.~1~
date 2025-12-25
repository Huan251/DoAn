// CCS C driver code for I2C LCDs (HD44780 compliant controllers)
// https://simple-circuit.com/



#define LCD_BACKLIGHT          0x08
#define LCD_NOBACKLIGHT        0x00
#define LCD_FIRST_ROW          0x80
#define LCD_SECOND_ROW         0xC0
#define LCD_THIRD_ROW          0x94
#define LCD_FOURTH_ROW         0xD4
#define LCD_CLEAR              0x01
#define LCD_RETURN_HOME        0x02
#define LCD_ENTRY_MODE_SET     0x04
#define LCD_CURSOR_OFF         0x0C
#define LCD_UNDERLINE_ON       0x0E
#define LCD_BLINK_CURSOR_ON    0x0F
#define LCD_MOVE_CURSOR_LEFT   0x10
#define LCD_MOVE_CURSOR_RIGHT  0x14
#define LCD_TURN_ON            0x0C
#define LCD_TURN_OFF           0x08
#define LCD_SHIFT_LEFT         0x18
#define LCD_SHIFT_RIGHT        0x1E

#ifndef LCD_TYPE
   #define LCD_TYPE 2           // 0=5x7, 1=5x10, 2=2 lines
#endif


int1 RS;
unsigned int8 i2c_addr, backlight_val = LCD_BACKLIGHT;

void LCD_Write_Nibble(unsigned int8 n);
void LCD_Cmd(unsigned int8 Command);
void LCD_Goto(unsigned int8 col, unsigned int8 row);
void LCD_Out(unsigned int8 LCD_Char);
void LCD_Begin(unsigned int8 _i2c_addr);
void Backlight();
void noBacklight();
void Expander_Write(unsigned int8 value);

void LCD_Write_Nibble(unsigned int8 n) {
  n |= RS;
  Expander_Write(n);
  Expander_Write(n | 0x04);
  delay_us(1);
  Expander_Write(n & 0xFB);
  delay_us(50);
}

void LCD_Cmd(unsigned int8 Command) {
  RS = 0;
  LCD_Write_Nibble(Command & 0xF0);
  LCD_Write_Nibble((Command << 4) & 0xF0);
}

void LCD_Goto(unsigned int8 col, unsigned int8 row) {
  switch(row) {
    case 2:
      LCD_Cmd(0xC0 + col-1);
      break;
    case 3:
      LCD_Cmd(0x94 + col-1);
      break;
    case 4:
      LCD_Cmd(0xD4 + col-1);
    break;
    default:      // case 1:
      LCD_Cmd(0x80 + col-1);
  }
}

void LCD_Out(unsigned int8 LCD_Char){
  RS = 1;
  LCD_Write_Nibble(LCD_Char & 0xF0);
  LCD_Write_Nibble((LCD_Char << 4) & 0xF0);
}

void LCD_Begin(unsigned int8 _i2c_addr) {
  i2c_addr = _i2c_addr;
  Expander_Write(0);
  delay_ms(40);
  LCD_Cmd(3);
  delay_ms(5);
  LCD_Cmd(3);
  delay_ms(5);
  LCD_Cmd(3);
  delay_ms(5);
  LCD_Cmd(LCD_RETURN_HOME);
  delay_ms(5);
  LCD_Cmd(0x20 | (LCD_TYPE << 2));
  delay_ms(50);
  LCD_Cmd(LCD_TURN_ON);
  delay_ms(50);
  LCD_Cmd(LCD_CLEAR);
  delay_ms(50);
  LCD_Cmd(LCD_ENTRY_MODE_SET | LCD_RETURN_HOME);
  delay_ms(50);
}

void Backlight() {
  backlight_val = LCD_BACKLIGHT;
  Expander_Write(0);
}

void noBacklight() {
  backlight_val = LCD_NOBACKLIGHT;
  Expander_Write(0);
}

void Expander_Write(unsigned int8 value) {
  I2C_Start(I2C_LCD);
  I2C_Write(I2C_LCD, i2c_addr);
  I2C_Write(I2C_LCD, value | backlight_val);
  I2C_Stop(I2C_LCD);
}
