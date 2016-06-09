//*****************************************************************************
/** @file bno055_driver.cpp
 *  @brief     Main driver for the Adafruit BNO055 IMU Sensor. Based heavily
 *             on KTown's driver created for the Arduino line, however 
 *             optimized for speed.
 *
 *  @details   Based on KTown's layout, this driver focuses on obtaining data
 *             in a fast and constant way. It uses the MAX 400 KHz speed on 
 *             the BNO055 and sets the IMU at Startup. The IMU is ready to go 
 *             approx 5 sec after it is powered on and can update every 100 
 *             ms or so before being slow.
 *
 *  @author Eddie Ruano
 *
 *  Revisions: @ 6/1/2016 <<EDD>> made the heading, roll and pitch functions 
 *                                super streamlined.
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


// Include standard library header files
#include <stdlib.h>
#include <avr/io.h>
// Include header for serial port class
#include "rs232int.h"
// Include header for the driver class
#include "bno055_driver.h"
#include <util/twi.h>
#include <math.h>
#include <limits.h>
#include "imumaths.h"

/// Mask lower 8 bits of number, needed when data is held in multiple registers
#define LOW_MASK 0x00FF
/// Takes you to the start of Offset Registers which hold calibration offsets
#define OFFSET_REG_OFFSET 85

/**
 * @brief       Constructor for the BNO055, needs a given address which can be
 *              0x29 or 0x28, I prefer 0x29.
 *              
 * @param       p_serial_incoming  Pointer to serial RS232 port used for 
 *                                 debugging purposes.
 * @param       address_incoming   8bit address of the BNO055 chip on the SDA
 *                                 line, I prefer 0x29 but 0x28 works too 
 */
bno055_driver::bno055_driver (
    emstream* p_serial_incoming,
    uint8_t address_incoming
)
{

    p_serial = p_serial_incoming;
    //Set Local Read and Write commands based on address given
    local_bno055_address = address_incoming;
    p_i2c_driver = new i2c_driver(p_serial);

    // initialize the correct settings on BNO055
    initialize();
    // Print a handy debugging message
    DBG (p_serial, "BNO055 Driver Created at [SCL, SDA] Ports: PD0, PD1" << endl);
}


bool bno055_driver::initialize(void)
{
    //uint8_t data_dump[1];
    //ping the address to make sure we're in the right spot
    if (p_i2c_driver -> ping(local_bno055_address))
    {
        *p_serial << "Successful Ping of BNO055" << endl;
    }

    //now we set it to config mode
    setMode(OPERATION_MODE_CONFIG);

    /* Reset */
    writeRegister(BNO055_SYS_TRIGGER_ADDR, 0x20);

    while (!(p_i2c_driver -> ping(local_bno055_address)));
    setMode(OPERATION_MODE_CONFIG);
    // Config the BNO055
    calibrate();
    //Set Power to Normal
    writeRegister(BNO055_PWR_MODE_ADDR, POWER_MODE_NORMAL);

    //Set the start Page Address
    writeRegister(BNO055_PAGE_ID_ADDR, 0x00);

    //Mask the Triggers/Interrupts
    writeRegister(BNO055_SYS_TRIGGER_ADDR, 0x00);

    /* Set the output units */

    uint8_t unitsel = (0 << 7) | // Orientation = Android
                      (0 << 4) | // Temperature = Celsius
                      (1 << 2) | // Euler = Degrees
                      (0 << 1) | // Gyro = Rads
                      (0 << 0);  // Accelerometer = m/s^2
    writeRegister(BNO055_UNIT_SEL_ADDR, unitsel);


    setMode(OPERATION_MODE_NDOF);
    systemStatus();
    return true;
}

/**
 * @brief       Reads a register on the BNO055 using the I2C driver
 *              initialized locally when this driver is created.
 *
 * @details     !! Use with Caution does not check incoming array bounds !!
 *              Uses the I2C driver initialized in the constructor to read
 *              data from the SDA port of th BNO055. Given a target register,
 *              pointer to a 8bit data array, and amount of reads it will
 *              perform.
 *
 * @param       target  register we will read, or start reading from
 *
 * @param       data    Pointer to 8bit data array which holds data returned
 *
 * @param       reads  Number of reads caller chooses to make
 *
 * @return      Returns TRUE if underlying I2C function is able to make all
 *              the reads, FALSE otherwise
 */
bool bno055_driver::readRegister(bno055_reg_t target, uint8_t *data, uint8_t reads)
{
    p_i2c_driver -> readData(local_bno055_address, (uint8_t)target, data, reads);
    return true;
}

