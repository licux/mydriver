#ifndef __RPI_GPIOLIB_H__
#define __RPI_GPIOLIB_H__

#include <stdint-gcc.h>

int rpi_gpio_init(void);
int rpi_gpio_deinit(void);
int rpi_gpio_function_set(int, uint32_t);
int rpi_gpio_pull_control(int, uint32_t);
void rpi_gpio_set32(uint32_t, uint32_t);
uint32_t rpi_gpio_get32(uint32_t);
void rpi_gpio_clear32(uint32_t, uint32_t);
void rpi_gpio_setpin(int);
uint32_t rpi_gpio_getpin(int);
void rpi_gpio_clearpin(int);

#endif