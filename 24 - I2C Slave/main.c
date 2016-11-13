//
//  This application demonstrates the principles behind developing an
//  I2C slave device on the STM8S microcontroller.  The application
//  will total the byte values written to it and then send the total
//  to the master when the device is read.
//
//  This software is provided under the CC BY-SA 3.0 licence.  A
//  copy of this licence can be found at:
//
//  http://creativecommons.org/licenses/by-sa/3.0/legalcode
//
#if defined DISCOVERY
    #include <iostm8S105c6.h>
#else
    #include <iostm8s103f3.h>
#endif
#include <intrinsics.h>

//
//  Define some pins to output diagnostic data.
//
#define PIN_BIT_BANG_DATA       PD_ODR_ODR4
#define PIN_BIT_BANG_CLOCK      PD_ODR_ODR5
#define PIN_ERROR               PD_ODR_ODR6

//
//  I2C device related constants.
//
#define DEVICE_ADDRESS          0x48
#define I2C_READ                1
#define I2C_WRITE               0

//
//  State machine for the I2C communications.
//
typedef enum
{
    ISWaiting,                    //  Waiing for comms to start.
    ISAdding,                     //  Adding bytes of data.
    ISSendingMSB,                 //  Sending MSB of total.
    ISSendingLSB                  //  Sending LSB of total.
} I2CStateType;
I2CStateType _i2cState;

//
//  Somewhere to hold the sum.
//
int _total;

//
//  Bit bang data on the diagnostic pins.
//
void BitBang(unsigned char byte)
{
    for (short bit = 7; bit >= 0; bit--)
    {
        if (byte & (1 << bit))
        {
            PIN_BIT_BANG_DATA = 1;
        }
        else
        {
            PIN_BIT_BANG_DATA = 0;
        }
        PIN_BIT_BANG_CLOCK = 1;
        __no_operation();
        PIN_BIT_BANG_CLOCK = 0;
    }
    PIN_BIT_BANG_DATA = 0;
}

//
//  Set up the system clock to run at 16MHz using the internal oscillator.
//
void InitialiseSystemClock()
{
    CLK_ICKR = 0;                       //  Reset the Internal Clock Register.
    CLK_ICKR_HSIEN = 1;                 //  Enable the HSI.
    CLK_ECKR = 0;                       //  Disable the external clock.
    while (CLK_ICKR_HSIRDY == 0);       //  Wait for the HSI to be ready for use.
    CLK_CKDIVR = 0;                     //  Ensure the clocks are running at full speed.
    CLK_PCKENR1 = 0xff;                 //  Enable all peripheral clocks.
    CLK_PCKENR2 = 0xff;                 //  Ditto.
    CLK_CCOR = 0;                       //  Turn off CCO.
    CLK_HSITRIMR = 0;                   //  Turn off any HSIU trimming.
    CLK_SWIMCCR = 0;                    //  Set SWIM to run at clock / 2.
    CLK_SWR = 0xe1;                     //  Use HSI as the clock source.
    CLK_SWCR = 0;                       //  Reset the clock switch control register.
    CLK_SWCR_SWEN = 1;                  //  Enable switching.
    while (CLK_SWCR_SWBSY != 0);        //  Pause while the clock switch is busy.
}

//
//  Set up Port D GPIO for diagnostics.
//
void InitialisePortD()
{
    PD_ODR = 0;             //  All pins are turned off.
    PD_DDR_DDR4 = 1;        //  Port D, bit 4 is output.
    PD_CR1_C14 = 1;         //  Pin is set to Push-Pull mode.
    PD_CR2_C24 = 1;         //  Pin can run up to 10 MHz.
    //
    PD_DDR_DDR5 = 1;        //  Port D, bit 5 is output.
    PD_CR1_C15 = 1;         //  Pin is set to Push-Pull mode.
    PD_CR2_C25 = 1;         //  Pin can run up to 10 MHz.
    //
    PD_DDR_DDR6 = 1;        //  Port D, bit 6 is output.
    PD_CR1_C16 = 1;         //  Pin is set to Push-Pull mode.
    PD_CR2_C26 = 1;         //  Pin can run up to 10 MHz.
}

//
//  Initialise the I2C system.
//
void InitialiseI2C()
{
    I2C_CR1_PE = 0;                     //  Disable I2C before configuration starts.
    //
    //  Set up the clock information.
    //
//    I2C_FREQR = 16;                     //  Set the internal clock frequency (MHz).
//    I2C_CCRH_F_S = 0;                   //  I2C running is standard mode.
//    I2C_CCRL = 0xa0;                    //  SCL clock speed is 50 KHz.
//    I2C_CCRH_CCR = 0x00;
    //
    //  Set the address of this device.
    //
    I2C_OARH_ADDMODE = 0;               //  7 bit address mode.
    I2C_OARH_ADD = 0;                   //  Set this device address to be 0x50.
    I2C_OARL_ADD = 0x48;
    I2C_OARH_ADDCONF = 1;               //  Docs say this must always be 1.
    //
    //  Setup the bus characteristics.
    //
    I2C_TRISER = 17;
    //
    //  Turn on the interrupts.
    //
    I2C_ITR_ITBUFEN = 1;                //  Buffer interrupt enabled.
    I2C_ITR_ITEVTEN = 1;                //  Event interrupt enabled.
    I2C_ITR_ITERREN = 1;                //  Error interrupt enabled.
    //
    //  Configuration complete so turn the peripheral on.
    //
    I2C_CR1_PE = 1;
    //
    //  Enter master mode.
    //
    I2C_CR2_ACK = 1;
}

//
//  I2C interrupts all share the same handler.
//
#pragma vector = I2C_RXNE_vector
__interrupt void I2C_IRQHandler()
{
    unsigned char reg;

    if (I2C_SR1_ADDR)
    {
        //
        //  Slave address received, work out if to expect a read or a write.
        //
        if (_i2cState == ISWaiting)
        {
            _i2cState = ISAdding;
            _total = 0;             // New addition so clear the total.
        }
        reg = I2C_SR1;
        reg = I2C_SR3;
        return;
    }
    if (I2C_SR1_RXNE)
    {
        //
        //  Receiving data from the master so we must be adding.
        //
        _total += I2C_DR;
        _i2cState = ISAdding;
        return;
    }
    if (I2C_SR1_TXE)
    {
        if (_i2cState == ISAdding)
        {
            I2C_DR = (_total >> 8) & 0xff;
            _i2cState = ISSendingMSB;
        }
        else
        {
            I2C_DR = _total & 0xff;
            _i2cState = ISSendingLSB;
        }
        return;
    }
    if (I2C_SR2_AF)
    {
        I2C_SR2_AF = 0;             //  End of slave transmission.
        _i2cState = ISWaiting;
        return;
    }
    //
    //  Send a diagnostic signal to indicate we have cleared
    //  the error condition.
    //
    PIN_ERROR = 1;
    __no_operation();
    PIN_ERROR = 0;
    //
    //  If we get here then we have an error so clear
    //  the error, output the status registers and continue.
    //
    reg = I2C_SR1;
    BitBang(reg);
    BitBang(I2C_SR2);
    reg = I2C_SR3;
    BitBang(reg);
}

//
//  Main program loop.
//
int main()
{
    _total = 0;
    _i2cState = ISWaiting;
    __disable_interrupt();
    InitialisePortD();
    InitialiseSystemClock();
    InitialiseI2C();
    __enable_interrupt();
    while (1)
    {
        __wait_for_interrupt();
    }
}
