//*****************************************************************************
/** @file controller_driver.cpp
 *  @brief     Driver to normalize and read values from controller.
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
 *   Control Reader Driver Class an ATmega1281 based controller transmitting BT
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
#include "controller_driver.h"               // Header for this class
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
controller_driver::controller_driver (emstream* p_ser_port)
{
    // Set the debugging serial port pointer
    p_serial = p_ser_port;
    // Set the data array
    

    initialize();
}

/**
 * @brief       Inititalize the controller
 *
 * @details     
 *
 */
void controller_driver::initialize(void)
{
    //initialize variables in memory
    local_error_adc = 0;
    memset(local_data_read, 0, 3);

    // initialize the local adc driver
    p_local_adc = new adc*(p_serial);

    // Normalize channel 0, ADC0[X-DIR]
    normalize(JOYSTICK_ANALOG_INPUT_X);
    // Normalize channel 1, ADC1[Y-DIR]
    normalize(JOYSTICK_ANALOG_INPUT_Y);

    return;
}


bool controller_driver::normalize(uint8_t channel_select)
{

   int8_t count = 0; 
   local_error_adc = 0;
   while (count < 10)
   {
      local_error_adc = local_error_adc + (p_local_adc -> read_once(channel_select));
      count++;
   }

   local_error_adc = (512 - (local_error_adc / 10));

   *p_serial <<"Channel: " local_channel_select << " ERROR: " << local_error_adc << endl;
}


uint16_t *controller_driver::read(void)
{
    local_data_read[0] = p_local_adc(JOYSTICK_ANALOG_INPUT_X);
    local_data_read[1] = p_local_adc(JOYSTICK_ANALOG_INPUT_Y);




}



