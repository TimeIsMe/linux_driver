#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/printk.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

#define DRIVER_NAME "misc-led"

struct misc_led_dev{
    dev_t dev_id;
    struct cdev cdev;
    struct class *class;
    struct device *device;
    struct device_node *nd;
    int led_gpio;
};

struct misc_led_dev led_dev;



static int evkLedOpen(struct inode *nd, struct file *fp);
static ssize_t evkLedWrite(struct file *fp, const char __user *ubuf, size_t ucnt, loff_t *ulof);


struct file_operations led_fops = {
    .open = evkLedOpen,
    .write = evkLedWrite,
};

struct miscdevice led_miscdev = {
    .minor = 11,
    .name = DRIVER_NAME,
    .fops = &led_fops,
};

static int misc_led_probe(struct platform_device *dev){
    int ret = 0;
    printk("msic led driver was matched!\r\n");
    led_dev.nd = of_find_node_by_path("/gpioled");
    if(NULL == led_dev.nd){
        printk("/gpioled node not found\r\n");
        return -EINVAL;
    }
    led_dev.led_gpio = of_get_named_gpio(led_dev.nd, "led-gpio", 0);
    if(led_dev.led_gpio < 0){
        printk("can't get led gpio\r\n");
        return -EINVAL;
    }

    ret = gpio_direction_output(led_dev.led_gpio, 1);
    if(ret < 0){
        printk("can't set gpio\r\n");
    }

    ret = misc_register(&led_miscdev);
    if(ret < 0){
        printk("misc device register failed\r\n");
        return -EFAULT;
    }
    return 0;
}

static int misc_led_remove(struct platform_device *dev){
    gpio_set_value(led_dev.led_gpio, 1);
    misc_deregister(&led_miscdev);
    return 0;
}


struct of_device_id misc_led_match_table[] = {
    {
        .compatible = "my-gpioled",
    },
    {

    }
};

struct platform_driver misc_led_driver = {
    .driver = {
        .name = DRIVER_NAME,
        .of_match_table = misc_led_match_table,
    },
    .probe = misc_led_probe,
    .remove = misc_led_remove,
};

module_platform_driver(misc_led_driver);

MODULE_AUTHOR("wml");
MODULE_LICENSE("GPL");



static int evkLedOpen(struct inode *nd, struct file *fp)
{
    fp->private_data = &led_dev;
    return 0;
}
static ssize_t evkLedWrite(struct file *fp, const char __user *ubuf, size_t ucnt, loff_t *ulof)
{
    int ret = 0;
    unsigned char databuf[1];
    unsigned char ledstat;
    struct misc_led_dev *dev = fp->private_data;

    ret = copy_from_user(databuf, ubuf, ucnt);

    if(ret < 0){
        printk("failed copy buf from user space\r\n");
        return -EFAULT;
    }

    ledstat = databuf[0];
    if(ledstat){
        gpio_set_value(dev->led_gpio, 0);
        printk("led on\r\n");
    }else{
        gpio_set_value(dev->led_gpio, 1);
        printk("led off\r\n");
    }

    return 0;
}

