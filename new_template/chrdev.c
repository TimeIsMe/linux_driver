#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/printk.h>
#include <asm/uaccess.h>


struct chrtestdev{
    dev_t dev_id;
    unsigned int major;
    unsigned int minor;
    struct cdev  cdev;
    struct class *class;
    struct device *device;

};
static struct chrtestdev chrtestdevice;

static char buf[100]={'\0'};

int chrtest_open(struct inode *inode, struct file *filp);
int chrtest_release(struct inode *inode, struct file *filp);
ssize_t chrtest_read(struct file *filp, char __user *buf, size_t cnt, loff_t *offt);
ssize_t chrtest_write(struct file *filp, const char __user *buf, size_t cnt, loff_t *offt);

struct file_operations chrtest_fops={
    .owner = THIS_MODULE,
    .open = chrtest_open,
    .release  = chrtest_release,
    .read = chrtest_read,
    .write = chrtest_write
};

static int __init chrtest_init(void)
{
    int ret = 0;
    printk("chrtest_init...\r\n");
    ret = alloc_chrdev_region(&(chrtestdevice.dev_id),0,1,"chrtest");
    if(ret){
        goto err1;
    }

    //add char driver to kernel.
    chrtestdevice.cdev.owner = THIS_MODULE;
    chrtestdevice.cdev.dev = chrtestdevice.dev_id;
    cdev_init(&chrtestdevice.cdev,&chrtest_fops);
    cdev_add(&chrtestdevice.cdev,chrtestdevice.dev_id,1);

    //Create class.
    chrtestdevice.class = class_create(THIS_MODULE,"chrtest");
    if(IS_ERR(chrtestdevice.class)){
        goto err2;
    }

    //Create a mount point in /dev/.
    chrtestdevice.device = device_create(chrtestdevice.class,NULL,chrtestdevice.dev_id,NULL,"chrtest");
    if(IS_ERR(chrtestdevice.device)){
        goto err3;
    }

    return 0;
err3:
    return PTR_ERR(chrtestdevice.device);
err2:
    return PTR_ERR(chrtestdevice.class);
err1:
    return ret;

}

static void __exit chrtest_exit(void)
{
    cdev_del(&chrtestdevice.cdev);
    unregister_chrdev_region(chrtestdevice.dev_id,1);

    device_destroy(chrtestdevice.class,chrtestdevice.dev_id);
    class_destroy(chrtestdevice.class);
}


module_init(chrtest_init);
module_exit(chrtest_exit);

MODULE_AUTHOR("WML");
MODULE_LICENSE("GPL");


int chrtest_open(struct inode *inode, struct file *filp)
{
    printk("chrtest open\r\n");
    return 0;
}

int chrtest_release(struct inode *inode, struct file *filp)
{
    printk("chrtest close\r\n");
    return 0;
}
ssize_t chrtest_read(struct file *filp, char __user *buf, size_t cnt, loff_t *offt)
{
    printk("chrtest read\r\n");
    return 0;
}
ssize_t chrtest_write(struct file *filp, const char __user *buf, size_t cnt, loff_t *offt)
{
    printk("chrtest write\r\n");
    return 0;
}