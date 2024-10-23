#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define DEVICE_PATH "/dev/uart_rgb_led"

int main() {
    int fd;
    char read_buffer[256] = {0};
    const char *write_message = "Test UART write operation";

    // Open the device file
    fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("Failed to open the device...");
        return -1;
    }

    // Write to the device
    printf("Writing to the device...\n");
    write(fd, write_message, strlen(write_message));

    // Read from the device
    printf("Reading from the device...\n");
    read(fd, read_buffer, sizeof(read_buffer));
    printf("Read: %s\n", read_buffer);

    // Close the device
    close(fd);
    return 0;
}
