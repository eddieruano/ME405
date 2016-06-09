//*****************************************************************************
/** @file i2c_driver.cpp
 *  @brief     Driver for I2C Protocol, or Two Wire Interface.
 *
 *  @details   Based on JR Ridgely's methodology, incorporated with
 *             previous code already established. It is more tailored for
 *             specific use with our BNO055 IMU and establishes a more
 *             structured approach to performing the protocol. Furthermore,
 *             this driver cuts down on extra functions by creating 2
 *             universal fucntions for reading and writing instead of having
 *             seperate ones for different sizes of payloads.
 *
 *  @author Eddie Ruano
 *
 *  Revisions:
        @ 5/30/2016 <<EDD>> added comments, updated license.
        @ 5/28/2016 <<EDD>> fixed everything and made everything more
 *       robust and fixed a much of calls. Made it so that it's easier
 *       to use function.

 *  License:
 *   BNO055 driver class for the ATMEGA1281 C
 *   Copyright (C) 2016  Eddie Ruano
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
*/
//*****************************************************************************
#include "FreeRTOS.h"                       // Main header for FreeRTOS
#include "task.h"                           // Needed for the vTaskDelay() function
#include "i2c_driver.h"                     // Header for this class
#include <util/delay.h>
// #define F_CPU 16000000UL  // 16 MHz
// Define the port where SDA and SCL are located
#define I2C_PORT        PORTD
/// Define the location of the SDA pin
#define SDA_PIN         PD1
/// Define the location of the SCL pin
#define SCL_PIN         PD0
// Define the STOP keyword to make code more legible
#define STOP            false
/// Define the CONTINUE keyword to make code more legible
#define CONTINUE        true
/// User defined write operation constant, usually (DEVICE_ADDR << 1)
#define DEVICE_WRITE    0x50
/// User defined read operation constant, usually (DEVICE_ADDR << 1) + 1
#define DEVICE_READ     0x51
/// Mask used to read correct bits from status register
#define MASK_STATUS     0xF8
/// User can define a value for timeout here
#define TIMEOUT         300000
// #define DELAY           300000
/// Sequence to return mutex and return a false when failure occurs.
#define RET_FAILURE     { xSemaphoreGive (mutex); stop(); return false; }
/**
 * @brief       Sets up a driver for I2C interfacing protocol, connects the
 *              pointers for the emstreams and creates mutex to protect SDA
 *              bus from multiple calls. Also calls intitialize() which sets
 *              up further parameters.
 *
 * @param       p_debug_port  Pointer to the serial output for debugging
 *              purposes
 *
 */
i2c_driver::i2c_driver (emstream* p_ser_port)
{
    // Set the debugging serial port pointer
    p_serial = p_ser_port;
    // Create the mutex which will protect the I2C bus from multiple calls
    if ((mutex = xSemaphoreCreateMutex ()) == NULL)
    {
        *p_serial << "Error: No I2C mutex" << endl;
    }

    initialize();
}

/**
 * @brief       Inititalize Pullups, Power Redux, Clock Prescalers, & Bitrates
 *
 * @details     In order for us to get i2c comms going, we need to initialize
 *              a couple of registers. First we set the pull up resistors on
 *              PORTD, PINS[0,1] although it's not completely necessary since
 *              the ATmega1281 chip onboard controller for Two Wire Interface
 *              comms takes control of these lines, however for consistency
 *              and protection it's in the code. We also activate the two wire
 *              comms in the power reduction register and set no prescaler on
 *              the clock. Since no prescaler is declared the bitrate in TWBR
 *              can be low if the clock is high since we are trying to stay
 *              under 400,000 HZ total freq.
 *
 */
