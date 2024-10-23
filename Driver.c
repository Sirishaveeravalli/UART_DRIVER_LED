/* UART with RGB LED Driver for Mock Testing (No Hardware) */

// Header Files
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <asm/io.h>

#define MYDEV_NAME "devUART0"
#define LLL_MAX_USER_SIZE 1024

#define PIN_ON _IOW('c', 'a', int32_t*)
int32_t val = 0;

static struct class *ptr;
static struct device *temp;

// GPIO pin mappings for the RGB LED
#define LED_R_PIN 16
#define LED_G_PIN 20
#define LED_B_PIN 21

// Function prototypes
static ssize_t myread(struct file *, char __user *, size_t, loff_t *);
static ssize_t mywrite(struct file *, const char __user *, size_t, loff_t *);
static int myopen(struct inode *, struct file *);
static int myclose(struct inode *, struct file *);
static long myioctl(struct file *file, unsigned int cmd, unsigned long arg);

dev_t devno;
struct cdev mycdev;

struct file_operations f_ops = {
    .open = myopen,
    .release = myclose,
    .unlocked_ioctl = myioctl,
    .read = myread,
    .write = mywrite
};

// GPIO control macros for simulation (log messages instead of actual GPIO control)
#define SET_GPIO_PIN(gp, pin) printk(KERN_INFO "GPIO pin %d SET (Simulated)\n", pin)
#define CLR_GPIO_PIN(gp, pin) printk(KERN_INFO "GPIO pin %d CLEARED (Simulated)\n", pin)

static int myopen(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "Device file opened successfully\n");

    try_module_get(THIS_MODULE);

    if (file->f_mode & FMODE_READ)
        printk(KERN_INFO "Opened for read mode\n");
    if (file->f_mode & FMODE_WRITE)
        printk(KERN_INFO "Opened for write mode\n");

    return 0;
}

static int myclose(struct inode *inode, struct file *file)
{
    module_put(THIS_MODULE);
    printk(KERN_INFO "Device file closed\n");
    return 0;
}

static int __init gpio_driver_init(void)
{
    int ret;

    printk(KERN_INFO "Welcome to the UART GPIO driver (Mocked version)!\n");

    ret = alloc_chrdev_region(&devno, 0, 1, "GPIO_RPI");
    if (ret < 0) {
        pr_info("Failed to register the device\n");
        return -1;
    }

    cdev_init(&mycdev, &f_ops);
    cdev_add(&mycdev, devno, 1);

    pr_info("Creating device file dynamically\n");

    ptr = class_create(THIS_MODULE, "sysmychar1");
    if (ptr == NULL) {
        pr_info("Failed to create class\n");
        return -1;
    }

    temp = device_create(ptr, NULL, devno, NULL, "GPIO_DEV");
    if (temp == NULL) {
        pr_info("Failed to create device file\n");
        return -1;
    }

    // Simulating the mapping of GPIO registers
    printk(KERN_INFO "Simulating GPIO register mapping\n");

    // Simulate turning on LED-R after loading the module
    printk(KERN_INFO "Simulated: LED-R ON after module load\n");

    printk(KERN_INFO "GPIO driver successfully initialized\n");
    return 0;
}

static ssize_t myread(struct file *filp, char __user *buf, size_t len, loff_t *offset)
{
    printk(KERN_INFO "Read operation started, simulating LED-G blink\n");

    // Simulating LED-G blinking
    for (int i = 0; i < 3; i++) {
        SET_GPIO_PIN(NULL, LED_G_PIN);
        msleep(500);
        CLR_GPIO_PIN(NULL, LED_G_PIN);
        msleep(500);
    }

    printk(KERN_INFO "Read operation completed\n");
    return 0;
}

static ssize_t mywrite(struct file *filp, const char __user *buf, size_t len, loff_t *offset)
{
    printk(KERN_INFO "Write operation started, simulating LED-B blink\n");

    // Simulating LED-B blinking
    for (int i = 0; i < 3; i++) {
        SET_GPIO_PIN(NULL, LED_B_PIN);
        msleep(500);
        CLR_GPIO_PIN(NULL, LED_B_PIN);
        msleep(500);
    }

    printk(KERN_INFO "Write operation completed\n");
    return len;
}

static long myioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    switch (cmd) {
        case PIN_ON:
            printk(KERN_INFO "IOCTL Command received: PIN_ON\n");
            if (copy_from_user(&val, (int32_t *)arg, sizeof(val))) {
                pr_err("Data Write: Error!\n");
            }
            pr_info("The value: %d\n", val);
            if (val == 1) {
                SET_GPIO_PIN(NULL, LED_R_PIN);
                pr_info("Simulated: LED-R ON\n");
            } else {
                CLR_GPIO_PIN(NULL, LED_R_PIN);
                pr_info("Simulated: LED-R OFF\n");
            }
            break;
    }
    return 0;
}

static void __exit gpio_driver_exit(void)
{
    printk(KERN_INFO "Module removal started, simulating all LEDs blink thrice\n");

    // Simulate blinking all LEDs three times during module removal
    for (int i = 0; i < 3; i++) {
        SET_GPIO_PIN(NULL, LED_R_PIN);
        SET_GPIO_PIN(NULL, LED_G_PIN);
        SET_GPIO_PIN(NULL, LED_B_PIN);
        msleep(500);
        CLR_GPIO_PIN(NULL, LED_R_PIN);
        CLR_GPIO_PIN(NULL, LED_G_PIN);
        CLR_GPIO_PIN(NULL, LED_B_PIN);
        msleep(500);
    }

    // Cleanup
    cdev_del(&mycdev);
    unregister_chrdev_region(devno, 1);
    device_destroy(ptr, devno);
    class_destroy(ptr);

    printk(KERN_INFO "GPIO driver unloaded (Mocked)\n");
}

module_init(gpio_driver_init);
module_exit(gpio_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("UART with RGB LED driver for Raspberry Pi 2 GPIO (Mocked for Testing)");
MODULE_VERSION("1.0");

