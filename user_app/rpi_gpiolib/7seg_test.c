#include <stdio.h>
#include <unistd.h>
#include "rpi_gpio.h"
#include "rpi_gpiolib.h"

#define LED_BASE 7

// $../../tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian/bin/arm-linux-gnueabihf-gcc 7seg_test.c librpi_gpio.a -o 7seg_test

void led_init(void){
    rpi_gpio_function_set(LED_BASE + 0, RPI_GPF_OUTPUT);
    rpi_gpio_function_set(LED_BASE + 1, RPI_GPF_OUTPUT);
    rpi_gpio_function_set(LED_BASE + 2, RPI_GPF_OUTPUT);
    rpi_gpio_function_set(LED_BASE + 3, RPI_GPF_OUTPUT);
}

void led_put(unsigned int v){
    rpi_gpio_clear32(RPI_GPIO_P1MASK, 0x0F << LED_BASE);
    rpi_gpio_set32(RPI_GPIO_P1MASK, (v & 0x0F) << LED_BASE);
}

int main(void){
    unsigned int value = 0;

    if(!rpi_gpio_init()){
        return -1;
    }
    led_init();

    for(value = 0; value < 10; value++){
        printf("LED = %d\n", value);
        led_put(value);
        sleep(1);
    }

    rpi_gpio_deinit();
    return 1;
}