void i2c_driver::initialize(void)
{
    /****** Begin initialization of I2C communication ******/
    //Enable Internal Pull up on SDA and SCL pins [ATmega1281 PORTD PIN1 & PIN0]
    PORTD |= (1 << SDA_PIN) | (1 << SCL_PIN);

    //Power Reduction Register 0 PPR0 (0x64)
    //PPR0 -PRTWI | PRTIM2 | PRTIM0 |   -  | PRTIM1 | PRSPI | PRUSART0 | PRADC
    //PPR0 ---7---|---6----|---5----|---4--|---3----|---2---|---1------|---0--
    //PRTWI - (---Power Reduction Two Wire Input---)
    //Normally set to 1, we need to set it to 0 to enable 2 wire comms

    PRR0 &= ~(1 << PRTWI);

    //  For the next two registers consider this formula:
    //  SCL Freq = (CPU Internal Clock Frequency) / 16 + 2 (TWBR) * 4 ^ TWPS
    //  It sets the SCL

    //  TWSR (TWI Status Register (Prescaler Bits)) -
    //  TWSR - TWS7 | TWS6 | TWS5 | TWS4 | TWS3 | –---- | TWPS1 | TWPS0
    //  TWSR - --7--|---6--|---5--|---4--|---3--|---2---|---1---|---0-----
    //  TWSR - (----------Status Bits-----------)-------(-Prescaler Bits-)
    //  TWSR will be 0x08 after a start condition
    //  The SCL clock should be 100kHz
    //No prescaler so clean bottom two bits
    TWSR &= ~((1 << TWPS1) | (1 << TWPS0));

    //  TWBR (Bit Rate Generator Unit)
    //  TWBR (TWI Bit Rate Register) - will be 2 for a 50kHz SCL
    //  TWBR - TWBR7 | TWBR6 | TWBR5 | TWBR4 | TWBR3 | TWBR2 | TWBR1 | TWBR0
    //  TWBR - --7---|---6---|---5---|---4---|---3---|---2---|---1---|---0----
    //  Set bitrate to either 12 to make SCL got to 400kHz which is max
    //  OR set to 72 and get 100kHz on SCL
    TWBR = 12;

    // Make sure we're still clear doe
    TWCR &= ~(1 << TWIE);

    *p_serial << PMS("Initialized I2C: ") << endl;
}

/**
 * @brief       Waits for TWINT flag to be sent, timeouts if it takes too long.
 *
 * @details     This waits for the TWINT flag in the Two Wire Control Register
 *              [TWCR] to be set, which indicates that a response was received.
 *              The function doesn't return anything, however will print out a
 *              message saying it timed out if the global counter supasses the
 *              TIMEOUT limit. The way it delays is by looping through a bunch
 *              of conditions.
 *
 * @param       address is an 8bit address value of the current device being
 *              queried. Mostly for debugging as it is not needed to check for
 *              TWINT flag.
 *
 */
void i2c_driver::waitForSet(void)
{
    // restart the timer at 0 to avoid overflow issues
    resetTimer();

    // testing whether flag has been set yet, & that we are still below TIMEOUT
    while (!(TWCR & (1 << TWINT)))
    {
        // increase timer 
        local_timer++;
        
        // check if valid
        if (local_timer > TIMEOUT)
        {
            *p_serial << "Timeout Triggered by Slave Address: "
                      << hex << local_current_address << endl;
            break;
        }
    }
}

/**
 * @brief       This checks to see if the feedback received by the slave
 *              device is comformant to that of prefined values included in
 *              <util/twi.h>
 *
 * @details     Using the predefined mask given by MASK_STATUS, we extract
 *              only the necessary bits from the 8bit status register. If the
 *              response we receive does not match the expected value, then
 *              the check fails &a FALSE is returned upstream from this callee.
 *
 * @param       expected  The value that we expect to be written in the upper
 *              5 bits of the Status or [TWSR] register.
 *
 * @return      bool Returns the logical truth of the comparison of the status
 *              register and the expected value;
 */
bool i2c_driver::checkError(uint8_t expected)
{
    // Check if the response sent by slave is what is expected
    // UPDATE: This function is too fast, making it slower by creating var
    _delay_ms(1);
    if((TWSR & MASK_STATUS) == expected)
    {
        return true;
    }
   return false;
}

/**
 * @brief       Function to start the i2c start condition.
 *
 * @details     This function begins the i2c protocol by causing a start
 *              condition on the SDA line. if the fucntion returns within the
 *              timeout limit and the correct acknowledgement is given by the
 *              slave then it passes and returns a true.
 *
 * @return      returns TRUE if the start happens within a resonable of time
 *              and FALSE if it fails.
 */
bool i2c_driver::start (void)
{
    //TWCR (Two Wire Control Register)  (0xBC)
    //TWCR (Two Wire Control Register)
    //TWCR - TWINT | TWEA  | TWSTA | TWSTO | TWWC  | TWEN  | TWIE  |   -
    //TWCR - --7---|---6---|---5---|---4---|---3---|---2---|---1---|---0----
    //Set the Enable TWEN
    //Set the Flag TWINT
    //Set the Start Bit
    TWCR = (1 << TWEN) | (1 << TWINT) | (1 << TWSTA);
    // here we wait for the flag to be set
    waitForSet();
    // if we obtained a response we make sure it's the correct one
    if (!checkError(TW_START)) { return false; }
    // return true if we get this far
    return true;
}

/**
 * @brief       Function to cause the i2c interface to be terminated by master
 *              in control on the SDA line.
 *
 * @details     This function places a set bit on the TWSTO pin of the Two
 *              Wire Control Register [TWCR] thereby terminating the comms w/
 *              the slave.
 *
 * @return      returns TRUE always since there are no more 'acks' from slave
 *              after termination.
 */
