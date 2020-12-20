#ifndef __I2C_LCD_H__
#define __I2C_LCD_H__

#include <linux/ioctl.h>

#define IOC_MAGIC 'd'

struct i2c_lcd_io {
    char str[8];
    int x;
    int y;
};

#define IOCTL_CLEARDISPLAY  _IO(IOC_MAGIC, 1)
#define IOCTL_WRITE     _IOW(IOC_MAGIC, 2, struct i2c_lcd_io)

#endif