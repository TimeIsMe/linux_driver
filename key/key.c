#include <linux/types.h>
#include <asm/uaccess.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/errno.h>
#include <linux/semaphore.h>


#define GPIOkey_CNT     1
#define GPIOkey_NAME    "mykey"
#define KEY0VALUE       0xF0
#define INVAKEY         0x00

//Function declaration.
static int key_open(struct inode *nd, struct file *fp);
static ssize_t key_read(struct file *fp, \
            char __user *u_buf, \
            size_t u_size, \
            loff_t *u_loft);
static ssize_t key_write(struct file *fp,\
             const char __user *u_buf,\
             size_t u_size, \
             loff_t *u_loff);
static int key_release(struct inode *np,\
                struct file *fp);


static struct gpiokey_dev{
    dev_t   devid;              //Device indentifer number.
    struct cdev cdev;
    struct class *class;
    struct device *device;
    int major;
    int minor;
    struct device_node *nd;     
    int key_gpio;               //Number of gpio used by key.
    atomic_t    keyvalue;
} gpiokey;                      //key device.

static struct file_operations gpiokey_fops = {
    .owner = THIS_MODULE,
    .open = key_open,
    .read = key_read,
    .write = key_write,
    .release = key_release,
};

static int __init key_init(void)
{
    int ret = 0;

    atomic_set(&gpiokey.keyvalue, INVAKEY);

    //1. get device tree node.
    gpiokey.nd = of_find_node_by_path("/key");
    if(NULL == gpiokey.nd){
        printk("/key node is not find.\r\n");
        return -EINVAL;
    }
    gpiokey.key_gpio = of_get_named_gpio(gpiokey.nd,"key-gpio", 0);
    if(gpiokey.key_gpio < 0){
        printk("Can't find the key-gpio property.\r\n");
        return -EINVAL;
    }
    printk("key-gpio num = %d \r\n", gpiokey.key_gpio);


    //2. Initilize the gpio's value.
    gpio_request(gpiokey.key_gpio, "key0");
    ret = gpio_direction_input(gpiokey.key_gpio);
    if(ret < 0){
        printk("Can't set gpio to input mode.\r\n");
    }

    //3. Get the device identifer number.
    alloc_chrdev_region(&gpiokey.devid, 0, GPIOkey_CNT, GPIOkey_NAME);
    gpiokey.major = MAJOR(gpiokey.devid);
    gpiokey.minor = MINOR(gpiokey.devid);

    //4. Initilize the cdev.
    gpiokey.cdev.owner = THIS_MODULE;
    cdev_init(&gpiokey.cdev, &gpiokey_fops);
    cdev_add(&gpiokey.cdev, gpiokey.devid, GPIOkey_CNT);

    //5. Create a class.
    gpiokey.class = class_create(THIS_MODULE, GPIOkey_NAME);
    if(IS_ERR(gpiokey.class)){
        return PTR_ERR(gpiokey.class);
    }

    //6. Create a device.
    gpiokey.device = device_create(gpiokey.class, NULL, gpiokey.devid, NULL, GPIOkey_NAME);
    if(IS_ERR(gpiokey.device)){
        return PTR_ERR(gpiokey.device);
    }

    return 0;
}

static void __exit key_exit(void)
{
    cdev_del(&gpiokey.cdev);
    unregister_chrdev_region(gpiokey.devid, GPIOkey_CNT);
    device_destroy(gpiokey.class, gpiokey.devid);
    class_destroy(gpiokey.class);
}

MODULE_AUTHOR("WML");
MODULE_LICENSE("GPL");

module_init(key_init);
module_exit(key_exit);


static int key_open(struct inode *nd, struct file *fp)
{
    fp->private_data = &gpiokey;
    return 0;
}

static ssize_t key_read(struct file *fp, \
            char __user *u_buf, \
            size_t u_size, \
            loff_t *u_loft)
{
    int ret = 0;
    unsigned char value;
    struct gpiokey_dev *dev = fp->private_data;
    if(gpio_get_value(dev->key_gpio) == 0){
        while(!gpio_get_value(dev->key_gpio));
        atomic_set(&dev->keyvalue, KEY0VALUE);
    }else{
        atomic_set(&dev->keyvalue, INVAKEY);
    }

    value = atomic_read(&dev->keyvalue);
    ret = copy_to_user(u_buf, &value, sizeof(value));
    return ret;
}

static ssize_t key_write(struct file *fp,\
             const char __user *u_buf,\
             size_t u_size, \
             loff_t *u_loff)
{
    return 0;
}

static int key_release(struct inode *np,\
                struct file *fp)
{
    return 0;
}
