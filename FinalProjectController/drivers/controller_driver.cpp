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
    memset(local_read_data, 0, 2);

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

    *p_local_serial_port << "Channel: " << channel_select << " ERROR: " << temp_error_adc << endl;
    local_error_select = temp_error_adc;
    return;
}


// void controller_driver::read(uint16_t* dat)
// {
//     uint16_t data[3];
//     int16_t div = 19;
//     memset(data, 0, 2);

//     // dat[0] = p_local_adc -> read_once(JOYSTICK_ANALOG_INPUT_X) - 512;
//     // dat[0] = (int16_t)dat[0] / div;
//     // dat[1] = ((p_local_adc -> read_once(JOYSTICK_ANALOG_INPUT_Y)) * 2) - 1024;
//     // dat[2] = p_local_adc -> read_once(JOYSTICK_ANALOG_INPUT_GEAR);

//     dat[0] = (p_local_adc -> read_once(JOYSTICK_ANALOG_INPUT_X) - 512) / 4;
//     dat[0] = (int16_t)dat[0] / div;
//     dat[1] = (((p_local_adc -> read_once(JOYSTICK_ANALOG_INPUT_Y)) * 2) - 1024) / 4;
//     dat[2] = (p_local_adc -> read_once(JOYSTICK_ANALOG_INPUT_GEAR)) /4;


//     *p_local_serial_port << "X Joystick: " << dec << (int16_t)dat[0] << endl;
//     *p_local_serial_port << "Y Joystick: " << dec << (int16_t)dat[1] << endl;
//     *p_local_serial_port << "Gear State: " << dec << (int16_t)dat[2] << endl;
//     *p_local_serial_port << endl << endl;

// }


void controller_driver::read(uint8_t* dat)
{
    int16_t read_x_joy, read_y_joy, read_gear;
    read_x_joy = read_x_joy = read_gear = 0;


    int16_t div = 19;

    // dat[0] = p_local_adc -> read_once(JOYSTICK_ANALOG_INPUT_X) - 512;
    // dat[0] = (int16_t)dat[0] / div;
    // dat[1] = ((p_local_adc -> read_once(JOYSTICK_ANALOG_INPUT_Y)) * 2) - 1024;
    // dat[2] = p_local_adc -> read_once(JOYSTICK_ANALOG_INPUT_GEAR);


    uint8_t gear_low_mask;
    // This will bring bit 6 low which isn't used in 2's
    gear_low_mask = 0b11011111;

    uint8_t x_joy;
    uint8_t y_joy;

    read_x_joy = p_local_adc -> read_once(JOYSTICK_ANALOG_INPUT_X) - 512;
    *p_local_serial_port << "pX Joystick: " << dec << read_x_joy << endl;
    //read_x_joy = (int16_t)dat[0] / div;
    read_x_joy = read_x_joy / div;
    read_y_joy = (((int16_t)(p_local_adc -> read_once(JOYSTICK_ANALOG_INPUT_Y)) * 2) - 1024) / 8;
    read_gear = ((int16_t)(p_local_adc -> read_once(JOYSTICK_ANALOG_INPUT_GEAR))) / 4;





    // x_joy = (uint8_t)data[0];
    // // bring value of -1023 to 1023 to -128 to 128
    // y_joy = (uint8_t)data[1] / 8;



    // Here 10 is the threshold
    if(read_gear < 20)
    {
      read_x_joy &= gear_low_mask;
    }
    else
    {
        read_x_joy |= ~(gear_low_mask);
    }



    dat[0] = (uint8_t)read_x_joy;
    dat[1] = (uint8_t)read_y_joy;
    // dat[2] = (uint8_t)data[2];

    *p_local_serial_port << "pX Joystick: " << dec << read_x_joy << endl;
    *p_local_serial_port << "Y Joystick: " << dec << read_y_joy << endl;
    *p_local_serial_port << "Gear State: " << dec << read_gear << endl;
    *p_local_serial_port << endl << endl;
    *p_local_serial_port << "xArr: " << bin << (int8_t)dat[0] << endl;
    *p_local_serial_port << "yArr: " << dec << (int8_t)dat[1] << endl;
    // *p_local_serial_port << "gArr: " << dec << dat[2] << endl;
    *p_local_serial_port << endl << endl;

}






