#include <linux/cdev.h>
#include <linux/fs.h>

#define MY_MAJOR_NUM 202

static struct cdev my_dev;

static int my_dev_open(struct inode *inode, struct file *file){
    pf_info("my_dev_open() is called.\n");
    return 0;
}

static int my_dev_close(struct inode *inode, struct file *file){
    pf_info("my_dev_close() is called.\n");
    return 0;
}

static long my_dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg){
    pf_info("my_dev_ioctl() is called. cmd = %d, arg = *%ld\n", cmd, arg);