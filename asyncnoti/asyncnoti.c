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
#include <linux/irq.h>
#include <linux/of_irq.h>
#include <linux/interrupt.h>
#include <linux/timer.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/poll.h>
#include <linux/fcntl.h>


#define GPIOkey_CNT     1
#define GPIOkey_NAME    "asyncnoti"
#define KEY0VALUE       0xF0
#define INVAKEY         0x00
#define KEY_NUM         1

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
static unsigned int key_poll(struct file *fp, \
                struct poll_table_struct *wait);
static int key_fasync(int fd, struct file *fp, int on);



struct irq_keydesc{
    int gpio;
    int irqnum;
    unsigned char value;
    char name[10];
    irqreturn_t (*handler)(int, void*);
};

static struct gpiokey_dev{
    dev_t   devid;              //Device indentifer number.
    struct cdev cdev;
    struct class *class;
    struct device *device;
    int major;
    int minor;
    struct device_node *nd;     
    int key_gpio;               //Number of gpio used by key.
    atomic_t    keyvalue;       //A valid key value;
    atomic_t    releasekey;     //Flag of the release.
    struct timer_list timer;
    struct irq_keydesc irqkeydesc[KEY_NUM];
    unsigned char curkeynum;    //Current number of the key.
    wait_queue_head_t r_wait;   
    struct fasync_struct *async_queue;
} gpiokey;                      //key device.

static struct file_operations gpiokey_fops = {
    .owner = THIS_MODULE,
    .open = key_open,
    .read = key_read,
    .write = key_write,
    .release = key_release,
    .poll = key_poll,
    .fasync = key_fasync,
};


static irqreturn_t key0_handler(int irq, void *dev_id){
    struct gpiokey_dev *dev = (struct gpiokey_dev *)dev_id;
    dev->curkeynum = 0;
    dev->timer.data = (volatile long)dev_id;
    mod_timer(&dev->timer, msecs_to_jiffies(10));
    return IRQ_RETVAL(IRQ_HANDLED);
}


void timer_function(unsigned long arg){
    unsigned char value;
    unsigned char num;
    struct irq_keydesc *keydesc;
    struct gpiokey_dev *dev = (struct gpiokey_dev*)arg;
    num = dev->curkeynum;
    keydesc = &dev->irqkeydesc[num];
    value = gpio_get_value(keydesc->gpio);
    if(0 == value){
        atomic_set(&dev->keyvalue, keydesc->value);
    }
    else{
        atomic_set(&dev->keyvalue, 0x80 | keydesc->value);
        atomic_set(&dev->releasekey, 1);
    }
    if(atomic_read(&dev->releasekey)){
        if(dev->async_queue){
            kill_fasync(&dev->async_queue, SIGIO, POLL_IN);
        }
    }
/*
    if(atomic_read(&dev->releasekey)){
        wake_up_interruptible(&dev->r_wait);
    }
*/
}

static int __init imx6ulkey_init(void)
{
    int ret = 0;
    unsigned char i = 0;
    char name[10];

    atomic_set(&gpiokey.keyvalue, INVAKEY);

    //1. get device tree node.
    gpiokey.nd = of_find_node_by_path("/key");
    if(NULL == gpiokey.nd){
        printk("/key node is not find.\r\n");
        return -EINVAL;
    }
    for(i = 0; i<KEY_NUM; i++){
        gpiokey.irqkeydesc[i].gpio = of_get_named_gpio(gpiokey.nd, "key-gpio",i);
        if(gpiokey.irqkeydesc[i].gpio<0){
            printk("can't get key%d\r\n", i);
        }
    }
    //2. Initilize the gpio's value.
    for(i = 0; i < KEY_NUM; i++){
        memset(gpiokey.irqkeydesc[i].name, 0, sizeof(name));
        sprintf(gpiokey.irqkeydesc[i].name, "KEY%d", i);
        gpio_request(gpiokey.irqkeydesc[i].gpio, name);
        gpio_direction_input(gpiokey.irqkeydesc[i].gpio);
        gpiokey.irqkeydesc[i].irqnum = irq_of_parse_and_map(gpiokey.nd, i);
        printk("key%d:gpio=%d, irqnum=%d\r\n", i, gpiokey.irqkeydesc[i].gpio, gpiokey.irqkeydesc[i].irqnum);
    }
    gpiokey.irqkeydesc[0].handler = key0_handler;
    gpiokey.irqkeydesc[0].value = KEY0VALUE;

    for(i=0; i<KEY_NUM; i++){
        ret = request_irq(gpiokey.irqkeydesc[i].irqnum,\
                            gpiokey.irqkeydesc[i].handler,\
                            IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,\
                            gpiokey.irqkeydesc[i].name, &gpiokey);
        if(ret < 0){
            printk("irq %d interrupt failed!\r\n", gpiokey.irqkeydesc[i].irqnum);
            return -EFAULT;
        }
    }

    //Initialize the timer.
    init_timer(&gpiokey.timer);
    gpiokey.timer.function = timer_function;

    //Initialize the wait queue head.
    init_waitqueue_head(&gpiokey.r_wait);

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

    atomic_set(&gpiokey.keyvalue, INVAKEY);
    atomic_set(&gpiokey.releasekey, 0);

    return 0;
}

