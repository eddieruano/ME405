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

/**
 * @brief       Sets up a driver for reading from controller.
 *
 * @param       p_ser_port  Pointer to the serial output for debugging
 *              purposes
 *
 */
controller_driver::controller_driver (emstream* p_ser_port)
{
    // Set the debugging serial port pointer
    p_local_serial_port = p_ser_port;
    // Init
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
    //memset(local_read_data, 0, 2);

    // initialize the local adc driver
    p_local_adc = new adc(p_local_serial_port);

    // Normalize channel 0, ADC0[X-DIR]
    normalize(JOYSTICK_ANALOG_INPUT_X, local_error_x);
    // Normalize channel 1, ADC1[Y-DIR]
    normalize(JOYSTICK_ANALOG_INPUT_Y, local_error_y);
    memset(local_read_data, 0, 3);

    return;
}


void controller_driver::normalize(uint8_t channel_select, uint16_t local_error_select)
{

   int8_t count = 0; 
   uint16_t temp_error_adc = 0;
   while (count < 10)
   {
      temp_error_adc = temp_error_adc + (p_local_adc -> read_once(channel_select));
      count++;
   }

   temp_error_adc = (512 - (temp_error_adc / 10));

   *p_local_serial_port <<"Channel: " << channel_select << " ERROR: " << temp_error_adc << endl;
   local_error_select = temp_error_adc;
   return;
}


void controller_driver::read(uint16_t* dat)
{
    uint16_t data[4];
    memset(data, 0, 3);
    // dat[0] = p_local_adc -> read_once(JOYSTICK_ANALOG_INPUT_X);
    // dat[1] = p_local_adc -> read_once(JOYSTICK_ANALOG_INPUT_Y);
    // dat[2] = p_local_adc -> read_once(JOYSTICK_ANALOG_INPUT_GEAR);
    // dat[3] = 0xFFF8;

    dat[0] = 0x1234;
    dat[1] = 0x5678;
    dat[2] = 0x9ABC;
    dat[3] = 0xDEF0;

}






