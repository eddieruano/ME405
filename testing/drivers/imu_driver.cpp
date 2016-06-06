//*****************************************************************************
/** @file imu_driver.cpp
 *  @brief     This class is the imu driver class.
 *
 *  @details   This class creates an instance of a imu assuming caller gives
 *             all the correct and legal addresses and pin values, driver is
 *             able to communicate with IMU via I2C.
 *
 *  @author Eddie Ruano
 *
 *  Revisions: @ 5/4/2016 <<EDD>> created.
 *  License:
 *    This file is copyright 2016 by Eddie Ruano and released under the Lesser
 *    GNU
 *    Public License, version 2. It intended for educational use only, but its
 *    use
 *    is not limited thereto. */
/*    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 *    IS"
 *    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *    PURPOSE
 *    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *    CONSEQUEN-
 *    TIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 *    GOODS
 *    OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 *    HOWEVER
 *    CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *    LIABILITY,
 *    OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 *    THE USE
 *    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */
//*****************************************************************************

// Include standard library header files
#include <stdlib.h>
#include <avr/io.h>
// Include header for serial port class
#include "rs232int.h"
// Include header for the driver class
#include "imu_driver.h"
#include <util/twi.h>
#include <math.h>
#include <limits.h>




#define STOP 0
#define CONTINUE 1
/**
 * @brief      This is the constuctor for the imu_driver class.
 * @details    This constructor takes in several addresses and delivers them
 *             to the protected variables declared in our header file.
 *
 * @param      serial_PORT_incoming  This holds the address to the incoming data
 *           register
 * @param      input_PORT_incoming   This holds the address to the incoming
 *                                   data register
 * @param      diag_PORT_incoming    This holds address port to incoming diag
 *                                   port reg
 * @param      pwm_PORT_incoming     This holds the address to the incoming
 *                                   data register
 * @param      ocr_PORT_incoming     This holds the address to the incoming
 *                                   ocr register
 * @param[in]  input_APIN_incoming   This holds defined value for INA pin
 * @param[in]  input_BPIN_incoming   This holds defined value for INB pin
 * @param[in]  diag_PIN_incoming     This holds defined value for appropriate
 *                                   diag ping
 * @param[in]  pwm_PIN_incoming      This holds defined value for pwm pin
 */
imu_driver::imu_driver (
    emstream* serial_PORT_incoming,
    volatile uint8_t* input_PORT_incoming,
    volatile uint8_t* input_DDR_incoming,
    uint8_t input_SCL_incoming,
    uint8_t input_SDA_incoming
)
{
    //Set locals
    input_PORT = input_PORT_incoming;
    serial_PORT = serial_PORT_incoming;
    input_SCL = input_SCL_incoming;
    input_SDA = input_SDA_incoming;
    input_DDR = input_DDR_incoming;
    data = 0;
    count = 0;
    slave_address = 0x28;
    slave_address_write = 0x50;
    slave_address_read = 0x51;

    *input_PORT |= (1 << input_SDA) | (1 << input_SCL);

    // Do all the cool stuff in this method.

    initializeIMU();

    // Print a handy debugging message
    DBG (serial_PORT, "IMU Driver Construced Successfully" << endl);

}






void imu_driver::initializeIMU(void)
{
    /****** Begin initialization of I2C communication ******/

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
    TWBR = 72;

    TWCR &= ~(1 << TWIE);

    *serial_PORT << PMS("Initialized BNO055: ")
                 << endl
                 << PMS("Value in PRR0[CHECK: BIT:7 = 0]: ")
                 << bin
                 << PRR0
                 << endl
                 << PMS("Value in TWSR[CHECK: BIT1->0 = 00]: ")
                 << bin
                 << TWSR
                 << endl
                 << PMS("Value in TWBR[CHECK: 72]: ")
                 << TWBR
                 << endl;





}



