#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/ioctl.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <asm/io.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("UART-0 Character Driver with RGB LED Control");

#define MYDEV_NAME        "devUART0"
#define MYDEV_MAJOR       42
#define SER_IOBASE        0x3f8    // I/O base for UART-0
#define SER_IRQ           4        // IRQ for UART-0

// Define GPIO pins for RGB LED
#define LED_R_PIN         16
#define LED_G_PIN         20
#define LED_B_PIN         21

// ioctl command definitions
#define IOCTL_SET_BAUDRATE _IOW(MYDEV_MAJOR, 0, int)
#define IOCTL_GET_BAUDRATE _IOR(MYDEV_MAJOR, 1, int)
#define IOCTL_RESET_BUFFER _IO(MYDEV_MAJOR, 2)

// Circular buffer structure
#define MAXSIZE           4000
typedef struct CircularBuf {
    int RdIndex;
    int WrIndex;
    int NoOfChar;
    char Buf[MAXSIZE];
} Cirq_t;

typedef struct {
    struct semaphore sem;
    wait_queue_head_t rxWq;
    wait_queue_head_t txWq;
    unsigned int baseAddr;
    int irq;
    int baudrate;
    Cirq_t Txq;
    Cirq_t Rxq;
} Cblock_t;

static Cblock_t Cblkp;

static int myserial_open(struct inode *inode, struct file *file);
static int myserial_close(struct inode *inode, struct file *file);
static ssize_t myserial_read(struct file *file, char *buf, size_t count, loff_t *offset);
static ssize_t myserial_write(struct file *file, const char *buf, size_t count, loff_t *offset);
static long myserial_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
static irqreturn_t myIntHandler(int irq, void *dev_id);
static void set_led(int gpio, int state);
static void blink_led(int gpio, int duration_ms);

// File operations structure
static struct file_operations myserial_ops = {
    .owner = THIS_MODULE,
    .open = myserial_open,
    .release = myserial_close,
    .read = myserial_read,
    .write = myserial_write,
    .unlocked_ioctl = myserial_ioctl
};

// LED helper functions
static void set_led(int gpio, int state) {
    gpio_set_value(gpio, state);
}

static void blink_led(int gpio, int duration_ms) {
    set_led(gpio, 1);
    msleep(duration_ms / 2);
    set_led(gpio, 0);
    msleep(duration_ms / 2);
}

// Interrupt handler for UART-0
static irqreturn_t myIntHandler(int irq, void *dev_id) {
    unsigned char intStat = inb(Cblkp.baseAddr + 2); // Interrupt Identification Register
    while ((intStat & 1) == 0) {
        if ((intStat & 0x2) == 0x2) // Transmitter interrupt
            printk(KERN_INFO "Transmit Interrupt\n");
        if ((intStat & 0x4) == 0x4) // Receiver interrupt
            printk(KERN_INFO "Receive Interrupt\n");
        intStat = inb(Cblkp.baseAddr + 2);
    }
    return IRQ_HANDLED;
}

// Device open function
static int myserial_open(struct inode *inode, struct file *file) {
    printk(KERN_INFO "UART device opened\n");
    set_led(LED_R_PIN, 1); // Turn on LED-R upon opening
    return 0;
}

// Device close function
static int myserial_close(struct inode *inode, struct file *file) {
    printk(KERN_INFO "UART device closed\n");
    return 0;
}

// Device read function
static ssize_t myserial_read(struct file *file, char *buf, size_t count, loff_t *offset) {
    printk(KERN_INFO "UART device read\n");
    blink_led(LED_G_PIN, 500); // Blink LED-G with 50% duty cycle during read
    return 0;
}

// Device write function
static ssize_t myserial_write(struct file *file, const char *buf, size_t count, loff_t *offset) {
    printk(KERN_INFO "UART device write\n");
    blink_led(LED_B_PIN, 500); // Blink LED-B with 50% duty cycle during write
    return count;
}

// ioctl function for UART device
static long myserial_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    int temp;
    switch (cmd) {
        case IOCTL_SET_BAUDRATE:
            if (copy_from_user(&temp, (int __user *)arg, sizeof(temp)))
                return -EFAULT;
            Cblkp.baudrate = temp;
            printk(KERN_INFO "Baud rate set to %d\n", Cblkp.baudrate);
            break;
        case IOCTL_GET_BAUDRATE:
            if (copy_to_user((int __user *)arg, &Cblkp.baudrate, sizeof(Cblkp.baudrate)))
                return -EFAULT;
            printk(KERN_INFO "Baud rate fetched: %d\n", Cblkp.baudrate);
            break;
        case IOCTL_RESET_BUFFER:
            Cblkp.Txq.RdIndex = 0;
            Cblkp.Txq.WrIndex = 0;
            Cblkp.Txq.NoOfChar = 0;
            Cblkp.Rxq.RdIndex = 0;
            Cblkp.Rxq.WrIndex = 0;
            Cblkp.Rxq.NoOfChar = 0;
            printk(KERN_INFO "Buffers reset\n");
            break;
        default:
            return -EINVAL;
    }
    return 0;
}

// Module init function
static int __init myserial_init(void) {
    int res;
    printk(KERN_INFO "Initializing UART-0 driver\n");

    // Register character device
    res = register_chrdev(MYDEV_MAJOR, MYDEV_NAME, &myserial_ops);
    if (res < 0) {
        printk(KERN_ERR "Registration error: %d\n", res);
        return res;
    }

    // Request GPIOs and set LED-R initially
    gpio_request_one(LED_R_PIN, GPIOF_OUT_INIT_LOW, "LED_R");
    gpio_request_one(LED_G_PIN, GPIOF_OUT_INIT_LOW, "LED_G");
    gpio_request_one(LED_B_PIN, GPIOF_OUT_INIT_LOW, "LED_B");
    set_led(LED_R_PIN, 1); // LED-R on after loading module

    // Request IRQ for UART-0
    if (request_irq(SER_IRQ, myIntHandler, 0, MYDEV_NAME, NULL)) {
        printk(KERN_ERR "Failed to get IRQ %d\n", SER_IRQ);
        unregister_chrdev(MYDEV_MAJOR, MYDEV_NAME);
        return -EIO;
    }
    return 0;
}

// Module exit function
static void __exit myserial_exit(void) {
    int i;
    printk(KERN_INFO "Exiting UART-0 driver\n");

    // Blink all LEDs thrice with 50% duty cycle on exit
    for (i = 0; i < 3; i++) {
        blink_led(LED_R_PIN, 500);
        blink_led(LED_G_PIN, 500);
        blink_led(LED_B_PIN, 500);
    }

    free_irq(SER_IRQ, NULL);
    gpio_free(LED_R_PIN);
    gpio_free(LED_G_PIN);
    gpio_free(LED_B_PIN);
    unregister_chrdev(MYDEV_MAJOR, MYDEV_NAME);
}

module_init(myserial_init);
module_exit(myserial_exit);
