#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>
#include <linux/timer.h>
#include <linux/delay.h>

#define DEVICE_NAME "devUART"     // Device name in /dev/
#define BUF_LEN 100                 // Max length of the message buffer

//gpio Pins
#define LED_R_PIN 16
#define LED_G_PIN 20
#define LED_B_PIN 21

static int major_number;
static char msg[BUF_LEN] = {0};
static short msg_size;
static int open_count = 0;

static struct file_operations fops = {
    .open = dev_open,
    .read = dev_read,
    .write = dev_write,
    .release = dev_release,
};

// Function to set LED state
void set_led_state(unsigned int led_pin, bool state) {
    gpio_set_value(led_pin, state);
}

// Function to blink LED with a 50% duty cycle
void blink_led(unsigned int led_pin, int times) {
    for (int i = 0; i < times; i++) {
        set_led_state(led_pin, 1);
        msleep(100);  // 100ms ON time
        set_led_state(led_pin, 0);
        msleep(100);  // 100ms OFF time
    }
}

// Device open function
static int dev_open(struct inode *inodep, struct file *filep) {
    if (open_count == 0) {
        set_led_state(LED_R_PIN, 1);  // Turn on LED-R when driver is loaded
    }
    open_count++;
    printk(KERN_INFO "devUART0: Device has been opened %d time(s)\n", open_count);
    return 0;
}

// Device read function
static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset) {
    int error_count = copy_to_user(buffer, msg, msg_size);

    if (error_count == 0) {
        blink_led(LED_G_PIN, 10);  // Blink LED-G with 50% duty cycle on read
        printk(KERN_INFO "devUART0: Sent %d characters to user\n", msg_size);
        return (msg_size = 0);     // Clear the buffer
    } else {
        printk(KERN_ERR "devUART0: Failed to send %d characters\n", error_count);
        return -EFAULT;            // Return bad address error
    }
}

// Device write function
static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset) {
    copy_from_user(msg, buffer, len);
    msg_size = strlen(msg);
    blink_led(LED_B_PIN, 10);      // Blink LED-B with 50% duty cycle on write
    printk(KERN_INFO "devUART0: Received %zu characters from user\n", len);
    return len;
}

// Device release function
static int dev_release(struct inode *inodep, struct file *filep) {
    blink_led(LED_R_PIN | LED_G_PIN | LED_B_PIN, 3);  // Blink all LEDs on close
    printk(KERN_INFO "devUART0: Device successfully closed\n");
    return 0;
}


static int __init devUART0_init(void) {
  
    major_number = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_number < 0) {
        printk(KERN_ALERT "devUART0: Failed to register a major number\n");
        return major_number;
    }
    printk(KERN_INFO "devUART0: Registered with major number %d\n", major_number);

    
    gpio_request_one(LED_R_PIN, GPIOF_OUT_INIT_LOW, "LED_R");
    gpio_request_one(LED_G_PIN, GPIOF_OUT_INIT_LOW, "LED_G");
    gpio_request_one(LED_B_PIN, GPIOF_OUT_INIT_LOW, "LED_B");

    return 0;
}


static void __exit devUART0_exit(void) {
    
    set_led_state(LED_R_PIN, 0);
    set_led_state(LED_G_PIN, 0);
    set_led_state(LED_B_PIN, 0);
    gpio_free(LED_R_PIN);
    gpio_free(LED_G_PIN);
    gpio_free(LED_B_PIN);

  
    unregister_chrdev(major_number, DEVICE_NAME);
    printk(KERN_INFO "devUART0: Unregistered and cleaned up\n");
}

module_init(devUART0_init);
module_exit(devUART0_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sirisha");
MODULE_DESCRIPTION("UART Driver with RGB LED Integration");
MODULE_VERSION("1.0");
