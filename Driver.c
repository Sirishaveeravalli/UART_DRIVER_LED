/* UART with RGB LED Driver for Mock Testing (No Hardware) 

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
MODULE_VERSION("1.0");*/



#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>
#include <linux/timer.h>
#include <linux/init.h>

#define DEVICE_NAME "uart_rgb_led"
#define UART_PORT "/dev/ttyAMA0"
#define GPIO_R 16
#define GPIO_G 20
#define GPIO_B 21

static int major;
static char uart_buffer[256] = {0};
static struct timer_list blink_timer;
static int led_state = 0;
static int blink_count = 0;

// Function prototypes
static int dev_open(struct inode *, struct file *);
static int dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);
static long dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

// File operations structure
static struct file_operations fops = {
    .open = dev_open,
    .read = dev_read,
    .write = dev_write,
    .release = dev_release,
    .unlocked_ioctl = dev_ioctl,
};

// Function to blink the LEDs
static void blink_led(struct timer_list *timer) {
    led_state = !led_state;
    gpio_set_value(GPIO_B, led_state);
    gpio_set_value(GPIO_G, led_state);
    
    if (blink_count < 3) {
        blink_count++;
        mod_timer(&blink_timer, jiffies + msecs_to_jiffies(500));
    } else {
        gpio_set_value(GPIO_B, 0);
        gpio_set_value(GPIO_G, 0);
        gpio_set_value(GPIO_R, 0);
        blink_count = 0;
    }
}

// Open function
static int dev_open(struct inode *inodep, struct file *filep) {
    printk(KERN_INFO "UART-LED: Device opened\n");
    gpio_set_value(GPIO_R, 1);  // LED-R ON after loading module
    return 0;
}

// Read function
static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset) {
    int bytes_read = len;
    int result = copy_to_user(buffer, uart_buffer, bytes_read);
    
    if (result == 0) {
        gpio_set_value(GPIO_G, 1);  // LED-G blinking for read operation
        mod_timer(&blink_timer, jiffies + msecs_to_jiffies(500));
        printk(KERN_INFO "UART-LED: Sent %d bytes to the user\n", bytes_read);
        return bytes_read;
    } else {
        printk(KERN_ERR "UART-LED: Failed to send %d bytes\n", result);
        return -EFAULT;
    }
}

// Write function
static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset) {
    int result = copy_from_user(uart_buffer, buffer, len);
    
    if (result == 0) {
        gpio_set_value(GPIO_B, 1);  // LED-B blinking for write operation
        mod_timer(&blink_timer, jiffies + msecs_to_jiffies(500));
        printk(KERN_INFO "UART-LED: Received %d bytes from the user\n", len);
        return len;
    } else {
        printk(KERN_ERR "UART-LED: Failed to receive %d bytes\n", result);
        return -EFAULT;
    }
}

// IOCTL function
static long dev_ioctl(struct file *filep, unsigned int cmd, unsigned long arg) {
    printk(KERN_INFO "UART-LED: IOCTL called\n");
    return 0;
}

// Release function
static int dev_release(struct inode *inodep, struct file *filep) {
    printk(KERN_INFO "UART-LED: Device successfully closed\n");
    return 0;
}

// Module init function
static int __init uart_led_init(void) {
    major = register_chrdev(0, DEVICE_NAME, &fops);
    if (major < 0) {
        printk(KERN_ALERT "UART-LED: Failed to register device\n");
        return major;
    }

    // GPIO setup
    gpio_request(GPIO_R, "LED_R");
    gpio_direction_output(GPIO_R, 0);
    gpio_request(GPIO_G, "LED_G");
    gpio_direction_output(GPIO_G, 0);
    gpio_request(GPIO_B, "LED_B");
    gpio_direction_output(GPIO_B, 0);

    // Timer setup
    timer_setup(&blink_timer, blink_led, 0);

    printk(KERN_INFO "UART-LED: Device registered with major number %d\n", major);
    return 0;
}

// Module exit function
static void __exit uart_led_exit(void) {
    gpio_set_value(GPIO_R, 0);  // Turn off all LEDs
    gpio_set_value(GPIO_G, 0);
    gpio_set_value(GPIO_B, 0);

    del_timer(&blink_timer);  // Delete the timer

    gpio_free(GPIO_R);  // Free GPIOs
    gpio_free(GPIO_G);
    gpio_free(GPIO_B);

    unregister_chrdev(major, DEVICE_NAME);
    printk(KERN_INFO "UART-LED: Device unregistered\n");
}

module_init(uart_led_init);
module_exit(uart_led_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("A UART and LED control driver");
MODULE_VERSION("1.0");
