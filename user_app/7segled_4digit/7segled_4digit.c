#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/i2c.h>
#include <linux/fs.h>
#include <linux/of.h>
#include <linux/uaccess.h>

struct ioexp_dev {
    struct i2c_client *client;
    struct miscdevice ioexp_miscdevice;
    char name[8];
};

static ssize_t ioexp_read_file(struct file *file, char __user *userbuf, size_t count, loff_t *ppos){
    int expval, size;
    char buf[3];
    struct ioexp_dev *ioexp;

    ioexp = container_of(file->private_data, struct ioexp_dev, ioexp_miscdevice);

    expval = i2c_smbus_read_byte(ioexp->client);
    if(expval < 0){
        return -EFAULT;
    }

    size = sprintf(buf, "%02x", expval);

    buf[size] = '\n';

    if(*ppos == 0){
        if(copy_to_user(userbuf, buf, size+1)){
            pr_info("Falied to return led_value to user space\n");
            return -EFAULT;
        }
        *ppos += 1;
        return size + 1;
    }

    return 0;
}

static ssize_t ioexp_write_file(struct file *file, const char __user *userbuf, size_t count, loff_t *ppos){
    int ret;
    unsigned long val;
    char buf[4];
    struct ioexp_dev *ioexp;

    ioexp = container_of(file->private_data, struct ioexp_dev, ioexp_miscde