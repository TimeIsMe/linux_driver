#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/resource.h>
#include <linux/printk.h>
#include <linux/init.h>

#define CCM_CCGR1_BASE				(0X020C406C)	
#define SW_MUX_GPIO1_IO03_BASE		(0X020E0068)
#define SW_PAD_GPIO1_IO03_BASE		(0X020E02F4)
#define GPIO1_DR_BASE				(0X0209C000)
#define GPIO1_GDIR_BASE				(0X0209C004)
#define REGISTER_LENGTH				4

static struct resource led_resource[] = {
    [0] = {
        .start = CCM_CCGR1_BASE,
        .end = CCM_CCGR1_BASE + REGISTER_LENGTH -1,
        .flags = IORESOURCE_MEM,
    },
    [1] = {
        .start = SW_MUX_GPIO1_IO03_BASE,
        .end = SW_MUX_GPIO1_IO03_BASE + REGISTER_LENGTH -1,
        .flags = IORESOURCE_MEM,
    },
    [2] = {
        .start = SW_PAD_GPIO1_IO03_BASE,
        .end = SW_PAD_GPIO1_IO03_BASE + REGISTER_LENGTH -1,
        .flags = IORESOURCE_MEM,
    },
    [3] = {
        .start = GPIO1_DR_BASE,
        .end = GPIO1_DR_BASE + REGISTER_LENGTH - 1,
        .flags = IORESOURCE_MEM,
    },
    [4] = {
        .start = GPIO1_GDIR_BASE,
        .end = GPIO1_GDIR_BASE + REGISTER_LENGTH -1,
        .flags = IORESOURCE_MEM,
    },
};

static void led_release(struct device *dev){
    printk("led device released!\r\n");
}

static struct platform_device leddevice = {
    .name = "platform-led",
    .id = -1,
    .dev = {
        .release = led_release,
    },
    .num_resources = ARRAY_SIZE(led_resource),
    .resource = led_resource,
};

static int __init leddevice_init(void)
{
    return platform_device_register(&leddevice);
}

static void __exit leddevice_exit(void)
{
    platform_device_unregister(&leddevice);
}

MODULE_AUTHOR("WML");
MODULE_LICENSE("GPL");
module_init(leddevice_init);
module_exit(leddevice_exit);
