//*****************************************************************************
/** @file controller_driver.h
 *  @brief     Header class for the conroller_driver class.
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

// Declare Header Guard
#ifndef _CONT_DRIVER_H
#define _CONT_DRIVER_H

#include <stdlib.h>                     // Standard C/C++ library stuff
#include "FreeRTOS.h"                   // Header for the RTOS
#include "semphr.h"                     // FreeRTOS semaphores (we use a mutex)
#include "emstream.h"                   // Header for base serial devices
#include <util/twi.h>                   // Header for establised ACK vernacular
#include "adc.h"


/// Port where ADC located
#define ADC_PORT                    PORTF
#define JOYSTICK_ANALOG_INPUT_X     PF0
#define JOYSTICK_ANALOG_INPUT_Y     PF1
#define JOYSTICK_ANALOG_INPUT_GEAR  PF2


class controller_driver
{
protected:
    /// @brief       Pointer to serial port object used for debugging purposes
    emstream* p_local_serial_port;
    /// @brief       Initializes the registers and A/D converter
    void initialize(void);
    /// @brief       Normalizes values on Joysticks and sets correct IDLEs
    void normalize(uint8_t, uint16_t);
    /// @brief       Array holds ADC0[X-DIR], ADC1[Y-DIR], ADC2[GEAR]
    uint16_t local_read_data[3];
    uint16_t local_error_x;
    uint16_t local_error_y;
    adc* p_local_adc;

public:
    /// This constructor sets up the driver
    controller_driver (
        emstream*
    );

    void read(uint16_t*);
    void paired(bool t)
    {
        if (t)
        {
            DDRE |= (1 << PE5);
            PORTE |= (1 << PE5);
            *p_local_serial_port << "ON" << endl;
            *p_local_serial_port << hex << PINE << endl;
        }
        else
        {
            DDRE &= ~(1 << PE5);
            PORTE &= ~(1 << PE5);
            *p_local_serial_port << "OFF" << endl;
            *p_local_serial_port << hex << PINE << endl;
        }
    }

    void drivemode(bool t)
    {
        if (t)
        {
            DDRE |= (1 << PE6);
            PORTE |= (1 << PE6);
            // *p_local_serial_port << "ON" << endl;
            // *p_local_serial_port << hex << PINE << endl;
        }
        else
        {
            DDRE &= ~(1 << PE6);
            PORTE &= ~(1 << PE6);
            // *p_local_serial_port << "OFF" << endl;
            // *p_local_serial_port << hex << PINE << endl;
        }
    }
};
#endif //  _CONT_DRIVER_H