bool i2c_driver::stop(void)
{
    //TWCR (Two Wire Control Register)  (0xBC)
    //TWCR (Two Wire Control Register)
    //TWCR - TWINT | TWEA  | TWSTA | TWSTO | TWWC  | TWEN  | TWIE  |   -
    //TWCR - --7---|---6---|---5---|---4---|---3---|---2---|---1---|---0----
    //Set the Flag TWINT
    //Set the Enable TWEN
    //But most importantly, Set TWSTO which tells the line that master wishes to stop comms.
    TWCR = ((1 << TWINT) | (1 << TWSTO) | (1 << TWEN));
    return true;
}

/**
 * @brief       Enables the [TWINT] and [TWEN] pins and sometimes enables the
 *              [TWEA] pin depending on the given directive variable.
 *
 * @param       directive This boolean value lets us know whether we wish to
 *              keep reading from line [TRUE/CONTINUE] or [STOP/STOP] and send
 *              an Acknowledge bit.
 */

void i2c_driver::clearEnable(bool directive)
{
    // Check to see if we wish to stop and send a response to slave or just want to keep going.
    // First condition happens when we need to send back an acknowledge.
    if (directive == STOP)
    {
        //TWCR (Two Wire Control Register)  (0xBC)
        //TWCR (Two Wire Control Register)
        //TWCR - TWINT | TWEA  | TWSTA | TWSTO | TWWC  | TWEN  | TWIE  |   -
        //TWCR - --7---|---6---|---5---|---4---|---3---|---2---|---1---|---0----
        //Set the Flag TWINT
        //Set the Enable TWEN
        TWCR = ((1 << TWINT) | (1 << TWEN));
        return;
    }
    else if(directive == CONTINUE)// This condition happens when we want to continue reading from [SDA]
    {
        //Set the Enable TWEA also
        TWCR = ((1 << TWINT) | (1 << TWEN) | (1 << TWEA));
        return;
    }
}

/**
 * @brief       Restarts the i2c process necessary during I2C/Read protocol.
 *
 * @details     Places the TWEN and TWINT and TWSTA in the Two Wire Control
 *              Register [TWCR], it differs from start() because it checks for
 *              different acknowledgement value from slave.
 *
 * @return      Returns TRUE if restart occurs successfully, FALSE otherwise.
 *
 */
bool i2c_driver::restart (void)
{
    //TWCR (Two Wire Control Register)  (0xBC)
    //TWCR (Two Wire Control Register)
    //TWCR - TWINT | TWEA  | TWSTA | TWSTO | TWWC  | TWEN  | TWIE  |   -
    //TWCR - --7---|---6---|---5---|---4---|---3---|---2---|---1---|---0----
    //Set the Flag TWINT
    //Set the Enable TWEN
    //But most importantly, Set TWSTO which tells the line that master wishes to stop comms.
    // Restart by setting correct registers
    TWCR = (1 << TWEN) | (1 << TWINT) | (1 << TWSTA);
    // Return a fail if we can't get the process to restart
    waitForSet();
    // Check of restart ack (0x10)
    if (!checkError(TW_REP_START)) {return false;}
    // Success
    return true;
}


/**
 * @brief       Writes a payload of variable size to a device and target
 *              register using i2c protocol.
 *
 * @details     Given a device address, target register, array of data, and
 *              length value, this function writes the data to the specified
 *              target using i2c protocol. It first takes the Mutex so no
 *              interfereence on the SDA line occurs while it is running the
 *              protocol. It saves the device address so that meaningful
 *              debuggning info can be relayed back to the caller. When it
 *              completes, it returns the Mutex and returns a boolean TRUE,
 *              otherwise FALSE.
 *
 * @param       device  8bit Address of device we wish to establish comms with
 *
 * @param       target  8bit Address of register we are reading on the device,
 *                      it can also be the starting register of a multiple
 *                      byte read.
 *
 * @param       data    Pointer to where we are storing the data read during
 *                      the operation. It is assumed that the caller will not
 *                      request more data than they are able to hold in this.
 *
 * @param       length  Number of bytes the caller wishes to read from the
 *                      device, if a simple read operation is needed, this
 *                      value should be 1. MAX bytes allowed to read is 255.
 *
 * @return      Returns TRUE if the operation is successful, FALSE otherwise
 */
