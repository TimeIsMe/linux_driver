#include <linux/types.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/printk.h>
#include <linux/errno.h>
#include <linux/compiler.h>
//#include <linux/gpio.h>
//#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#define DEV_CNT 1

static int led_open(struct inode * inode,struct file *ifile);
static int led_release(struct inode *inode, struct file *ifile);
static ssize_t led_read(struct file *ifile, char __user *ubuf, size_t cnt,loff_t *offset);
static ssize_t led_write(struct file *ifile, const char __user *ubuf, size_t cnt, loff_t *offset);


struct myleddev{
    dev_t dev_id;
    u16 major;
    u16 minor;
    struct cdev cdev;
    struct class *class;
    struct device *device;
    struct device_node *nd;
};

struct file_operations myled_fops={
    .owner = THIS_MODULE,
    .open = led_open,
    .release = led_release,
    .read = led_read,
    .write = led_write
};

static struct myleddev myled;
static void __iomem *IMX6U_CCM_CCGR1;
static void __iomem *SW_MUX_GPIO1_IO03;
static void __iomem *SW_PAD_GPIO1_IO03;
static void __iomem *GPIO1_DR;
static void __iomem *GPIO1_GDIR;
static void __iomem *GPIO1_PSR;

static int __init myled_init(void)
{
    int ret = 0;
    u32 val = 0;
    struct property *proper;
    const char* str;
    u32 regdata[14];
    u8 i = 0;
    printk("#myled: init function.\r\n");

    //step negative one.
    myled.nd = of_find_node_by_path("/myled");
    if(myled.nd == NULL){
        printk("myled node nost find!\r\n");
        return -EINVAL;
    }
    printk("#myled: myled node find!\r\n");
    proper = of_find_property(myled.nd,"compatible",NULL);
    if(NULL == proper){
        printk("#myled: compatible property find failed\r\n");
    }
    printk("#myled: compatible=%s\r\n",(char*)proper->value);
    ret = of_property_read_string(myled.nd, "status", &str);
    if(ret < 0){
        printk("#myled: status read failed!\r\n");
    }
    printk("#myled: status = %s", str);

    ret = of_property_read_u32_array(myled.nd, "reg", regdata, 12);
    if(ret < 0){
        printk("#myled: reg property read failed!\r\n");
    }else{
        printk("#myled: reg data:\r\n");
        for(i = 0; i<10; i++){
            printk("\t%#X ",regdata[i]);
        }
        printk("\r\n");
    }

    //step zero.
    IMX6U_CCM_CCGR1 = of_iomap(myled.nd,0);
    SW_MUX_GPIO1_IO03 = of_iomap(myled.nd,1);
    SW_PAD_GPIO1_IO03 = of_iomap(myled.nd,2);
    GPIO1_DR = of_iomap(myled.nd,3);
    GPIO1_GDIR = of_iomap(myled.nd,4);
    GPIO1_PSR = of_iomap(myled.nd,5);

    //step zero point five.
    val = readl(IMX6U_CCM_CCGR1);
    val &= ~(3<<26);
    val |= (3<<26);
    writel(val, IMX6U_CCM_CCGR1);

    writel(5, SW_MUX_GPIO1_IO03);

    writel(0x10B0, SW_PAD_GPIO1_IO03);

    val = readl(GPIO1_GDIR);
    val &= ~(1<<3);
    val |= (1<<3);
    writel(val, GPIO1_GDIR);

    val = readl(GPIO1_DR);
    val |= (1<<3);
    writel(val, GPIO1_DR);
    
    //step one.
    ret = alloc_chrdev_region(&(myled.dev_id),0,DEV_CNT,"myled");
    if(ret){
        goto err1;
    }
    myled.major = MAJOR(myled.dev_id);
    myled.minor = MINOR(myled.dev_id);

    //step two.
    myled.cdev.owner = THIS_MODULE;
    cdev_init(&myled.cdev,&myled_fops);
    cdev_add(&myled.cdev,myled.dev_id,DEV_CNT);

    //step three.
    myled.class = class_create(THIS_MODULE,"myled");
    if(IS_ERR(myled.class)){
        goto err2;
    }

    //step four.
    myled.device = device_create(myled.class,NULL,\
                                myled.dev_id,NULL,"myled");
    if(IS_ERR(myled.device)){
        goto err3;
    }


    return 0;
err3:
    return PTR_RET(myled.device);
err2:
    return PTR_RET(myled.class);

err1:
    return ret;
}

static void __exit myled_exit(void)
{
    iounmap(IMX6U_CCM_CCGR1);
    iounmap(SW_MUX_GPIO1_IO03);
    iounmap(SW_PAD_GPIO1_IO03);
    iounmap(GPIO1_DR);
    iounmap(GPIO1_GDIR);

    cdev_del(&myled.cdev);
    unregister_chrdev(myled.dev_id, "myled");
    device_destroy(myled.class, myled.dev_id);
    class_destroy(myled.class);
}

static int led_open(struct inode * inode,\
                    struct file *ifile)
{
    ifile->private_data = &myled;
    return 0;
}
static int led_release(struct inode *inode, \
                        struct file *ifile)
{
    return 0;
}
static ssize_t led_read(struct file *ifile,\
                        char __user *ubuf,\
                        size_t cnt,\
                        loff_t *offset)
{
    unsigned char sta;
    int retval;
    u32 val = 0x00;
#if 0
    val = readl(GPIO1_PSR);
    val &= ~(1<<3);
    writel(val,GPIO1_PSR);
#endif
    val = readl(GPIO1_DR);
    if(val & (1<<3)){
        sta = 1;
    }else{
        sta = 0;
    }
    retval = copy_to_user(ubuf, &sta, 1);
    return 0;
}

static void led_switch(u8 sta)
{
    u32 val = 0;
    if(sta == 0){
        val = readl(GPIO1_DR);
        val |= (1<<3);
        writel(val, GPIO1_DR);
    }else{
        val = readl(GPIO1_DR);
        val &= ~(1<<3);
        writel(val,GPIO1_DR);
    }
}

static ssize_t led_write(struct file *ifile, \
                        const char __user *ubuf, \
                        size_t cnt, \
                        loff_t *offset)
{
    unsigned char databuf[1];
    int retval;
    retval = copy_from_user(databuf,ubuf,cnt);
    if(retval < 0){
        printk("#led_write: kernel write failed!\r\n");
        return -EFAULT;
    }
    if(databuf[0] == 0){
        printk("#led_write: led off\r\n");
        led_switch(0);
    }else if(databuf[0] == 1){
        printk("#led_write: led on\r\n");
        led_switch(1);
    }else{
        printk("#led_write: illegal led status\r\n");
    }
    return 0;
}


module_init(myled_init);
module_exit(myled_exit);

MODULE_AUTHOR("WML");
MODULE_LICENSE("GPL");