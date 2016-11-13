//
//  Write a series of bytes to the EEPROM of the STM8S105C6 and then
//  verify that the data has been written correctly.
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

//
//  Define where we will be working in the EEPROM.
//
#define EEPROM_BASE_ADDRESS         0x4000
#define EEPROM_INITIAL_OFFSET       0x0040
#define EEPROM_DATA_START           (EEPROM_BASE_ADDRESS + EEPROM_INITIAL_OFFSET)

//
//  Data to write into the EEPROM.
//
unsigned int _pulseLength[] = { 2000U, 27830U, 400U, 1580U, 400U, 3580U, 400U };
unsigned char _onOrOff[] =    {   1,      0,     1,     0,    1,     0,    1 };
char numberOfValues = 7;

//--------------------------------------------------------------------------------
//
//  Write the default values into EEPROM.
//
void SetDefaultValues()
{
    //
    //  Check if the EEPROM is write-protected.  If it is then unlock the EEPROM.
    //
    if (FLASH_IAPSR_DUL == 0)
    {
        FLASH_DUKR = 0xae;
        FLASH_DUKR = 0x56;
    }
    //
    //  Write the data to the EEPROM.
    //
    char *address = (char *) EEPROM_DATA_START;     //  Data location in EEPROM.
    *address++ = (char) numberOfValues;
    for (int index = 0; index < numberOfValues; index++)
    {
        *address++ = (char) ((_pulseLength[index] >> 8) & 0xff);
        *address++ = (char) (_pulseLength[index] & 0xff);
        *address++ = _onOrOff[index];
    }
    //
    //  Now write protect the EEPROM.
    //
    FLASH_IAPSR_DUL = 0;
}

//--------------------------------------------------------------------------------
//
//  Verify that the data in the EEPROM is the same as the data we
//  wrote originally.
//
void VerifyEEPROMData()
{
    PD_ODR_ODR2 = 1;            //  Checking the data
    PD_ODR_ODR3 = 0;            //  No errors.
    //
    char *address = (char *) EEPROM_DATA_START;     //  Data location in EEPROM.
    if (*address++ != numberOfValues)
    {
        PD_ODR_ODR3 = 1;
    }
    else
    {
        for (int index = 0; index < numberOfValues; index++)
        {
            unsigned int value = (*address++ << 8);
            value += *address++;
            if (value != _pulseLength[index])
            {
                PD_ODR_ODR3 = 1;
            }
            if (*address++ != _onOrOff[index])
            {
                PD_ODR_ODR3 = 1;
            }
        }
    }
    PD_ODR_ODR2 = 0;        // Finished processing.
}

//--------------------------------------------------------------------------------
//
//  Setup port D for data output.
//
void SetupPorts()
{
    //
    //  Initialise Port D.
    //
    PD_ODR = 0;             //  All pins are turned off.
    PD_DDR = 0xff;          //  All bits are output.
    PD_CR1 = 0xff;          //  All pins are Push-Pull mode.
    PD_CR2 = 0xff;          //  Pins can run up to 10 MHz.
}

//--------------------------------------------------------------------------------
//
//  Main program loop.
//
void main()
{
    SetupPorts();
    SetDefaultValues();
    VerifyEEPROMData();
}