bool i2c_driver::readData(uint8_t device, uint8_t target, uint8_t * data, uint8_t length)
{
    // Update the local address holder, so we have a way to relay debugging information back to the caller
    local_current_address = device;

    // initialize the loop counter
    uint8_t count;

    //Takes the mutex, or waits for it based on portMAX_DELAY value established in Semph.h

    xSemaphoreTake (mutex, portMAX_DELAY);

    /*Step 1*/
    // Start the protocol by master, first possible point of failure
    if (!start()) RET_FAILURE

    /*Step 2*/
    // Now we send Slave Address and wait for it to ack our request.
    TWDR = DEVICE_WRITE;
    clearEnable(STOP);
    waitForSet();
    // Check if 0x18 is returned by Slave
    if (!checkError(TW_MT_SLA_ACK)) RET_FAILURE

    /*Step 3*/
    //Send Register we want to access
    TWDR = target;
    clearEnable(STOP);
    waitForSet();
    // Check if it data request ack (0x28)
    if (!checkError(TW_MT_DATA_ACK)) RET_FAILURE

    /*Step 4*/
    // Repeat Start as per protocol requires
    restart();

    /*Step 5*/
    // Send Slave address of reg w/ READ command
    TWDR = DEVICE_READ;
    clearEnable(STOP);
    waitForSet();
    // Check if the read command is acknowledged, (0x40)
    if (!checkError(TW_MR_SLA_ACK)) RET_FAILURE

    /*Step 6*/
    // If we get to this point, we know that the next byte of data sent by the device will be data
    clearEnable(CONTINUE);
    waitForSet();
    // Check if we have data waiting (0x48)
    if (!checkError(TW_MR_DATA_ACK)) RET_FAILURE

    /*Step 7*/
    // Here we set up a loop that will repeat this process for however many number of bytes required.
    for (count = 0; count < length; count++)
    {
        data[count] = TWDR;
        clearEnable(CONTINUE);
        waitForSet();
        if (!checkError(TW_MR_DATA_ACK)) RET_FAILURE
    }

    /*Step 8*/
    // Master sends a nack to stop the flow of data.
    clearEnable(STOP);
    waitForSet();
    // Fail if the slave doesn't want to stop
    if (!checkError(TW_MR_DATA_NACK)) RET_FAILURE

    /*Step 9*/
    // Stop I2C Protocol
    stop();
    xSemaphoreGive (mutex); 

    return true;
}

/**
 * @brief       Writes a byte of data to a target register on a device on the
 *              slave line using i2c protocol
 *
 * @details     This function uses i2c protocol in order to send a byte of
 *              information to a device address specified by the caller. It
 *              takes Mutex so that the caller has sole control on the SDA
 *              line and no other task interferes, returns the mutex when
 *              finished.
 *
 * @param       device  8bit address of the device we wish to send data to
 *
 * @param       target  8bit address of the register we wish to target/send to
 *
 * @param       data    8bit payload that will be delivered to target register
 *                      of device specified
 *
 * @return      returns FALSE if the protocol fails, TRUE otherwise
 */

bool i2c_driver::writeData(uint8_t device, uint8_t target, uint8_t data)
{
    // Set local address to this most recently called address, for debugging purposes
    local_current_address = device;

    // establish command needed to write to device
    uint8_t write_command = DEVICE_WRITE;

    // Take Mutex in order to prevent unauthorized use of I2C bus
    xSemaphoreTake (mutex, portMAX_DELAY);

    /* Step 1 */
    // Start I2C protocol
    if (!start()) RET_FAILURE
    
    /* Step 2 */
    // Send slave device command to tell it that a write is requested
    TWDR = DEVICE_WRITE;
    clearEnable(STOP);
    waitForSet();
    // Fail if slave response is not ACK (0x18)
    if (!checkError(TW_MT_SLA_ACK)) RET_FAILURE

        /* Step 3 */
        // Send the slave device a target register
        TWDR = target;
    clearEnable(STOP);
    waitForSet();
    // Fail if the slave doesn't acknowledge this data req (0x28)
    if (!checkError(TW_MT_DATA_ACK)) RET_FAILURE;

    /* Step 4 */
    // Send byte we want written to the target register
    TWDR = data;
    clearEnable(STOP);
    waitForSet();
    // Fail if the slave does not send NACK
    //*p_serial << "STATUS: " << hex << TWSR << endl;
    if (!checkError(0x28)) RET_FAILURE

    /* Step 5 */
    // End I2C communication
    stop();
    xSemaphoreGive (mutex); 

    return true;
}

/**
 * @brief       Pings the address given, and returns true if the address
 *              replies with data.
 *
 * @param       address  8bit value of address we want to ping on the SDA line
 *
 * @return      Returns true of response happens successfully, FALSE otherwise.
 */

bool i2c_driver::ping(uint8_t address)
{
    // Creates an empty 1 entry array to hold response.
    uint8_t response[1];
    // Start the ping at the 0x00 address which is default Read-Only on many devices
    if (readData(address, 0x00, response, 1))
    {
        // Print Response if Success and Return TRUE
        *p_serial << "Ping Response: " << response[0] <<  endl;
        return true;
    }
    return false; // Return FALSE otherwise.
}

/**
 * @brief       This function resets the local timer that counts up during a
 *              waiting period.
 */
void i2c_driver::resetTimer(void)
{
    local_timer = 0;
}