/**
 * @brief       Write 8bit value to a register in BNO055
 * 
 * @param       reg       Register target
 * 
 * @param       data_inc  8bit data to be written
 * 
 * @return      Returns TRUE if process successful, FALSE otherwise
 * 
 */
bool bno055_driver::writeRegister(bno055_reg_t reg, uint8_t data_inc)
{
    p_i2c_driver -> writeData(local_bno055_address, (uint8_t)reg, data_inc);
    return true;
}


/**
 * @brief       Sets the Mode of the BNO055 after comparing the mode to list
 *              of legal modes.
 *
 * @details     Fails at compiler time if the mode does not check against the
 *              typedef, also stores the mode locally for debugging purposes.
 *
 * @param       mode  New mode that the BNO055 will receive
 *
 */

void bno055_driver::setMode(bno055_opmode_t mode)
{
    //Checks to see if the mode is a legal mode
    local_bno055_mode = mode;
    writeRegister(BNO055_OPR_MODE_ADDR, (uint8_t)mode);
}

/**
 * @brief       { function_description }
 * @param       void  { parameter_description }
 * @return      { description_of_the_return_value } */
void bno055_driver::systemStatus(void)
{
    writeRegister(BNO055_PAGE_ID_ADDR, 0);

    /* System Status (see section 4.3.58)
       ---------------------------------
       0 = Idle
       1 = System Error
       2 = Initializing Peripherals
       3 = System Iniitalization
       4 = Executing Self-Test
       5 = Sensor fusio algorithm running
       6 = System running without fusion algorithms */
    readRegister((bno055_reg_t)0x39, data_dump, 1);
    *p_serial << "System Status: " << data_dump[0] << endl;

    /* Self Test Results (see section )
       --------------------------------
       1 = test passed, 0 = test failed

       Bit 0 = Accelerometer self test
       Bit 1 = Magnetometer self test
       Bit 2 = Gyroscope self test
       Bit 3 = MCU self test

       0x0F = all good! */

    // if (self_test_result != 0)
    //   *self_test_result = read8(BNO055_SELFTEST_RESULT_ADDR);
    readRegister(BNO055_SELFTEST_RESULT_ADDR, data_dump, 1);
    *p_serial << "System Self Test: " << data_dump[0] << endl;

    /* System Error (see section 4.3.59)
       ---------------------------------
       0 = No error
       1 = Peripheral initialization error
       2 = System initialization error
       3 = Self test result failed
       4 = Register map value out of range
       5 = Register map address out of range
       6 = Register map write error
       7 = BNO low power mode not available for selected operat ion mode
       8 = Accelerometer power mode not available
       9 = Fusion algorithm configuration error
       A = Sensor configuration error */

    // if (system_error != 0)
    //   *system_error     = read8(BNO055_SYS_ERR_ADDR);
    readRegister(BNO055_SYS_ERR_ADDR, data_dump, 1);
    *p_serial << "System Error: " << data_dump[0] << endl;
}




/**
 * @brief       Gets and returns BNO055 temperature in degrees Celsius
 *
 * @details     Queries the temperature of BNO055 by reading the register that
 *              contains temperature information
 *
 * @return      temperature signed 8bit value;
 */
int8_t bno055_driver::getTemp(void)
{
    readRegister(BNO055_TEMP_ADDR, data_dump, 1);
    int8_t temperature = (int8_t)data_dump[0];
    return temperature;
}

/**
 * @brief       Print All Registers in BNO055 Page 0
 * 
 */
void bno055_driver::printAll(void)
{
    uint8_t big_data_dump[106];
    memset (big_data_dump, 0, 106);
    uint8_t count;
    readRegister(BNO055_CHIP_ID_ADDR , big_data_dump, 106);
    for (count = 0; count < 106; count++)
    {
        *p_serial << "Register: " << hex << count << ": " << big_data_dump[count] << endl;
    }
    return;
}

/**
 * @brief       { function_description }
 * @param       vector_type  { parameter_description }
 * @return      { description_of_the_return_value } 
 */