uint8_t imu_driver::readIMU(uint8_t address, uint8_t byte_size)
{
    int thecount = -1;
    data = 0;
    /* Pseudo Code for I2C PROTOCOL READ*/

    // Initialze Start by master, change SDA while SCL is high
    // Send Slave Address and W/R bit attached to the LSB (0 Write FIRST)
    // Slave will send us an ACK (acknowledge) || BNO055 -> ACKS = 0
    // Master sends 8 bit address to write (7bits really doe)
    // Slave sends ACKS = 0
    // Master RESTARTS
    // Master sends Slave Address and a READ BIT OF 1
    // Slave Sends ACKS
    // Slave Sends 1 byte of data
    // Master Acknowledges ACK = 0
    // Slave keeps sending until Master makes NMACK
    // NMACK = 1, bus is released by Slave
    // Now Master can generate Stop

    *serial_PORT << PMS("Reading IMU..: ") << hex << slave_address << endl;
    *serial_PORT << PMS("IMU Address: ") << hex << slave_address << endl;
    *serial_PORT << PMS("IMU Address Write: ") << hex << slave_address_write << endl;
    *serial_PORT << PMS("IMU Address Read: ") << hex << slave_address_read << endl;
    *serial_PORT << PMS("Given Address: ") << hex << address << endl;
    *serial_PORT << PMS("Current Data: ") << hex << data << endl;

    //  TWCR (Two Wire Control Register)  (0xBC)
    //  TWCR (Two Wire Control Register)
    //  TWCR - TWINT | TWEA  | TWSTA | TWSTO | TWWC  | TWEN  | TWIE  |   -
    //  TWCR - --7---|---6---|---5---|---4---|---3---|---2---|---1---|---0----
    //  Set the Enable TWEN
    //  Set the Flag TWINT
    //  Set the Start Bit

    /***OFFICIAL START***/
    TWCR = (1 << TWEN) | (1 << TWINT) | (1 << TWSTA);
    //wait for the flag to be set
    waitForSet();
    checkError(TW_START);
    // Now we send Slave Address and wait for it
    TWDR = slave_address_write;
    clearEnable(STOP);
    waitForSet();
    checkError(TW_MT_SLA_ACK); //0x18
    //Send Register we want to access
    TWDR = address;
    clearEnable(STOP);
    waitForSet();
    checkError(TW_MT_DATA_ACK); //check if it data request ack (0x28)
    //Repeat Start
    TWCR = (1 << TWEN) | (1 << TWINT) | (1 << TWSTA);
    waitForSet();
    checkError(TW_REP_START); //check of restart ack (0x10)
    //send slave address of reg w/ READ
    TWDR = slave_address_read;
    clearEnable(STOP);
    waitForSet();
    checkError(TW_MR_SLA_ACK);

    *serial_PORT << PMS("1st Value Read at ") << hex << thecount << PMS(" : ") << hex << TWDR << endl;
    clearEnable(CONTINUE);
    waitForSet();
    checkError(TW_MR_DATA_ACK); //master has data send ACKM (0x40)
    //
    //
    thecount++;
    // now starts actual data
    while ((thecount + 1) < byte_size)
    {
        *serial_PORT << PMS("Value Read at ") << hex << thecount << PMS(" : ") << hex << TWDR << endl;
        //merge new data and our data holder
        mergeData(TWDR);
        shiftData();
        clearEnable(CONTINUE);
        waitForSet();
        checkError(TW_MR_DATA_ACK);
        thecount++;
    }



    //merge with last bits
    mergeData(TWDR);
    clearEnable(STOP);
    waitForSet();
    checkError(TW_MR_DATA_NACK); //master has data and sent nack

    TWCR = (1 << TWEN) | (1 << TWINT) | (1 << TWSTO);
    *serial_PORT << PMS("THE FINAL DATA: ") << hex << data << endl;
    //data_read -> put(data);

    return data;

}

void imu_driver::clearEnable(int val)
{
    if (val == STOP)
    {
        TWCR = ((1 << TWINT) | (1 << TWEN));
    }
    else if (val == CONTINUE)
    {
        TWCR = ((1 << TWINT) | (1 << TWEN) | (1 << TWEA));
        *serial_PORT << PMS("CONTINUES") << endl;
    }
    else
    {
        *serial_PORT << PMS("error in clear enable") << endl;
        while (1)
        {

        }
    }
    return;
}

void imu_driver::waitForSet(void)
{
    while (!(TWCR & (1 << TWINT)))
    {
        *serial_PORT << PMS("waiting..") << endl;
    }
    count++;
    *serial_PORT << PMS("Wait Set: ") << count << endl;
    *serial_PORT << PMS("STATUS: ") << hex << TWSR << endl;
    return;
}

void imu_driver::checkError(int code)
{
    if ((TWSR & 0xF8) != code)
    {
        *serial_PORT << PMS("Error at: ") << hex << count << endl;
        *serial_PORT << PMS("Needed: ") << hex << code << endl;
        *serial_PORT << PMS("Received: ") << hex << TWSR << endl;
        while (1)
        {


        }
    }
    else
    {
        return;
    }
}

void imu_driver::shiftData(void)
{
    //shift data by 8 to make way for other concat
    data = data << 8;
}

void imu_driver::mergeData(uint8_t newdata)
{
    data |= newdata;
}


/**
 * @brief                     This overloaded operator prints information
 *                            about a imu driver.
 *
 * @details                   It prints out appropriate information about the
 *                            imu driver being delivered in the parameter.
 *
 * @param      serpt          A pointer to the serial port used.
 * @param      motdrv         A pointer to a driver object
 *
 * @return                    A reference to the same serial device on which
 *                            we write information. This is used to string
 *                            together things to write with @c << operators
 */

emstream& operator << (emstream& serpt, imu_driver& imudrv)
{
    // Prints info to serial
    serpt << PMS ("imu Driver Input: ") << endl;


    return (serpt);
}

