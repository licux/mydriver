#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>

#define BUFFER_LENGTH 128

#define GPIO_11 11
#define GPIO_6 6
#define GPIO_27 27
#define GPIO_22 22
#define GPIO_26 26

#define GPFSEL0_offset 0x00
#define GPFSEL1_offset 0x04
#define GPFSEL2_offset 0x08
#define GPSET0_offset 0x1C
#define GPCLR0_offset 0x28

#define GPIO_11_INDEX   1 << (GPIO_11 % 32)
#define GPIO_6_INDEX   1 << (GPIO_6 % 32)
// #define GPIO_27_INDEX   1 << (GPIO_27 % 32)
// #define GPIO_22_INDEX   1 << (GPIO_22 % 32)
#define GPIO_26_INDEX   1 << (GPIO_26 % 32)

#define GPIO_11_FUNC   1 << ( (GPIO_11 % 10) * 3)
#define GPIO_6_FUNC   1 << ( (GPIO_6 % 10) * 3)
// #define GPIO_27_FUNC   1 << ( (GPIO_27 % 10) * 3)
// #define GPIO_22_FUNC   1 << ( (GPIO_22 % 10) * 3)
#define GPIO_26_FUNC   1 << ( (GPIO_26 % 10) * 3)

#define FSEL_11_MASK   0b111 << ( (GPIO_11 % 10) * 3)
#define FSEL_6_MASK   0b111 << ( (GPIO_6 % 10) * 3)
// #define FSEL_27_MASK   0b111 << ( (GPIO_27 % 10) * 3)
// #define FSEL_22_MASK   0b111 << ( (GPIO_22 % 10) * 3)
#define FSEL_26_MASK   0b111 << ( (GPIO_26 % 10) * 3)

// #define GPIO_SET_FUNCTION_LEDS  (GPIO_27_FUNC | GPIO_22_FUNC | GPIO_26_FUNC)
// #define GPIO_MASK_ALL_LEDS      (FSEL_27_MASK | FSEL_22_MASK | FSEL_26_MASK)
// #define GPIO_SET_ALL_LEDS       (GPIO_27_INDEX | GPIO_22_INDEX | GPIO_26_INDEX)
#define GPIO_SET_FUNCTION_LEDS  (GPIO_11_FUNC | GPIO_6_FUNC | GPIO_26_FUNC)
#define GPIO_MASK_ALL_LEDS      (FSEL_11_MASK | FSEL_6_MASK | FSEL_26_MASK)
#define GPIO_SET_ALL_LEDS       (GPIO_11_INDEX | GPIO_6_INDEX | GPIO_26_INDEX)

#define UIO_SIZE "/sys/class/uio/uio0/maps/map0/size"

int main(){
    int ret, devuio_fd;
    int mem_fd;
    unsigned int uio_size;
    void *temp;
    int GPFSEL_read, GPFSEL_write;
    void *demo_driver_map;
    char sendstring[BUFFER_LENGTH];
    char *led_on = "on";
    char *led_off = "off";
    char *Exit = "exit";

    printf("Starting led example\n");

    if((mem_fd = open("/dev/mem", O_RDWR | O_SYNC)) < 0){
        printf("could not open /dev/mem\n");
        exit(-1);
    }
    printf("opened /dev/mem\n");

    devuio_fd = open("/dev/uio0", O_RDWR | O_SYNC);
    if(devuio_fd < 0){
        perror("Failed to open the device");
        exit(EXIT_FAILURE);
    }

    FILE *size_fp = fopen(UIO_SIZE, "r");
    fscanf(size_fp, "0x%x", &uio_size);
    fclose(size_fp);
    printf("The value is %d\n", uio_size);

    demo_driver_map = mmap(0, uio_size, PROT_READ | PROT_WRITE, MAP_SHARED, devuio_fd, 0);
    if(demo_driver_map == MAP_FAILED){
        perror("devuio mmap error");
        close(devuio_fd);
        exit(EXIT_FAILURE);
    }

    GPFSEL_read = *(int *)(demo_driver_map + GPFSEL0_offset);
    GPFSEL_write = (GPFSEL_read & ~FSEL_6_MASK) | (GPIO_6_FUNC & FSEL_6_MASK);
    *(int *)(demo_driver_map + GPFSEL0_offset) = GPFSEL_write;

    GPFSEL_read = *(int *)(demo_driver_map + GPFSEL1_offset);
    GPFSEL_write = (GPFSEL_read & ~FSEL_11_MASK) | (GPIO_11_FUNC & FSEL_11_MASK);
    *(int *)(demo_driver_map + GPFSEL1_offset) = GPFSEL_write;

    GPFSEL_read = *(int *)(demo_driver_map + GPFSEL2_offset);
    GPFSEL_write = (GPFSEL_read & ~FSEL_26_MASK) | (GPIO_26_FUNC & FSEL_26_MASK);
    *(int *)(demo_driver_map + GPFSEL2_offset) = GPFSEL_write;

    // *(int *)(demo_driver_map + GPFSEL2_offset) = GPFSEL_write;
    *(int *)(demo_driver_map + GPCLR0_offset)  = GPIO_SET_ALL_LEDS;

    do{
        printf("Enter led value: on, off, or exit :");
        scanf("%[^\n]%*c", sendstring);
        if(strncmp(led_on, sendstring, 3) == 0){
            temp = demo_driver_map + GPSET0_offset;
            *(int *)temp = GPIO_11_INDEX;
        }else if(strncmp(led_off, sendstring, 2) == 0){
            temp = demo_driver_map + GPCLR0_offset;
            *(int *)temp = GPIO_11_INDEX;
        }else if(strncmp(Exit, sendstring, 4) == 0){
            printf("Exit application\n");
    
        }else{
            printf("Bad value\n");
            return -EINVAL;
        }
    }while(strncmp(sendstring, "exit", strlen(sendstring)));

    ret = munmap(demo_driver_map, uio_size);
    if(ret < 0){
        perror("devuio munmap");
        close(devuio_fd);
        exit(EXIT_FAILURE);
    }

    close(devuio_fd);
    printf("Application termiated\n");
    exit(EXIT_SUCCESS);
}