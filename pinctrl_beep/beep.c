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


#define GPIOLED_CNT     1
#define GPIOLED_NAME    "mybeep"
#define LEDOFF          0
#define LEDON           1

//Function declaration.
static int led_open(struct inode *nd, struct file *fp);
static ssize_t led_read(struct file *fp, \
            char __user *u_buf, \
            size_t u_size, \
            loff_t *u_loft);
static ssize_t led_write(struct file *fp,\
             const char __user *u_buf,\
             size_t u_size, \
             loff_t *u_loff);
static int led_release(struct inode *np,\
                struct file *fp);


static struct gpioled_dev{
    dev_t   devid;              //Device indentifer number.
    struct cdev cdev;
    struct class *class;
    struct device *device;
    int major;
    int minor;
    struct device_node *nd;     
    int led_gpio;               //Number of gpio used by led.
} gpioled;                      //Led device.

static struct file_operations gpioled_fops = {
    .owner = THIS_MODULE,
    .open = led_open,
    .read = led_read,
    .write = led_write,
    .release = led_release,
};

static int __init led_init(void)
{
    int ret = 0;


    //1. get device tree node.
    gpioled.nd = of_find_node_by_path("/beep");
    if(NULL == gpioled.nd){
        printk("/beep node is not find.\r\n");
        return -EINVAL;
    }
    gpioled.led_gpio = of_get_named_gpio(gpioled.nd,"beep-gpio", 0);
    if(gpioled.led_gpio < 0){
        printk("Can't find the led-gpio property.\r\n");
        return -EINVAL;
    }
    printk("led-gpio num = %d \r\n", gpioled.led_gpio);


    //2. Initilize the gpio's value.
    ret = gpio_direction_output(gpioled.led_gpio, 1);
    if(ret < 0){
        printk("Can't set gpio.\r\n");
    }

    //3. Get the device identifer number.
    alloc_chrdev_region(&gpioled.devid, 0, GPIOLED_CNT, GPIOLED_NAME);
    gpioled.major = MAJOR(gpioled.devid);
    gpioled.minor = MINOR(gpioled.devid);

    //4. Initilize the cdev.
    gpioled.cdev.owner = THIS_MODULE;
    cdev_init(&gpioled.cdev, &gpioled_fops);
    cdev_add(&gpioled.cdev, gpioled.devid, GPIOLED_CNT);

    //5. Create a class.
    gpioled.class = class_create(THIS_MODULE, GPIOLED_NAME);
    if(IS_ERR(gpioled.class)){
        return PTR_ERR(gpioled.class);
    }

    //6. Create a device.
    gpioled.device = device_create(gpioled.class, NULL, gpioled.devid, NULL, GPIOLED_NAME);
    if(IS_ERR(gpioled.device)){
        return PTR_ERR(gpioled.device);
    }

    return 0;
}

static void __exit led_exit(void)
{
    cdev_del(&gpioled.cdev);
    unregister_chrdev_region(gpioled.devid, GPIOLED_CNT);
    device_destroy(gpioled.class, gpioled.devid);
    class_destroy(gpioled.class);
}

MODULE_AUTHOR("WML");
MODULE_LICENSE("GPL");

module_init(led_init);
module_exit(led_exit);


static int led_open(struct inode *nd, struct file *fp)
{
    fp->private_data = &gpioled;
    return 0;
}

static ssize_t led_read(struct file *fp, \
            char __user *u_buf, \
            size_t u_size, \
            loff_t *u_loft)
{
    return 0;
}

static ssize_t led_write(struct file *fp,\
             const char __user *u_buf,\
             size_t u_size, \
             loff_t *u_loff)
{
    int retval;
    unsigned char databuf[1];
    unsigned char ledstat;
    struct gpioled_dev *dev = fp->private_data;

    retval = copy_from_user(databuf, u_buf, u_size);
    if(retval < 0){
        printk("Kernel write failed!\r\n");
        return -EINVAL;
    }

    ledstat = databuf[0];

    if(ledstat == LEDON){
        gpio_set_value(dev->led_gpio, 0);
    }else if(ledstat == LEDOFF){
        gpio_set_value(dev->led_gpio, 1);
    }
    return 0;
}

static int led_release(struct inode *np,\
                struct file *fp)
{
    return 0;
}
