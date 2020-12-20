#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include "i2c_lcd.h"
#include "libi2clcd.h"

#define I2C_DEV "/dev/i2c-1"
#define LCD_I2C_ADDR 0x3e
#define LCD_OSC_FREQ 0x04
#define LCD_AMP_RATIO 0x02

static int i2c_fd;

int i2c_lcd_init(int contrast){
    unsigned char data[8];

    i2c_fd = open(I2C_DEV, O_RDWR);
    if(i2c_fd < 0){
        printf("Cannot open i2c-dev\n");
        return -1;
    }

    if(ioctl(i2c_fd, I2C_SLAVE, LCD_I2C_ADDR) < 0){
        printf("Failed to set i2c address.\n");
        return -1;
    }

    i2c_smbus_write_byte_data(i2c_fd, LCD_RS_CMD_WRITE, LCD_FUNCTIONSET | LCD_FUNC_8BITMODE | LCD_FUNC_2LINE);
    usleep(27);
    i2c_smbus_write_byte_data(i2c_fd, LCD_RS_CMD_WRITE, LCD_FUNCTIONSET | LCD_FUNC_8BITMODE | LCD_FUNC_2LINE | LCD_FUNC_INSTABLE);
    usleep(27);
    data[0] = LCD_IS_OSC | LCD_OSC_FREQ;
    data[1] = LCD_IS_CONTSET1 | contrast_lower(contrast);
    data[2] = LCD_IS_CONTSET2 | contrast_upper(contrast);
    data[3] = LCD_IS_FOLLOWER | LCD_IS_FOLLOWER_ON | LCD_AMP_RATIO;
    int ret = i2c_smbus_write_i2c_block_data(i2c_fd, LCD_RS_CMD_WRITE, 4, data);
    if(ret < 0){
        return ret;
    }

    usleep(200 * 1000);

    i2c_smbus_write_byte_data(i2c_fd, LCD_RS_CMD_WRITE, LCD_DISPLAYCONTROL | LCD_DISP_ON);
    usleep(27);
    i2c_smbus_write_byte_data(i2c_fd, LCD_RS_CMD_WRITE, LCD_ENTRYMODESET | LCD_ENTRYLEFT);
    usleep(27);

    i2c_lcd_cleardisplay();
    return 1;
}

int i2c_lcd_deinit(void){
    close(i2c_fd);
    return 1;
}

int i2c_lcd_puts(char *str){
    i2c_smbus_write_i2c_block_data(i2c_fd, LCD_RS_CMD_WRITE, strlen(str), (unsigned char*)str);
    usleep(30);

    return 1;
}

int i2c_lcd_cleardisplay(void){
    i2c_smbus_write_byte_data(i2c_fd, LCD_RS_CMD_WRITE, LCD_CLEARDISPLAY);
    usleep(1080);

    return 1;
}

int i2c_lcd_setcursor(int col, int row){
    int row_offs[] = {0x00, 0x40, 0x14, 0x54};

    i2c_smbus_write_byte_data(i2c_fd, LCD_RS_CMD_WRITE, LCD_SETDRAMADDR | (col + row_offs[row]));
    usleep(30);

    return 1;
}