imu::Vector<3> bno055_driver::getVector(vector_type_t vector_type)
{
    imu::Vector<3> xyz;

    //memset (data_dump, 0, 6);

    int16_t x, y, z;
    x = y = z = 0;

    //Read vector data which is 6 bytes long
    readRegister((bno055_reg_t)vector_type, data_dump, 6);

    x = ((int16_t)data_dump[0]) | (((int16_t)data_dump[1]) << 8);
    y = ((int16_t)data_dump[2]) | (((int16_t)data_dump[3]) << 8);
    z = ((int16_t)data_dump[4]) | (((int16_t)data_dump[5]) << 8);

    /* Convert the value to an appropriate range (section 3.6.4) */
    /* and assign the value to the Vector type */
    switch (vector_type)
    {
    case VECTOR_MAGNETOMETER:
        /* 1uT = 16 LSB */
        xyz[0] = ((double)x) / 16.0;
        xyz[1] = ((double)y) / 16.0;
        xyz[2] = ((double)z) / 16.0;
        break;
    case VECTOR_GYROSCOPE:
        /* 1rps = 900 LSB */
        xyz[0] = ((double)x) / 900.0;
        xyz[1] = ((double)y) / 900.0;
        xyz[2] = ((double)z) / 900.0;
        break;
    case VECTOR_EULER:
        /* 1 degree = 16 LSB */
        //Heading
        xyz[0] = ((double)x) / 16.0;
        //Roll
        xyz[1] = ((double)y) / 16.0;
        //Pitch
        xyz[2] = ((double)z) / 16.0;
        break;
    case VECTOR_ACCELEROMETER:
    case VECTOR_LINEARACCEL:
    case VECTOR_GRAVITY:
        /* 1m/s^2 = 100 LSB */
        xyz[0] = ((double)x) / 100.0;
        xyz[1] = ((double)y) / 100.0;
        xyz[2] = ((double)z) / 100.0;
        break;
    }

    return xyz;
}


/**
 * @brief       { function_description }
 * @param       void  { parameter_description }
 * @return      { description_of_the_return_value } 
 */
int16_t bno055_driver::getHeading(void)
{
    int16_t x;
    x = 0;
    readRegister((bno055_reg_t)VECTOR_EULER, data_dump, 2);
    x = ((int16_t)data_dump[0]) | (((int16_t)data_dump[1]) << 8);
    //imu::Vector<3> vect
    return (x / 16);
    //return (int16_t)vect.x();
}

/**
 * @brief       { function_description }
 * @param       void  { parameter_description }
 * @return      { description_of_the_return_value } 
 */
int16_t bno055_driver::getRoll(void)
{
    //return getVector(VECTOR_EULER)[1];
    // imu::Vector<3> vect = getVector(VECTOR_EULER);
    // return (int16_t)vect.y();
    int16_t x;
    x = 0;
    readRegister(BNO055_EULER_R_LSB_ADDR, data_dump, 2);
    x = ((int16_t)data_dump[0]) | (((int16_t)data_dump[1]) << 8);
    //imu::Vector<3> vect
    return (x / 16);
}

/**
 * @brief       { function_description }
 * @param       void  { parameter_description }
 * @return      { description_of_the_return_value } */
int16_t bno055_driver::getPitch(void)
{

    // return getVector(VECTOR_EULER)[2];
    // imu::Vector<3> vect = getVector(VECTOR_EULER);

    // return (int16_t)vect.z();
    // BNO055_EULER_P_LSB_ADDR

    int16_t x;
    x = 0;
    readRegister(BNO055_EULER_P_LSB_ADDR, data_dump, 2);
    x = ((int16_t)data_dump[0]) | (((int16_t)data_dump[1]) << 8);
    //imu::Vector<3> vect
    return (x / 16);
}

/**
 * @brief       Gets calibration values and places them into given array
 *
 * @details     ...
 *
 * @param       status  Pointer to array that will hold the four values of
 *                      calibration
 *
 */
void bno055_driver::getCalibration(uint8_t* status)
{
    uint8_t read_call[1];
    memset(read_call, 0, 1);
    readRegister(BNO055_CALIB_STAT_ADDR, read_call, 1);
    *p_serial << "Read Cali: " << hex << read_call[0] << endl;

    if (data_dump[0] != NULL) {
        status[0] = ((read_call[0]) >> 6) & 0x03;

        status[1] = ((read_call[0]) >> 4) & 0x03;

        status[2] = ((read_call[0]) >> 2) & 0x03;

        status[3] = ((read_call[0])) & 0x03;
    }
    *p_serial << "Sys  : " << status[0] << endl;
    *p_serial << "Gyro : " << status[1] << endl;
    *p_serial << "Accel: " << status[2] << endl;
    *p_serial << "Mag  : " << status[3] << endl;
}

