#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>
#include <linux/timer.h>
#include <linux/ioctl.h>

#define DEVICE_NAME "devUART0"
#define BUF_LEN 80

// GPIO Pins for RGB LED (set as per hardware)
#define LED_R_PIN 16
#define LED_G_PIN 20
#define LED_B_PIN 21


static int dev_open(struct inode *, struct file *);
static int dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);
static long dev_ioctl(struct file *, unsigned int, unsigned long);


static void blink_led(unsigned int led_pin, int count);
static void led_off(void);
static void led_on(unsigned int led_pin);

static int major_number;
static char msg[BUF_LEN];
static int msg_len;
static struct timer_list blink_timer;

static struct file_operations fops = {
    .open = dev_open,
    .read = dev_read,
    .write = dev_write,
    .unlocked_ioctl = dev_ioctl,
    .release = dev_release,
};


static int dev_open(struct inode *inodep, struct file *filep) {
    led_on(LED_R_PIN);  
    printk(KERN_INFO "devUART0: Device opened\n");
    return 0;
}

static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset) {
    int bytes_read = len < msg_len ? len : msg_len;
    blink_led(LED_G_PIN, 10);  
    if (copy_to_user(buffer, msg, bytes_read)) {
        return -EFAULT;
    }
    printk(KERN_INFO "devUART0: Sent %d characters to user\n", bytes_read);
    return bytes_read;
}


static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset) {
    if (len > BUF_LEN - 1) {
        len = BUF_LEN - 1;
    }
    if (copy_from_user(msg, buffer, len)) {
        return -EFAULT;
    }
    msg[len] = '\0';
    msg_len = len;
    blink_led(LED_B_PIN, 10);  
    printk(KERN_INFO "devUART0: Received %zu characters from user\n", len);
    return len;
}

// IOCTL function
static long dev_ioctl(struct file *filep, unsigned int cmd, unsigned long arg) {
    // Custom IOCTL commands can be added here
    return 0;
}

// Release function
static int dev_release(struct inode *inodep, struct file *filep) {
    blink_led(LED_R_PIN | LED_G_PIN | LED_B_PIN, 3); 
    printk(KERN_INFO "devUART0: Device successfully closed\n");
    return 0;
}


static void blink_led(unsigned int led_pin, int count) {
    int i;
    for (i = 0; i < count; i++) {
        gpio_set_value(led_pin, 1);
        msleep(50);
        gpio_set_value(led_pin, 0);
        msleep(50);
    }
}

// LED control functions
static void led_on(unsigned int led_pin) {
    gpio_set_value(led_pin, 1);
}

static void led_off(void) {
    gpio_set_value(LED_R_PIN, 0);
    gpio_set_value(LED_G_PIN, 0);
    gpio_set_value(LED_B_PIN, 0);
}

static int __init devUART0_init(void) {
    major_number = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_number < 0) {
        printk(KERN_ALERT "devUART0 failed to register a major number\n");
        return major_number;
    }
    printk(KERN_INFO "devUART0: Registered with major number %d\n", major_number);

    gpio_request_one(LED_R_PIN, GPIOF_OUT_INIT_LOW, "LED_R");
    gpio_request_one(LED_G_PIN, GPIOF_OUT_INIT_LOW, "LED_G");
    gpio_request_one(LED_B_PIN, GPIOF_OUT_INIT_LOW, "LED_B");

    return 0;
}

static void __exit devUART0_exit(void) {
    led_off();
    gpio_free(LED_R_PIN);
    gpio_free(LED_G_PIN);
    gpio_free(LED_B_PIN);
    unregister_chrdev(major_number, DEVICE_NAME);
    printk(KERN_INFO "devUART0: Unregistered and exiting\n");
}

module_init(devUART0_init);
module_exit(devUART0_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sirisha");
MODULE_DESCRIPTION("UART Driver with RGB LED Integration");
MODULE_VERSION("1.0");

