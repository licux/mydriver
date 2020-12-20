#include <linux/module.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/of_device.h>

static int my_dev_open(struct inode *inode, struct file *file){
    pr_info("my_dev_open() is called.\n");
    return 0;
}

static int my_dev_close(struct inode *inode, struct file *file){
    pr_info("my_dev_close() is called.\n");
    return 0;
}

static long my_dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg){
    pr_info("my_dev_ioctl() is called. cmd = %d, arg = %ld\n", cmd, arg);
    return 0;
}

static const struct file_operations my_dev_fops = {
    .owner = THIS_MODULE,
    .open =  my_dev_open,
    .release = my_dev_close,
    .unlocked_ioctl = my_dev_ioctl,
};

static struct miscdevice helloworld_miscdevice = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "mydev",
    .fops = &my_dev_fops,
};

static int my_probe(struct platform_device *pdev){
    int ret;
    pr_info("my_probe() function is called.\n");
    ret = misc_register(&helloworld_miscdevice);
    if(ret != 0){
        pr_err("Could not register misc device mydev");
        return ret;
    }

    pr_info("mydev: got minor %i\n", helloworld_miscdevice.minor);
    return 0;
}

static int my_remove(struct platform_device *pdev){
    pr_info("my_remove() function is called.\n");
    misc_deregister(&helloworld_miscdevice);
    return 0;
}

static const struct of_device_id my_of_ids[2] = { 
    { .compatible = "arrow,hellokeys" },
    {},
};
MODULE_DEVICE_TABLE(of, my_of_ids);

static struct platform_driver my_platform_driver = {
    .probe = my_probe,
    .remove = my_remove,
    .driver = {
        .name = "hellokeys",
        .of_match_table = my_of_ids,
        .owner = THIS_MODULE
    }
};

module_platform_driver(my_platform_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Masaki TSUKADA");
MODULE_DESCRIPTION("This is the simplest platform driver");