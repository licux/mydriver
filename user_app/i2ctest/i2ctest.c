#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include "i2c-lcd.h"

int main(void){
    int count;
    char buf[32];
    struct i2c_lcd_io lcd_io;
    int fd = open("/dev/i2c-lcd", O_RDWR);
    if(fd < 0){
        fprintf(stderr, "Failed to open device file.\n");
        return 0;
    }
    printf("open()\n");

    sleep(1);
    strcpy(buf, "Test");
    count = write(fd, buf, 8);
    printf("write(\"Test\") [count:%d]\n", count);
    sleep(1);
    strcpy(buf, "RasPi");
    count = write(fd, buf, 8);
    printf("write(\"RasPi\") [count:%d]\n", count);

    sleep(1);
    memset(buf, '\0', 16);
    count = read(fd, buf, 16);
    printf("read() [count:%d]\n", count);
    printf("%s\n", buf);

    sleep(1);
    printf("ioctl(CLEARDISPLAY)\n");
    ioctl(fd, IOCTL_CLEARDISPLAY);

    sleep(3);
    printf("ioctl(WRITE)\n");
    strcpy(lcd_io.str, "ioctl");
    lcd_io.x = 1;
    lcd_io.y = 1;
    ioctl(fd, (IOCTL_WRITE), &lcd_io);


    sleep(1);
    close(fd);
    printf("close()\n");

    sleep(1);
    return 0;
}