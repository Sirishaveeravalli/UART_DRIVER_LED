# UART_DRIVER_LED

Please find the following project.

Integration of LED functionalities with UART driver
Objective :

    Develop and design a character device driver for the UART-0 in RPI / beaglebone board
    Overall brief of the high level design.
        Create a UART-0 driver with name devUART0
        enable the following functionalities.
        
            1. open
            2. read
            2. write
            3. ioctl
            
        Integrate an RGB LED with the following functionalities.
            post loading the module, LED-R will turn ON.
            while reading the data, LED-G will blink with a duty cyle 50%
            while writing the data LED-B will blink with a duty cycle of 50%
            while removing the module, all the LEDs will blink with a duty cycle of 50% thrice. 
