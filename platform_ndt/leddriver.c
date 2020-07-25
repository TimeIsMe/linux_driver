#include <linux/module.h>
#include <linux/init.h>
#include <linux/printk.h>
#include <linux/platform_device.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/resource.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <asm/io.h>


#define LEDDEV_NAME "platled"
#define LEDDEV_CNT  1
#define LED_ON      1
#define LED_OFF     0

static void __iomem *IMX6U_CCM_CCGR1;
static void __iomem *SW_MUX_GPIO1_IO03;
static void __iomem *SW_PAD_GPIO1_IO03;
static void __iomem *GPIO1_DR;
static void __iomem *GPIO1_GDIR;

struct myled_dev{
    dev_t dev_id;
    struct cdev cdev;
    struct class *class;
    struct device *device;
};

struct myled_dev leddev;


static int led_open(struct inode *inode_p, struct file *file_p)
{
    file_p->private_data = &leddev;
    return 0;
}

static ssize_t led_write(struct file *file_p, const char __user *u_buf, size_t u_size, loff_t *u_loff)
{
    int retval;
    u32 tmp = 0;
    unsigned char databuf[1];
    unsigned char ledstat;
    retval = copy_from_user(databuf, u_buf, u_size);
    if(retval < 0){
        return -EFAULT;
    }

    ledstat = databuf[0];
    if(ledstat == LED_ON){
        tmp = readl(GPIO1_DR);
        tmp &= ~(1<<3);
        writel(tmp, GPIO1_DR);
    }else if(ledstat == LED_OFF){
        tmp = readl(GPIO1_DR);
        tmp |= (1<<3);
        writel(tmp, GPIO1_DR);
    }
    return 0;
}

struct file_operations led_fops = {
    .open = led_open,
    .write = led_write,
};

static int led_probe(struct platform_device *dev)
{
    int i = 0;
    struct resource *ledresource[5];
    int ressize[5];
    u32 val = 0;

    //Get resource.
    for(i = 0; i < 5; i++){
        ledresource[i] = platform_get_resource(dev, IORESOURCE_MEM, i);
        if(!ledresource[i]){
            dev_err(&dev->dev, "No MEM resource for always on!\r\n");
            return -ENXIO;
        }
        ressize[i] = resource_size(ledresource[i]);
    }

    //Register map.
    IMX6U_CCM_CCGR1 = ioremap(ledresource[0]->start, ressize[0]);
    SW_MUX_GPIO1_IO03 = ioremap(ledresource[1]->start, ressize[1]);
    SW_PAD_GPIO1_IO03 = ioremap(ledresource[2]->start, ressize[2]);
    GPIO1_DR = ioremap(ledresource[3]->start, ressize[3]);
    GPIO1_GDIR = ioremap(ledresource[4]->start, ressize[4]);

    //Configured for register.
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

    //1. Crate ddevice id.
    alloc_chrdev_region(&leddev.dev_id, 0, LEDDEV_CNT, LEDDEV_NAME);

    //2. Initialize cdev.
    leddev.cdev.owner = THIS_MODULE;
    cdev_init(&leddev.cdev, &led_fops);

    //3. Add cdev to kernel.
    cdev_add(&leddev.cdev, leddev.dev_id, LEDDEV_CNT);

    //4. Create class.
    leddev.class = class_create(THIS_MODULE, LEDDEV_NAME);
    if(IS_ERR(leddev.class)){
        return PTR_ERR(leddev.class);
    }

    //5. Create a device.
    leddev.device = device_create(leddev.class, NULL, leddev.dev_id, NULL, LEDDEV_NAME);
    if(IS_ERR(leddev.device)){
        return PTR_ERR(leddev.device);
    }
    return 0;  
}

static int led_remove(struct platform_device *dev)
{
    iounmap(IMX6U_CCM_CCGR1);
    iounmap(SW_MUX_GPIO1_IO03);
    iounmap(SW_PAD_GPIO1_IO03);
    iounmap(GPIO1_DR);
    iounmap(GPIO1_GDIR);
    device_destroy(leddev.class, leddev.dev_id);
    class_destroy(leddev.class);
    cdev_del(&leddev.cdev);
    unregister_chrdev(MAJOR(leddev.dev_id), LEDDEV_NAME);
    return 0;
}

static struct platform_driver led_driver = {
    .driver = {
        .name = "platform-led",
    },
    .probe = led_probe,
    .remove = led_remove,
};

static int __init led_init(void)
{
    return platform_driver_register(&led_driver);
}

static void __exit led_exit(void)
{
    platform_driver_unregister(&led_driver);
}



module_init(led_init);
module_exit(led_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("WML");

