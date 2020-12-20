#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdint-gcc.h>
#include "rpi_gpio.h"
#include "rpi_gpiolib.h"

static volatile uint32_t *gpio_base = NULL;

int rpi_gpio_init(void){
    int fd = open("/dev/mem", O_RDWR | O_SYNC);
    void *gpio_mmap;

    if(fd < 0){
        fprintf(stderr, "Failed to open /dev/mem\n");
        return 0;
    }

    gpio_mmap = mmap(NULL, RPI_BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, RPI_GPIO_BASE);
    if(fd < 0){
        fprintf(stderr, "Failded to mmap GPIO\n");
        return 0;
    }
    close(fd);
    gpio_base = (volatile uint32_t*)gpio_mmap;
    return 1;
}

int rpi_gpio_deinit(void){
    munmap((void*)gpio_base, RPI_BLOCK_SIZE);
    gpio_base = NULL;
    return 1;
}

int rpi_gpio_function_set(int pin, uint32_t func){
    int index = RPI_GPFSEL0_INDEX + pin / 10;
    uint32_t mask = ~(0x07 << ((pin % 10) * 3));
    gpio_base[index] = (gpio_base[index] & mask) | ((func & 0x07) << ((pin % 10) * 3));
    return 1;
}

int rpi_gpio_pull_control(int pin, uint32_t pullmode){
    gpio_base[RPI_GPPUD_INDEX] = pullmode & 0x03;
    gpio_base[RPI_GPPUDCLK0_INDEX] = 0x01 << pin;

    usleep(100);
    gpio_base[RPI_GPPUDCLK0_INDEX] = 0;
    gpio_base[RPI_GPPUD_INDEX] = 0;
    return 1;
}

void rpi_gpio_set32(uint32_t mask, uint32_t val){
    gpio_base[RPI_GPSET0_INDEX] = val & mask;
}

uint32_t rpi_gpio_get32(uint32_t mask){
    return gpio_base[RPI_GPLEV0_INDEX] & mask;
}

void rpi_gpio_clear32(uint32_t mask, uint32_t val){
    gpio_base[RPI_GPCLR0_INDEX] = val & mask;
}

void rpi_gpio_setpin(int pin){
    gpio_base[RPI_GPSET0_INDEX] = 1 << pin;
}

uint32_t rpi_gpio_getpin(int pin){
    return (gpio_base[RPI_GPLEV0_INDEX] & (1 << pin));
}

void rpi_gpio_clearpin(int pin){
    gpio_base[RPI_GPCLR0_INDEX] = 1 << pin;
}