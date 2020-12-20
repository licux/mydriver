#include <linux/module.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/types.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/leds.h>

#define GPIO_11 11
#define GPIO_6 6
// #define GPIO_27 27
// #define GPIO_22 22
#define GPIO_26 26

#define GPFSEL0_offset  0x00
#define GPFSEL1_offset  0x04
#define GPFSEL2_offset  0x08
#define GPSET0_offset   0x1C
#define GPCLR0_offset   0x28

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


struct led_dev{
    uint32_t led_mask;
    void __iomem *base;
    struct led_classdev cdev;
};

static void led_control(struct led_classdev *led_cdev, enum led_brightness b){
    struct led_dev *led = container_of(led_cdev, struct led_dev, cdev);

    iowrite32(GPIO_SET_ALL_LEDS, led->base + GPCLR0_offset);

    if(b != LED_OFF){
        iowrite32(led->led_mask, led->base + GPSET0_offset);
    }else{
        iowrite32(led->led_mask, led->base + GPCLR0_offset);
    }
}

static int ledclass_probe(struct platform_device *pdev){
    void __iomem *g_ioremap_addr;
    struct device_node *child;
    struct resource *res;
    uint32_t GPFSEL_read, GPFSEL_write;
    struct device *dev = &pdev->dev;
    int ret = 0;
    int count;

    pr_info("ledclass_probe enter\n");

    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if(!res){
        pr_err("IORESOUCE_MEM, 0 does not exist\n");
        return -EINVAL;
    }
    pr_info("res->start = 0x%08lx\n", (long unsigned int)res->start);
    pr_info("res->end   = 0x%08lx\n", (long unsigned int)res->end);

    g_ioremap_addr = devm_ioremap(dev, res->start, resource_size(res));
    if(!g_ioremap_addr){
        pr_err("ioremap failed\n");
        return -ENOMEM;
    }

    count = of_get_child_count(dev->of_node);
    if(!count){
        return -EINVAL;
    }

    pr_info("There are %d nodes\n", count);


    GPFSEL_read = ioread32(g_ioremap_addr + GPFSEL0_offset);
    GPFSEL_write = (GPFSEL_read & ~FSEL_6_MASK) | (GPIO_6_FUNC & FSEL_6_MASK);
    iowrite32(GPFSEL_write, g_ioremap_addr + GPFSEL0_offset);

    GPFSEL_read = ioread32(g_ioremap_addr + GPFSEL1_offset);
    GPFSEL_write = (GPFSEL_read & ~FSEL_11_MASK) | (GPIO_11_FUNC & FSEL_11_MASK);
    iowrite32(GPFSEL_write, g_ioremap_addr + GPFSEL1_offset);

    GPFSEL_read = ioread32(g_ioremap_addr + GPFSEL2_offset);
    GPFSEL_write = (GPFSEL_read & ~FSEL_26_MASK) | (GPIO_26_FUNC & FSEL_26_MASK);
    iowrite32(GPFSEL_write, g_ioremap_addr + GPFSEL2_offset);


    // GPFSEL_read = ioread32(g_ioremap_addr + GPFSEL2_offset);
    // GPFSEL_write = (GPFSEL_read & ~GPIO_MASK_ALL_LEDS) | (GPIO_SET_FUNCTION_LEDS & GPIO_MASK_ALL_LEDS);

    // iowrite32(GPFSEL_write, g_ioremap_addr + GPFSEL2_offset);
    iowrite32(GPIO_SET_ALL_LEDS, g_ioremap_addr + GPCLR0_offset);

    for_each_child_of_node(dev->of_node, child){
        struct led_dev *led_device;
        struct led_classdev *cdev;
        led_device = devm_kzalloc(dev, sizeof(*led_device), GFP_KERNEL);
        if(!led_device){
            return -ENOMEM;
        }

        cdev = &led_device->cdev;
        led_device->base = g_ioremap_addr;

        of_property_read_string(child, "label", &cdev->name);

        if(strcmp(cdev->name, "red") == 0){
            led_device->led_mask = GPIO_11_INDEX;
        }else if(strcmp(cdev->name, "green") == 0){
            led_device->led_mask = GPIO_6_INDEX;
        }else if(strcmp(cdev->name, "yellow") == 0){
            led_device->led_mask = GPIO_26_INDEX;
        }else{
            pr_info("Bad device tree value: %s\n", cdev->name);
            return -EINVAL;
        }

        led_device->cdev.brightness = LED_OFF;
        led_device->cdev.brightness_set = led_control;

        ret = devm_led_classdev_register(dev, &led_device->cdev);
        if(ret){
            dev_err(dev, "Failded to register the led %s\n", cdev->name);
            of_node_put(child);
            return ret;
        }
    }

    pr_info("ledclass_probe exit\n");
    return 0;
}

static int ledclass_remove(struct platform_device *pdev){
    pr_info("ledclass_remove enter");
    pr_info("ledclass_remove exit");

    return 0;
};

static const struct of_device_id my_of_ids[] = {
    { .compatible = "arrow,RGBclassleds" },
    {},
};
MODULE_DEVICE_TABLE(of, my_of_ids);

static struct platform_driver led_platform_driver = {
    .probe = ledclass_probe,
    .remove = ledclass_remove,
    .driver = {
        .name = "RGBclassleds",
        .of_match_table = my_of_ids,
        .owner = THIS_MODULE,
    }
};

static int ledRGBclass_init(void){
    int ret;
    pr_info("ledRGBclass_init enter\n");

    ret = platform_driver_register(&led_platform_driver);
    if(ret != 0){
        pr_err("platform value returned %d\n", ret);
        return ret;
    }

    pr_info("ledRGBclass_init exit\n");
    return 0;
}

static void ledRGBclass_exit(void){
    pr_info("ledRGBclass_exit enter\n");

    platform_driver_unregister(&led_platform_driver);

    pr_info("ledRGBclass_exit driver exit\n");
}

module_init(ledRGBclass_init);
module_exit(ledRGBclass_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Masaki TSUKADA");
MODULE_DESCRIPTION("This is a driver that turns on/off RGB leds using LED subsystem");