bool bno055_driver::getCalibrationStatus(void)
{
    uint8_t read_data[1];
    memset(read_data, 0, 1);

    readRegister(BNO055_CALIB_STAT_ADDR, read_data, 1);
    if (read_data[0] & 0x3F)
    {
        return true;
    }
    return false;
}

void bno055_driver::calibrate(void)
{
    // Using these values to calibrate
    // Accelerometer: 0 65515 17
    // Gyro: 65535 65535 0
    // Mag: 65520 65467 4
    // Accel Radius: 1000
    // Mag Radius: 71
    uint8_t i;
    uint8_t calibration_data[22];
    memset(calibration_data, 0, 22);
    uint16_t* Accel_X;
    uint16_t* Accel_Y;
    uint16_t* Accel_Z;
    uint16_t* Gyro_X;
    uint16_t* Gyro_Y;
    uint16_t* Gyro_Z;
    uint16_t* Mag_X;
    uint16_t* Mag_Y;
    uint16_t* Mag_Z;
    uint16_t* Accel_Rad;
    uint16_t* Mag_Rad;
    *Accel_X    = 0;
    *Accel_Y    = 65515;
    *Accel_Z    = 17;
    *Gyro_X     = 65535;
    *Gyro_Y     = 65535;
    *Gyro_Z     = 0;
    *Mag_X      = 65520;
    *Mag_Y      = 65467;
    *Mag_Z      = 4;
    *Accel_Rad  = 1000;
    *Mag_Rad    = 71;


    /* Set Accel Offsets */
    // Accel X
    calibration_data[0] = (uint8_t)(LOW_MASK & *Accel_X);
    calibration_data[1] = (uint8_t)((*Accel_X) >> 8);
    // Accel Y
    calibration_data[2] = (uint8_t)(LOW_MASK & *Accel_Y);
    calibration_data[3] = (uint8_t)((*Accel_Y) >> 8);
    // Accel Z
    calibration_data[4] = (uint8_t)(LOW_MASK & *Accel_Z);
    calibration_data[5] = (uint8_t)((*Accel_Z) >> 8);

    /* Set Gyro Offsets */
    // Gyro X
    calibration_data[6] = (uint8_t)(LOW_MASK & *Gyro_X);
    calibration_data[7] = (uint8_t)((*Gyro_X) >> 8);
    // Gyro Y
    calibration_data[8] = (uint8_t)(LOW_MASK & *Gyro_Y);
    calibration_data[9] = (uint8_t)((*Gyro_Y) >> 8);
    // Gyro Z
    calibration_data[10] = (uint8_t)(LOW_MASK & *Gyro_Z);
    calibration_data[11] = (uint8_t)((*Gyro_Z) >> 8);

    /* Set Mag Offsets */
    // Mag X
    calibration_data[12] = (uint8_t)(LOW_MASK & *Mag_X);
    calibration_data[13] = (uint8_t)((*Mag_X) >> 8);
    // mag Y
    calibration_data[14] = (uint8_t)(LOW_MASK & *Mag_Y);
    calibration_data[15] = (uint8_t)((*Mag_Y) >> 8);
    // Mag Z
    calibration_data[16] = (uint8_t)(LOW_MASK & *Mag_Z);
    calibration_data[17] = (uint8_t)((*Mag_Z) >> 8);

    /* Set Accel Radius Offset */
    // Accel Radius LSB
    calibration_data[18] = (uint8_t)(LOW_MASK & *Accel_Rad);
    calibration_data[19] = (uint8_t)((*Accel_Rad) >> 8);

    /* Set Mag Radius Offset */
    // Mag Radius LSB
    calibration_data[20] = (uint8_t)(LOW_MASK & *Mag_Rad);
    calibration_data[21] = (uint8_t)((*Mag_Rad) >> 8);

    for (i = 0; i <= 21; i++)
    {
        //*p_serial << "R0x" << hex << (i + OFFSET_REG_OFFSET) << ": "  << calibration_data[i] << endl;
        writeRegister((bno055_reg_t)(OFFSET_REG_OFFSET + i), calibration_data[i]);
    }

}

void bno055_driver::printCalibrationData(void)
{
    uint8_t dat_temp[21];
    uint8_t i;
    readRegister(ACCEL_OFFSET_X_LSB_ADDR, dat_temp, 21);
    for(i = 0; i < 22; i++)
    {
        *p_serial << "Register Hex 0x" << hex << (OFFSET_REG_OFFSET + i) << ": " << dat_temp[i] << endl;
    }
}