static void __exit imx6ulkey_exit(void)
{
    unsigned i = 0;
    del_timer_sync(&gpiokey.timer);
    for(i =0; i < KEY_NUM; i++){
        free_irq(gpiokey.irqkeydesc[i].irqnum, &gpiokey);
    }
    cdev_del(&gpiokey.cdev);
    unregister_chrdev_region(gpiokey.devid, GPIOkey_CNT);
    device_destroy(gpiokey.class, gpiokey.devid);
    class_destroy(gpiokey.class);
}

MODULE_AUTHOR("WML");
MODULE_LICENSE("GPL");

module_init(imx6ulkey_init);
module_exit(imx6ulkey_exit);


static int key_open(struct inode *nd, struct file *fp)
{
    fp->private_data = &gpiokey;
    return 0;
}

static unsigned int key_poll(struct file *fp, struct poll_table_struct *wait)
{  
    unsigned int mask = 0;
    struct gpiokey_dev *dev = (struct gpiokey_dev *)fp->private_data;
    poll_wait(fp, &dev->r_wait, wait);
    if(atomic_read(&dev->releasekey)){                          //Key0 was pressed.
        mask = POLLIN | POLLRDNORM;
    }
    return mask;
}

static ssize_t key_read(struct file *fp, \
            char __user *u_buf, \
            size_t u_size, \
            loff_t *u_loft)
{
    int ret = 0;
    unsigned char keyvalue = 0;
    unsigned char releasekey = 0;
    struct gpiokey_dev *dev = (struct gpiokey_dev *)fp->private_data;
/*
    DECLARE_WAITQUEUE(wait, current);
*/

    //Nonblock read.
    if(fp->f_flags & O_NONBLOCK){
        if(atomic_read(&dev->releasekey) == 0){
            return -EAGAIN;
        }
    }else{                                          //Block read.
        ret = wait_event_interruptible(dev->r_wait, atomic_read(&dev->releasekey));
        if(ret){
            goto wait_error;
        }
    }

/*
    if(atomic_read(&dev->releasekey) == 0){         //No key was pressed.
        add_wait_queue(&dev->r_wait, &wait);
        __set_current_state(TASK_INTERRUPTIBLE);
        schedule();
        if(signal_pending(current)){                //Judge waked up signal is not current signal.
            ret = -ERESTARTSYS;
            goto wait_error;
        }
    }
    remove_wait_queue(&dev->r_wait, &wait);
*/
    keyvalue = atomic_read(&dev->keyvalue);
    releasekey = atomic_read(&dev->releasekey);
    if(releasekey){
        if(keyvalue & 0x80){
            keyvalue &= ~0x80;
            ret = copy_to_user(u_buf, &keyvalue, sizeof(keyvalue));
        }else{
            goto data_error;
        }
        atomic_set(&dev->releasekey, 0);
    }else{
        goto data_error;
    }
    return 0;
data_error:
    return -EINVAL;

wait_error:
/*
    set_current_state(TASK_RUNNING);
    remove_wait_queue(&dev->r_wait, &wait);
*/
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
    key_fasync(-1, fp, 0);
    return 0;
}


static int key_fasync(int fd, struct file *fp, int on)
{
    struct gpiokey_dev *dev = (struct gpiokey_dev *)fp->private_data;
    return fasync_helper(fd, fp, on, &dev->async_queue);
}
