#ifndef __LIBI2CLCD_H__
#define __LIBI2CLCD_H__

int i2c_lcd_cleardiaplsy(void);
int i2c_lcd_setcursor(int, int);
int i2c_lcd_puts(char *);
int i2c_lcd_init(int);
int i2c_lcd_deinit(void);

#endif