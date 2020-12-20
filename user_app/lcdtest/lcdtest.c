#include <stdlib.h>
#include <stdio.h>

#define CONTRAST 56

int main(int argc, char **argv){
    if(i2c_lcd_init(CONTRAST) < 0){
        fprintf(stderr, "Cannot init LCD display\n");
        return -1;
    }

    i2c_lcd_setcursor(0, 0);
    i2c_lcd_puts("Hello");
    i2c_lcd_setcursor(0, 1);
    i2c_lcd_puts("Hello");
    i2c_lcd_deinit();

    return 0;
}