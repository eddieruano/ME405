//*****************************************************************************
/** @file i2c_driver.h
 *  @brief     Header class for the i2c_driver class.
 *
 *  @details   Based heavily on JR Ridgely's methodology, incorporated with 
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
        @ 5/30/2016 EDD added comments, updated license.
        @ 5/28/2016 EDD fixed everything and made everything more
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

// Declare Header Guard
#ifndef _I2C_DRIVER_H
#define _I2C_DRIVER_H

#include <stdlib.h>                     // Standard C/C++ library stuff
#include "FreeRTOS.h"                   // Header for the RTOS
#include "semphr.h"                     // FreeRTOS semaphores (we use a mutex)
#include "emstream.h"                   // Header for base serial devices
#include <util/twi.h>                   // Header for establised ACK vernacular

class i2c_driver
{
protected:
    /// @brief       Pointer to serial port object used for debugging purposes
    emstream* p_serial;
    /// Function to reset timer used for timeouts
    void resetTimer(void);
    /// @brief       Pointer to timer variable
    uint32_t local_timer;
    /// @brief       Holds address of device currently being read/written from
    uint8_t local_current_address;
    /// @brief       Inititalizes the correct regsiters for I2C on ATmega1281
    void initialize(void);
    /// @brief       Waits for the TWINT flag to be set for an alotted time
    void waitForSet(void);
    /// @brief       Checks to see if response in status register is correct
    bool checkError(uint8_t);
    /// @brief       Mutex used to prevent simultaneous uses of the I2C port.
    SemaphoreHandle_t mutex;
    /// @brief       Starts I2C communication protocol
    bool start(void);
    /// Stops I2C communication protocol
    bool stop(void);
    /// Creats and checks a Restart condition on I2C interface
    bool restart(void);
    /*  @brief       Clears the flags resulting from stop and go comms on I2C interface, also re-enables the communcation, more like a Clear/Enable dual function.
    */
    void clearEnable(bool);
public:
    /// This constructor sets up the driver
    i2c_driver (
        emstream*
    );
    /// Function to read data using I2C
    bool readData(uint8_t, uint8_t, uint8_t *, uint8_t);
    /// Function to write data using I2C
    bool writeData(uint8_t, uint8_t, uint8_t);
    /// Function to ping address on SDA line
    bool ping(uint8_t);
};
#endif //  _I2C_DRIVER_H

