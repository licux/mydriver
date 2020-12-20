#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/errno.h>
#include <linux/cdev.h>
#include <linux/sched.h>
#include <linux/stat.h>
#include <linux/io.h>
#include <linux/init.h>
#include <linux/version.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>

#include "rpi_gpio.h"

MODULE_AUTHOR("Masaki Tsukada");
MODULE_LICENSE("Dual BSD/GPL");

#define SSEGLED_NUM_DEVS 1
#define SSEGLED_DEVNAME "ssegled"
#define SSEGLED_MAJOR 0
#define SSEGLED_MINOR 0

/* Base Major number */
static int _ssegled_major = SSEGLED_MAJOR;
/* Base Minor number */
static int _ssegled_minor = SSEGLED_MINOR;

/* cdev structure array */
static struct cdev *ssegled_cdev_array = NULL;

/* Device class pointer */
static struct class *ssegled_class = NULL;

// #define SSEG_GPIO_MAPNAME "sseg_gpio_map"

/* GPIO register pointer */
// static void __iomem *gpio_map;
// static volatile uint32_t *gpio_base;

#define LED_BASE 7

/* Number displaying to LED */
static int ssegled_display_value = 0;


// static int ssegled_gpio_map(void);
// static int ssegled_gpio_unmap(void);
static int rpi_gpio_function_set(int, uint32_t);
void rpi_gpio_update(uint32_t, uint32_t);
static void rpi_gpio_set32(uint32_t, uint32_t);
static void rpi_gpio_clear32(uint32_t, uint32_t);
static void ssegled_gpio_setup(void);
static int ssegled_put(int);
static int ssegled_open(struct inode *, struct file *);
static int ssegled_release(struct inode *, struct file *);
static ssize_t ssegled_write(struct file *, const char __user *, size_t, loff_t *);
static int ssegled_register_dev(void);
static int ssegled_init(void);
static void ssegled_exit(void);


struct file_operations ssegled_fops = {
    .open = ssegled_open,
    .release = ssegled_release,
    .write = ssegled_write,
};


// static int ssegled_gpio_map(void){
//     // if(!request_mem_region(RPI_GPIO_BASE, RPI_BLOCK_SIZE, SSEG_GPIO_MAPNAME)){
//     //     printk(KERN_ALERT "resuet_mem_region failed\n");
//     //     return -EBUSY;
//     // }
//     gpio_map = ioremap_nocache(RPI_GPIO_BASE, BLOCK_SIZE);
//     gpio_base = (volatile uint32_t*)gpio_map;

//     return 0;
// }

// static int ssegled_gpio_unmap(void){
//     iounmap(gpio_map);
//     release_mem_region(RPI_GPIO_BASE, RPI_BLOCK_SIZE);

//     gpio_map = NULL;
//     gpio_base = NULL;

//     return 0;
// }

static int rpi_gpio_function_set(int pin, uint32_t func){
    if(func == RPI_GPF_INPUT){
        gpio_direction_input(pin);
    }else if(func == RPI_GPF_OUTPUT){
        gpio_direction_output(pin, 0);
    }

    // int index = RPI_GPFSEL0_INDEX + pin / 10;
    // uint32_t mask = ~(0x7 << ((pin % 10) * 3));
    // gpio_base[index] = (gpio_base[index] & mask) | ((func & 0x07) << ((pin % 10) * 3));
    return 1;
}

void rpi_gpio_update(uint32_t mask, uint32_t val){
    int i;
    int flag = mask & val;
    for(i = 0; i < 32; i++){
        if((flag & 0x01) == 1){
            // Update target
            if((val >> i) & 0x01){
                gpio_set_value(i, 1);
            }else{
                gpio_set_value(i, 0);
            }
        }
    }
}

void rpi_gpio_set32(uint32_t mask, uint32_t val){
    int i;
    int flag = mask & val;
    for(i = 0; i < 32; i++){
        if(((flag >> i) & 0x01) == 1){
            gpio_set_value(i, 1);
        }
    }
    // gpio_base[RPI_GPSET0_INDEX] = val & mask;
}

void rpi_gpio_clear32(uint32_t mask, uint32_t val){
    int i;
    int flag = mask & val;
    for(i = 0; i < 32; i++){
        if(((flag >> i) & 0x01) == 1){
            gpio_set_value(i, 0);
        }
    }
    // gpio_base[RPI_GPCLR0_INDEX] = val & mask;
}

static void ssegled_gpio_setup(void){
    rpi_gpio_function_set(LED_BASE + 0, RPI_GPF_OUTPUT);
    rpi_gpio_function_set(LED_BASE + 1, RPI_GPF_OUTPUT);
    rpi_gpio_function_set(LED_BASE + 2, RPI_GPF_OUTPUT);
    rpi_gpio_function_set(LED_BASE + 3, RPI_GPF_OUTPUT);
}


