#include <linux/delay.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/mutex.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <linux/uaccess.h> 

#include "i2c_lcd_instruction.h"
#include "i2c-lcd.h"

#define LCD_I2C_ADDR    0x3e
#define LCD_OSC_FREQ    0x04
#define LCD_AMP_RATIO   0x02
#define LCD_COLS        8
#define LCD_LINES       2

#define MIN(a, b) ((a) < (b) ? (a) : (b))

struct i2c_lcd_device_data{
    struct cdev cdev;
    struct class* class;
    struct i2c_client *client;
    unsigned int i2c_lcd_major;
};

struct i2c_lcd_data{
    struct i2c_client *client;
    char **characters;
};

static int contrast = 56;

static const struct i2c_device_id i2c_lcd_id[] = {
    {"i2c_lcd", 0},
    { }
};

MODULE_DEVICE_TABLE(i2c, i2c_lcd_id);

static const unsigned short i2c_lcd_addr[] = {LCD_I2C_ADDR, I2C_CLIENT_END};

static int i2c_lcd_cleardisplay(struct i2c_client *client){
    i2c_smbus_write_byte_data(client, LCD_RS_CMD_WRITE, LCD_CLEARDISPLAY);
    usleep_range(1080, 2000);

    return 0;
}

static int i2c_lcd_puts(struct i2c_client *client, char *str){
    i2c_smbus_write_i2c_block_data(client, LCD_RS_DATA_WRITE, strlen(str), (unsigned char*)str);
    usleep_range(26, 100);

    return 0;
}

static int i2c_lcd_setcursor(struct i2c_client *client, int col, int row){
    int row_offs[] = {0x00, 0x40, 0x14, 0x54};

    i2c_smbus_write_byte_data(client, LCD_RS_CMD_WRITE, LCD_SETDRAMADDR | (col + row_offs[row]) );
    usleep_range(26, 100);

    return 0;
}

static int i2c_lcd_open(struct inode *inode, struct file *filep){
    int i;
    struct i2c_lcd_device_data *lcd_device_data;
    struct i2c_lcd_data *lcd_data;
    
    printk(KERN_INFO "i2c_lcd_open()\n");


    lcd_device_data = container_of(inode->i_cdev, struct i2c_lcd_device_data, cdev);

    lcd_data = kmalloc(sizeof(struct i2c_lcd_data), GFP_KERNEL);
    lcd_data->client = lcd_device_data->client;
    lcd_data->characters = kmalloc(sizeof(char *) * LCD_LINES, GFP_KERNEL);
    for(i = 0; i < LCD_LINES; i++){
        lcd_data->characters[i] = kmalloc(sizeof(char) * LCD_COLS, GFP_KERNEL);
        memset(lcd_data->characters[i], 0x00, sizeof(char) * LCD_COLS);
    }
    filep->private_data = lcd_data;

    return 0;
}

static int i2c_lcd_release(struct inode *inode, struct file *filep){
    int i;
    struct i2c_lcd_data *lcd_data;

    printk(KERN_INFO "i2c_lcd_release()\n");

    lcd_data = filep->private_data;
    for(i = 0; i < LCD_LINES; i++){
        kfree(lcd_data->characters[i]);
    }
    kfree(lcd_data->characters);
    return 0;
}

static ssize_t i2c_lcd_read(struct file *filep, char __user *buf, size_t count, loff_t *f_pos){
    int i, j, min;
    int c = 0;
    char data[18];
    struct i2c_lcd_data *lcd_data = filep->private_data;
    printk(KERN_INFO "i2c_lcd_read()\n");

    for(i = 0; i < LCD_LINES; i++){
        for(j = 0; j < LCD_COLS; j++){
            if(lcd_data->characters[i][j] == '\0'){
                break;
            }
            data[c++] = lcd_data->characters[i][j];
        }
        data[c++] = '\n';
    }

    min = MIN(c, count);
    if(copy_to_user(buf, data, min)){
        printk(KERN_ERR "Failed copy_from_user().\n");
        return -EFAULT;
    }    

    return min;
}

static ssize_t i2c_lcd_write(struct file *filep, const char __user *buf, size_t count, loff_t *f_pos){
    int i, min;
    char data[LCD_COLS];
    struct i2c_lcd_data *lcd_data = filep->private_data;
    printk(KERN_INFO "i2c_lcd_write()\n");
    min = MIN(count, LCD_COLS);

    if(copy_from_user(data, buf, min)){
        printk(KERN_ERR "Failed copy_from_user().\n");
        return -EFAULT;
    }
    for(i = 0; i < LCD_COLS; i++){
        lcd_data->characters[1][i] = lcd_data->characters[0][i];
        if(i < min){
            lcd_data->characters[0][i] = data[i];
        }
    }

    i2c_lcd_cleardisplay(lcd_data->client);
    i2c_lcd_setcursor(lcd_data->client, 0, 0);
    i2c_lcd_puts(lcd_data->client, lcd_data->characters[0]);
    i2c_lcd_setcursor(lcd_data->client, 0, 1);
    i2c_lcd_puts(lcd_data->client, lcd_data->characters[1]);

    return min;
}

