#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/device.h>

#define DEVICE_NAME "mydev"
#define CLASS_NAME "hello_class"

static struct cdev my_dev;
static struct class *helloClass;
dev_t dev;

static int my_dev_open(struct inode *inode, struct file *file){
    pr_info("my_dev_open() is called.\n");
    return 0;
}

static int my_dev_close(struct inode *inode, struct file *file){
    pr_info("my_dev_close() is called.\n");
    return 0;
}

static long my_dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg){
    pr_info("my_dev_ioctl() is called. cmd = %d, arg = *%ld\n", cmd, arg);
    return 0;
}

static const struct file_operations my_dev_fops = {
    .owner = THIS_MODULE,
    .open = my_dev_open,
    .release = my_dev_close,
    .unlocked_ioctl = my_dev_ioctl,
};

static int __init hello_init(void){
    int ret;
    dev_t dev_no;
    int major;
    struct device *helloDevice;

    pr_info("Hello world init()\n");

    ret = alloc_chrdev_region(&dev_no, 0, 1, DEVICE_NAME);
    if(ret < 0){
        pr_info("Unable to allocate major number\n");
        return ret;
    }

    major = MAJOR(dev_no);
    dev = MKDEV(major, 0);

    pr_info("Allocated correctly with major number %d\n", major);

    cdev_init(&my_dev, &my_dev_fops);
    ret = cdev_add(&my_dev, dev, 1);
    if(ret < 0){
        unregister_chrdev_region(dev, 1);
        pr_info("Unable to add cdev\n");
        return ret;
    }

    helloClass = class_create(THIS_MODULE, CLASS_NAME);
    if(IS_ERR(helloClass)){
        unregister_chrdev_region(dev, 1);
        cdev_del(&my_dev);
        pr_info("Failed to register device class\n");
        return PTR_ERR(helloClass);
    }
    pr_info("device class registered correctly\n");

    helloDevice = device_create(helloClass, NULL, dev, NULL, DEVICE_NAME);
    if(IS_ERR(helloDevice)){
        class_destroy(helloClass);
        cdev_del(&my_dev);
        unregister_chrdev_region(dev, 1);
        pr_info("Failed to create the device\n");
        return PTR_ERR(helloDevice);
    }
    pr_info("The device is created correctly\n");

    return 0;
}

static void __exit hello_exit(void){
    device_destroy(helloClass, dev);
    class_destroy(helloClass);
    cdev_del(&my_dev);
    unregister_chrdev_region(MKDEV(dev, 0), 1);
    pr_info("Hello world class driver exit.\n");
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Masaki Tsukada");
MODULE_DESCRIPTION("This is a print out Hello World module");