static int ssegled_put(int arg){
    int v = 0;

    if(arg < 0){
        return -1;
    }
    if(arg > 9){
        if(('0' <= arg) && (arg <= '9')){
            v = arg - 0x30;
        }else{
            return -1;
        }
    }else{
        v = arg;
    }

    ssegled_display_value = arg;
    rpi_gpio_clear32(RPI_GPIO_P1MASK, 0x0F << LED_BASE);
    rpi_gpio_set32(RPI_GPIO_P1MASK, (v & 0x0F) << LED_BASE);

    return 0;
}


static int ssegled_open(struct inode *inode, struct file *filep){
    // int retval;

    // if(gpio_base != NULL){
    //     printk(KERN_ERR "ssegled is already open.\n");
    //     return -EBUSY;
    // }
    // retval = ssegled_gpio_map();
    // if(retval != 0){
    //     printk(KERN_ERR "Cannot open ssegled.\n");
    //     return retval;
    // }
    printk(KERN_INFO "ssegled driver opened!\n");

    return 0;
}

static int ssegled_release(struct inode *inode, struct file *filep){
    printk(KERN_INFO "ssegled driver closed!\n");
    // ssegled_gpio_unmap();
    return 0;
}

static ssize_t ssegled_write(struct file *filep, const char __user *buf, size_t count, loff_t *f_pos){
    char cvalue;
    int retval;

    if(count > 0){
        if(copy_from_user(&cvalue, buf, sizeof(char))){
            return -EFAULT;
        }
        retval = ssegled_put(cvalue);
        if(retval != 0){
            printk(KERN_WARNING "Cannot display %d\n", cvalue);
        }else{
            ssegled_display_value = cvalue;
        }
        return sizeof(char);
    }
    return 0;
}


static int ssegled_register_dev(void){
    int retval;
    dev_t dev;
    size_t size;
    int i;

    retval = alloc_chrdev_region(
        &dev,
        SSEGLED_MINOR,
        SSEGLED_NUM_DEVS,
        SSEGLED_DEVNAME
    );

    if(retval < 0){
        printk(KERN_ERR "alloc_chrdev_region failed\n");
        return retval;
    }

    _ssegled_major = MAJOR(dev);

    ssegled_class = class_create(THIS_MODULE, SSEGLED_DEVNAME);
    if(IS_ERR(ssegled_class)){
        return PTR_ERR(ssegled_class);
    }

    /* Prepare cdev structure */
    size = sizeof(struct cdev) * SSEGLED_NUM_DEVS;
    ssegled_cdev_array = (struct cdev*)kmalloc(size, GFP_KERNEL);

    /* Register character device for the number of device */
    for(i = 0; i < SSEGLED_NUM_DEVS; i++){
        dev_t devno = MKDEV(_ssegled_major, _ssegled_minor++);
        cdev_init(&(ssegled_cdev_array[i]), &ssegled_fops);
        ssegled_cdev_array[i].owner = THIS_MODULE;
        if(cdev_add(&(ssegled_cdev_array[i]), devno, 1) < 0){
            /* Failed to raegister */
            printk(KERN_ERR "cdev_add failed minor = %d\n", _ssegled_minor++);
        }else{
            device_create(
                ssegled_class,
                NULL,
                devno,
                NULL,
                SSEGLED_DEVNAME"%u", _ssegled_minor+i
            );
        }
    }
    return 0;
}


static int ssegled_init(void){
    int retval;
    int i;

    /* start message */
    printk(KERN_INFO "%s loading...\n", SSEGLED_DEVNAME);

    // retval = ssegled_gpio_map();
    // if(retval != 0){
    //     printk(KERN_ALERT "Can not use GPIO registers.\n");
    //     return -EBUSY;
    // }

    /* Initialize GPIO */
    ssegled_gpio_setup();
    for(i = 0; i < 10; i++){
        ssegled_put(i);
        msleep(200);
    }

    retval = ssegled_register_dev();
    if(retval != 0){
        printk(KERN_ALERT "ssegled driver register failed.\n");
        return retval;
    }

    printk(KERN_INFO "ssegled driver register successed.\n");

    // ssegled_gpio_unmap();

    return 0;
}

static void ssegled_exit(void){
    int i;
    dev_t devno;

    /* Unregister character devices */
    for(i = 0; i < SSEGLED_NUM_DEVS; i++){
        cdev_del(&(ssegled_cdev_array[i]));
        devno = MKDEV(_ssegled_major, _ssegled_minor);
        device_destroy(ssegled_class, devno);
    }
    devno = MKDEV(_ssegled_major, _ssegled_minor);
    unregister_chrdev_region(devno, SSEGLED_NUM_DEVS);
    class_destroy(ssegled_class);
}

module_init(ssegled_init);
module_exit(ssegled_exit);

module_param(ssegled_display_value, int, S_IRUSR | S_IRGRP | S_IROTH);
