# The Way of the Register

*The Way of the Register* is a series of posts looking at the STM8S series of processors. Each article in the series takes a distinct task (or group of related tasks) and describes how to achieve the task using the STM8S. Where possible, full project and source code for the IAR environment is included.

## STM8S

All of these examples use the STM8S103F3 chip.  If you are using another chip (say the STM8S Discovery Board) then you will also want to have a look at the post on [Converting The Way of The Register Examples](http://blog.mark-stevens.co.uk/?p=664) in order to work out what you will need to do to convert these examples.

### [Converting STD Library Code to use Direct Register Access on the STM8S](http://blog.mark-stevens.co.uk/?p=526)

Going from using the STD Peripheral Library on the ST processors can be a little bewildering.  This article attempt to guide you through the process of converting you code from using the STD Peripheral library to using registers.

### [Simple GPIO on the STM8S](http://blog.mark-stevens.co.uk/?p=533)

Toggle a single GPIO line to produce an approximate square wave signal.

### [Configuring the System Clock on the STM8S](http://blog.mark-stevens.co.uk/?p=568)

Configure the system clock to use the internal HSI oscillator running at 16MHz.  Generate a square wave signal using a GPIO line and show how the frequency of the square wave can be changed by manipulating the clock divider.

### [External Interrupts on the STM8S](http://blog.mark-stevens.co.uk/?p=601)

Capturing user input through a switch using the STM8S.

### [Using the UART on the STM8S Microcontroller](http://blog.mark-stevens.co.uk/?p=625)

Learn how to setup and use the serial port on the STM8S to generate debug information or control serial devices.

### [Generating a Regular Pulse Using Timer 2](http://blog.mark-stevens.co.uk/?p=635)

Here we will learn how to use one of the timers on the STM8S to generate a square wave signal.

### [Generating a PWM Signal on the STM8S](http://blog.mark-stevens.co.uk/?p=645)

In this article we look at generating a PWM signal using channel 1 of Timer 2 on the STM8S.

### [Interrupts on the STM8S](http://blog.mark-stevens.co.uk/?p=682)

An overview of how interrupts work on the STM8S including setting priorities for the various interrupts and how to tell the compiler that a method is an ISR and which vector it relates to.

### [Single Pulse Generation with the STM8S](http://blog.mark-stevens.co.uk/?p=699)

This post examines two methods for generating a single pulse of a defined width.

### [Timer 1 Counting Modes](http://blog.mark-stevens.co.uk/?p=715)

Looking at using the up and down counting modes of the STM8S.  We also look at how we can change the overflow interrupt frequency using the Repetition counter.

### [Single conversion ADC on the STM8S](http://blog.mark-stevens.co.uk/?p=725)

Using a potentiometer and a simple LED circuit we will learn how to read an analog signal using the ADC on the STM8S.

### [STM8S SPI Slave](http://blog.mark-stevens.co.uk/?p=791)

Enabling single byte communication between a Netduino Plus (acting as SPI master) and a STM8S (acting as a SPI slave).

### [STM8S SPI Slave (Part 2)](http://blog.mark-stevens.co.uk/?p=800)

Enabling transmission of a buffer of data between the Netduino Plus and the STM8S.  This posts also shows how to use software slave chip select.

### [STM8S SPL Slave (Part 3) - Making a Netduino Go! Module](http://blog.mark-stevens.co.uk/?p=812)

Building on the previous two posts, this post looks at what is necessary to build a Netduino Go module using the STM8S microcontroller.

### [Transmitting Data Using the STM8S SPI Master Mode](http://blog.mark-stevens.co.uk/?p=976)

Using the master mode of the SPI interface on the STM8S to control the brightness of 16 LEDs connected to the TLC5940 16-channel PWM controller IC.

### [Storing data in the data EEPROM area of the STM8S](http://blog.mark-stevens.co.uk/2013/09/storing-data-eeprom-stm8s/)

Writing a small amount of data in the EEPROM area of the STM8S in order that the data can survive a device reset or loss of power.

### [Using the Auto-Wakeup on the STM8S Discovery Board](http://blog.mark-stevens.co.uk/2014/06/auto-wakeup-stm8s/)

Using the Auto-Wakeup function on the STM8S Discovery board to put the microcontroller to sleep and the wakeup after a predefined time period.

### [STM8S Independent Watchdog](http://blog.mark-stevens.co.uk/2014/06/stm8s-independent-watchdog/)

Adding a watchdog to your code in case of software failure.

### [STM8S Beep Function](http://blog.mark-stevens.co.uk/2014/07/stm8s-beep-function/)

Using the beep function on the STM8S to generate a signal ranging from 500Hz to 32KHz.

### [STM8S Window Watchdog](http://blog.mark-stevens.co.uk/2014/07/window-watchdog/)

Using the Window Watchdog function to detect software failures on the STM8S.

### [STM8S as an I2C bus master](http://blog.mark-stevens.co.uk/2015/05/stm8s-i2c-master-devices/)

Create an I2C master device capable of retrieving temperature data from the TMP102 temperature sensor.

### [STM8S as an I2C slave device](http://blog.mark-stevens.co.uk/2015/05/stm8s-i2c-slave-device/)

Creating a simple I2C slave device using the STM8S micrcontroller.
