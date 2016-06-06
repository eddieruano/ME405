//*****************************************************************************
/** @file bno055_driver.cpp
 *  @brief      This is the header file for the BNO055 driver.
 *
 *  @details    This contains all prototypes of functions used in the BNO055
 *
 *  @author Eddie Ruano
 *
 *  Revisions: @ 5/28/2016 <<EDD>> fixed everything and made everything more
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

// Declare Header Guards
#ifndef _BNO055_
#define _BNO055_


#include <stdlib.h>             // Include standard library header files
#include <avr/io.h>             // Include header for serial port class
#include "rs232int.h"           // Include header for the driver class
#include <math.h>               // header for math operations performed later 
#include <limits.h>             // header for limit operations
#include "i2c_driver.h"         // I2C driver used to interface w/ BNO
#include "Adafruit_BNO055.h"    // Reusing declarations in Adafruit's Arduino library written by KTWON 
#include "imumaths.h"           // libraries used for calculations

class bno055_driver
{
private:
    //no private variables needed.
protected:
    /// Pointer to serial port used for debugging purposes
    emstream* p_serial;
    /// Instance of the i2c driver used for comms with the BNO055
    i2c_driver* p_i2c_driver;
    /// Variable holds the correct hex address for the BNO055
    uint8_t local_bno055_address;
    /// This structure holds the current operational mode of the BNO055
    bno055_opmode_t local_bno055_mode;


    ///set the public constructor and the public methods
public:
    /// Constructor only requires serial and address
    bno055_driver (
        emstream*,
        uint8_t
    );
    /// Sets Up the BNO055 & Places it in the correct mode
    bool initialize(void);
    /// Reads a register from the BNO055
    bool readRegister(bno055_reg_t reg, uint8_t *, uint8_t);
    /// Writes a register to the BNO055
    bool writeRegister(bno055_reg_t, uint8_t);
    /// Sets the Mode of the BNO055
    void setMode(bno055_opmode_t);
    /// Prints the system status registers
    void systemStatus(void);
    /// Returns the temperature in Celsius
    int8_t getTemp(void);
    /// Data holder variable for reads and 
    uint8_t data_dump[6];
    /// Prints all registers on Page 0 of BNO055
    void printAll(void);
    /// Returns a Vector holding the type specified, look at Adafruit_BNO055 for avaliable parameters
    imu::Vector<3> getVector(vector_type_t vector_type);
    int16_t getHeading(void);
    /// Gets current Roll 
    int16_t getRoll(void);
    /// Gets current Pitch
    int16_t getPitch(void);
    /// Given array of size 4 (uint8_t)s it returns the Calibration of S,A,G,M
    void getCalibration(uint8_t*);
    /// Returns true if data from current BNO055 is reliable
    bool getCalibrationStatus(void);
    /// Sets the offset registers
    void calibrate(void);
    /// Prints the offset registers
    void printCalibrationData(void);
};
#endif
