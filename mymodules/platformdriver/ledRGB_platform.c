#include <linux/module.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/types.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/delay.h>

struct led_dev{
    struct miscdevice led_misc_device;
    uint32_t led_mask;
    const char *led_name;
    char led_value[8];
};

#define BCM2710_PERI_BASE   0x20000000
#define GPIO_BASE           (BCM2710_PERI_BASE + 0x200000)

#define GPIO_11 11
#define GPIO_6 6
#define GPIO_27 27
#define GPIO_22 22
#define GPIO_26 26


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

#define GPFSEL0     GPIO_BASE + 0x00
#define GPFSEL1     GPIO_BASE + 0x04
#define GPFSEL2     GPIO_BASE + 0x08
#define GPSET0      GPIO_BASE + 0x1C
#define GPCLR0      GPIO_BASE + 0x28

static void __iomem *GPFSEL0_V;
static void __iomem *GPFSEL1_V;
static void __iomem *GPFSEL2_V;
static void __iomem *GPSET0_V;
static void __iomem *GPCLR0_V;


static ssize_t led_write(struct file *file, const char __user *buff, size_t count, loff_t *ppos){
    const char *led_on = "on";
    const char *led_off = "off";
    struct led_dev *led_device;

    pr_info("led_write() is called.\n");

    led_device = container_of(file->private_data, struct led_dev, led_misc_device);

    if(copy_from_user(led_device->led_value, buff, count)){
        pr_info("Failed to copy_from_user()\n");
        return -EFAULT;
    }

    led_device->led_value[count - 1] = '\0';
    pr_info("Message received from User Space: %s", led_device->led_value);

    if(!strcmp(led_device->led_value, led_on)){
        iowrite32(led_device->led_mask, GPSET0_V);
    }else if(!strcmp(led_device->led_value, led_off)){
        iowrite32(led_device->led_mask, GPCLR0_V);
    }else{
        pr_info("Bad value\n");
        return -EINVAL;
    }

    pr_info("led_write() is exit.\n");
    return count;
}

static ssize_t led_read(struct file *file, char __user *buff, size_t count, loff_t *ppos){
    struct led_dev *led_device;

    pr_info("led_read() is called.\n");

    led_device = container_of(file->private_data, struct led_dev, led_misc_device);

    if(*ppos == 0){
        if(copy_to_user(buff, &led_device->led_value, sizeof(led_device->led_value))){
            pr_info("Failed to return led_value to user space\n");
            return -EFAULT;
        }
        *ppos += 1;
        return sizeof(led_device->led_value);
    }

    pr_info("led_read() is exit.\n");

    return 0;
}

static const struct file_operations led_fops = {
    .owner = THIS_MODULE,
    .read = led_read,
    .write = led_write,
};

static int led_probe(struct platform_device *pdev){
    struct led_dev *led_device;
    int ret;
    char led_val[8] = "off\n";

    pr_info("led_probe enter\n");

    led_device = devm_kzalloc(&pdev->dev, sizeof(struct led_dev), GFP_KERNEL);

    of_property_read_string(pdev->dev.of_node, "label", &led_device->led_name);
    led_device->led_misc_device.minor = MISC_DYNAMIC_MINOR;
    led_device->led_misc_device.name = led_device->led_name;
    led_device->led_misc_device.fops = &led_fops;

    if(strcmp(led_device->led_name, "ledred") == 0){
        led_device->led_mask = GPIO_11_INDEX;
    }else if(strcmp(led_device->led_name, "ledgreen") == 0){
        led_device->led_mask = GPIO_6_INDEX;
    }else if(strcmp(led_device->led_name, "ledyellow") == 0){
        led_device->led_mask = GPIO_26_INDEX;
    }else{
        pr_info("Bad device tree value\n");
        return -EINVAL;
    }

    memcpy(led_device->led_value, led_val, sizeof(led_val));

    ret = misc_register(&led_device->led_misc_device);
    if(ret){
        return ret;
    }

    platform_set_drvdata(pdev, led_device);

    pr_info("leds_probe exit");

    return 0;
}

static int led_remove(struct platform_device *pdev){
    struct led_dev *led_device = platform_get_drvdata(pdev);

    pr_info("led_remove enter\n");

    misc_deregister(&led_device->led_misc_device);

    pr_info("led_remove exit\n");

    return 0;    
}

static const struct of_device_id my_of_ids[] = {
    { .compatible = "arrow,RGBleds" },
    {},
};
MODULE_DEVICE_TABLE(of, my_of_ids);

static struct platform_driver led_platform_driver = {
    .probe = led_probe,
    .remove = led_remove,
    .driver = {
        .name = "RGBleds",
        .of_match_table = my_of_ids,
        .owner = THIS_MODULE,
    }
};

static int led_init(void){
    int ret;
    uint32_t GPFSEL_read, GPFSEL_write;
    pr_info("led_init enter\n");

    ret = platform_driver_register(&led_platform_driver);
    if(ret != 0){
        pr_err("platform value returned: %d\n", ret);
        return ret;
    }

    GPFSEL0_V = ioremap(GPFSEL0, sizeof(uint32_t));
    GPFSEL1_V = ioremap(GPFSEL1, sizeof(uint32_t));
    GPFSEL2_V = ioremap(GPFSEL2, sizeof(uint32_t));
    GPSET0_V = ioremap(GPSET0, sizeof(uint32_t));
    GPCLR0_V = ioremap(GPCLR0, sizeof(uint32_t));

    GPFSEL_read = ioread32(GPFSEL0_V);
    GPFSEL_write = (GPFSEL_read & ~FSEL_6_MASK) | (GPIO_6_FUNC & FSEL_6_MASK);
    iowrite32(GPFSEL_write, GPFSEL0_V);
    
    GPFSEL_read = ioread32(GPFSEL1_V);
    GPFSEL_write = (GPFSEL_read & ~FSEL_11_MASK) | (GPIO_11_FUNC & FSEL_11_MASK);
    iowrite32(GPFSEL_write, GPFSEL1_V);

    GPFSEL_read = ioread32(GPFSEL2_V);
    GPFSEL_write = (GPFSEL_read & ~FSEL_26_MASK) | (GPIO_26_FUNC & FSEL_26_MASK);
    iowrite32(GPFSEL_write, GPFSEL2_V);

    // GPFSEL_read = ioread32(GPFSEL2_V);

    // GPFSEL_write = (GPFSEL_read & ~GPIO_MASK_ALL_LEDS) | (GPIO_SET_FUNCTION_LEDS & GPIO_MASK_ALL_LEDS);

    // iowrite32(GPFSEL_write, GPFSEL2_V);
    iowrite32(GPIO_SET_ALL_LEDS, GPCLR0_V);

    pr_info("led_init exit\n");
    return 0;
}

static void led_exit(void){
    pr_info("led driver enter\n");

    iowrite32(GPIO_SET_ALL_LEDS, GPCLR0_V);

    iounmap(GPFSEL0_V);
    iounmap(GPFSEL1_V);
    iounmap(GPFSEL2_V);
    iounmap(GPSET0_V);
    iounmap(GPCLR0_V);

    platform_driver_unregister(&led_platform_driver);

    pr_info("led driver exit\n");
}

module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Masaki TSUKADA");
MODULE_DESCRIPTION("This is a platform driver that turns on/off tree led devices");