static long i2c_lcd_ioctl(struct file *filep, unsigned int cmd, unsigned long arg){
    int ret = 0;
    struct i2c_lcd_data *lcd_data = filep->private_data;
    struct i2c_lcd_io data;
    printk(KERN_INFO "ioctl()\n");

    switch(cmd){
        case IOCTL_CLEARDISPLAY:
            printk(KERN_INFO "ioctl(IOCTL_CLEARDISPLAY)\n");
            i2c_lcd_cleardisplay(lcd_data->client);
            break;
        case IOCTL_WRITE:
            printk(KERN_INFO "ioctl(IOCTL_WRITE)\n");
            memset(&data, 0, sizeof(data));
            if(copy_from_user(&data, (void __user *)arg, sizeof(data))){
                return -EFAULT;
            }
            i2c_lcd_setcursor(lcd_data->client, data.x, data.y);
            i2c_lcd_puts(lcd_data->client, data.str);
            break;
        default:
            printk(KERN_INFO "ioctl() default\n");
            ret = -ENOTTY;
    }

    return ret;
} 

struct file_operations i2c_lcd_fops = {
    .open = i2c_lcd_open,
    .release = i2c_lcd_release,
    .read = i2c_lcd_read,
    .write = i2c_lcd_write,
    .unlocked_ioctl  = i2c_lcd_ioctl,
    .compat_ioctl = i2c_lcd_ioctl,
};

static int lcd_row1_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count){
    char str[LCD_COLS + 1];
    int size = strlen(buf);

    struct i2c_lcd_device_data *data = (struct i2c_lcd_device_data *)dev_get_drvdata(dev);

    i2c_lcd_setcursor(data->client, 0, 0);
    strncpy(str, buf, LCD_COLS);
    str[LCD_COLS] = '0';

    i2c_lcd_puts(data->client, str);
    
    return size;
}
static DEVICE_ATTR(lcd_row1,  S_IWUSR | S_IWGRP, NULL, lcd_row1_store);

static int lcd_row2_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count){
    char str[LCD_COLS + 1];
    int size = strlen(buf);

    struct i2c_lcd_device_data *data = (struct i2c_lcd_device_data *)dev_get_drvdata(dev);

    i2c_lcd_setcursor(data->client, 0, 1);
    strncpy(str, buf, LCD_COLS);
    str[LCD_COLS] = '0';

    i2c_lcd_puts(data->client, str);

    return size;
}
static DEVICE_ATTR(lcd_row2, S_IWUSR | S_IWGRP, NULL, lcd_row2_store);

static int lcd_clear_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count){
    int size = strlen(buf);
    struct i2c_lcd_device_data *data = (struct i2c_lcd_device_data *)dev_get_drvdata(dev);

    i2c_lcd_cleardisplay(data->client);

    return size;
}
static DEVICE_ATTR(lcd_clear, S_IWUSR | S_IWGRP, NULL, lcd_clear_store);

#define I2CLCD_NUM_DEVS 1
#define I2CLCD_DEVNAME "i2c-lcd"
#define I2CLCD_MAJOR 0
#define I2CLCD_MINOR 0

static char *i2c_lcd_devnode(struct device *dev, umode_t *mode){
    if(mode != NULL){
        *mode = ((umode_t)(S_IRUGO|S_IWUGO));
    }
    return NULL;
}

static int i2c_register_dev(struct i2c_lcd_device_data *device_data){
    int ret;
    dev_t dev;

    ret = alloc_chrdev_region(
        &dev,
        I2CLCD_MINOR,
        I2CLCD_NUM_DEVS,
        I2CLCD_DEVNAME 
    );
    if(ret < 0){
        printk(KERN_ERR "Failed alloc_chrdev_region.\n");
        return ret;
    }

    device_data->i2c_lcd_major = MAJOR(dev);
    device_data->class = class_create(THIS_MODULE, I2CLCD_DEVNAME);
    if(IS_ERR(device_data->class)){
        return PTR_ERR(device_data->class);
    }
    device_data->class->devnode = i2c_lcd_devnode;

    /* Prepare cdev structure */
    cdev_init(&device_data->cdev, &i2c_lcd_fops);
    device_data->cdev.owner = THIS_MODULE;
    if(cdev_add(&device_data->cdev, MKDEV(device_data->i2c_lcd_major, 0), 1)){
        printk(KERN_ERR "Failed cdev_add.\n");
    }else{
        device_create(
            device_data->class,
            NULL,
            MKDEV(device_data->i2c_lcd_major, 0),
            NULL,
            I2CLCD_DEVNAME
        );
    }
    
    return 0;
}

