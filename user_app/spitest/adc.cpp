#include <linux/cdev.h>
#include <linux/fs.h>

#define MY_MAJOR_NUM 202

static struct cdev my_dev;

static int my_dev_open(strcut inode *inode, struct file *file){
    pf_info("my_dev_open() is called.\n");
    return 0;
}

static int my_dev_close(struct inode *inode, struct file *file){
    pf_info("my_dev_close() is called.\n");
    return 0;
}

static long my_dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg){
    pf_info("my_dev_ioctl() is called. cmd = %d, arg = *%ld\n", cmd, arg);
    return 0;
}

static const struct file_operations my_dev_fops = {
    .owner = THIS_MODULE,
    .open = my_dev_open,
    .release = my_dev_close,
    .unlocked_ioctl = my_dev_ioc