static int i2c_lcd_probe(struct i2c_client *client, const struct i2c_device_id *id){
    struct i2c_lcd_device_data *private_data;
    int ret;
    unsigned char data[8];

    printk(KERN_INFO "probing... addr=%d\n", client->addr);

    if(!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_BYTE_DATA | I2C_FUNC_SMBUS_I2C_BLOCK)){
        printk(KERN_ERR "I2C function is not supported\n");
        return -ENODEV;
    }

    ret = i2c_smbus_write_byte_data(client, LCD_RS_CMD_WRITE, LCD_FUNCTIONSET | LCD_FUNC_8BITMODE | LCD_FUNC_2LINE);
    if(ret < 0){
        printk(KERN_ERR "Could not write first function set to i2c device./n");
        return -ENODEV;
    }
    usleep_range(27, 100);
    i2c_smbus_write_byte_data(client, LCD_RS_CMD_WRITE, LCD_FUNCTIONSET | LCD_FUNC_8BITMODE | LCD_FUNC_2LINE | LCD_FUNC_INSTABLE);
    usleep_range(27, 100);

    data[0] = LCD_IS_OSC | LCD_OSC_FREQ;
    data[1] = LCD_IS_CONTSET1 | contrast_lower(contrast);
    data[2] = LCD_IS_CONTSET2 | contrast_upper(contrast);
    data[3] = LCD_IS_FOLLOWER | LCD_IS_FOLLOWER_ON | LCD_AMP_RATIO;

    ret = i2c_smbus_write_i2c_block_data(client, LCD_RS_CMD_WRITE, 4, data);
    if(ret < 0){
        printk(KERN_ERR "Could not initialize i2c lcd device.\n");
        return -ENODEV;
    }
    msleep(200);

    i2c_smbus_write_byte_data(client, LCD_RS_CMD_WRITE, LCD_DISPLAYCONTROL | LCD_DISP_ON);
    usleep_range(27, 100);
    i2c_smbus_write_byte_data(client, LCD_RS_CMD_WRITE, LCD_ENTRYMODESET | LCD_ENTRYLEFT);
    usleep_range(27, 100);

    i2c_lcd_cleardisplay(client);
    i2c_lcd_setcursor(client, 0, 0);
    i2c_lcd_puts(client, "Hello");
    i2c_lcd_setcursor(client, 0, 1);
    i2c_lcd_puts(client, "RasPi");

    private_data = (struct i2c_lcd_device_data *)kmalloc(sizeof(struct i2c_lcd_device_data), GFP_KERNEL);
    if(private_data == NULL){
        printk(KERN_ERR "Not enough kernel memory.\n");
        return -ENOMEM;
    }

    private_data->client = client;
    i2c_set_clientdata(client, private_data);

    i2c_register_dev(private_data);

    ret = device_create_file(&client->dev, &dev_attr_lcd_row1);
    if(ret){
        printk(KERN_ERR "Failed to add lcd_row1\n");
    }
    ret = device_create_file(&client->dev, &dev_attr_lcd_row2);
    if(ret){
        printk(KERN_ERR "Failed to add lcd_row2\n");
    }
    ret = device_create_file(&client->dev, &dev_attr_lcd_clear);
    if(ret){
        printk(KERN_ERR "Failed to add lcd_clear\n");
    }

    return 0;
}

static int i2c_lcd_remove(struct i2c_client *client){
    struct i2c_lcd_device_data *data;

    printk(KERN_INFO "removing...\n");

    device_remove_file(&client->dev, &dev_attr_lcd_row1);
    device_remove_file(&client->dev, &dev_attr_lcd_row2);
    device_remove_file(&client->dev, &dev_attr_lcd_clear);

    data = (struct i2c_lcd_device_data *)i2c_get_clientdata(client);
    kfree(data);
    return 0;
}

static struct i2c_driver i2c_lcd_driver = {
    .probe          = i2c_lcd_probe,
    .remove         = i2c_lcd_remove,
    .id_table       = i2c_lcd_id,
    .address_list   = i2c_lcd_addr,
    .driver = {
        .owner = THIS_MODULE,
        .name = "i2c_lcd",
    },
};

module_i2c_driver(i2c_lcd_driver);

MODULE_DESCRIPTION("I2C LCD driver");
MODULE_LICENSE("GPL");