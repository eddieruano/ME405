//*****************************************************************************
/** @file task_imu.h
 *  @brief     Header and Prototypes for IMU Sensor Task to control an 
 *             Adafruit BNO055 IMU Sensor
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
        @ 5/30/2016 EDD added comments, updated license.
        @ 5/28/2016 EDD created barebones

 *  License:
 *   BNO055 Sensor Control Task for use with FreeRTOS
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
#ifndef _TASK_IMU_ 
#define _TASK_IMU_

#include <stdlib.h>                 // Prototype declarations for I/O functions
#include <avr/io.h>                 // Header for special function registers
#include "FreeRTOS.h"               // Primary header for FreeRTOS
#include "task.h"                   // Header for FreeRTOS task functions
#include "queue.h"                  // FreeRTOS inter-task communication queues
#include "taskbase.h"               // ME405/507 base task class
#include "time_stamp.h"             // Class to implement a microsecond timer
#include "taskqueue.h"              // Header of wrapper for FreeRTOS queues
#include "taskshare.h"              // Header for thread-safe shared data
#include "rs232int.h"               // ME405/507 library for serial comm.
#include "task_imu.h"               // Header for the BNO055 Sensor Driver
#include "bno055_driver.h"

/**
 * @brief       { class_description } 
 * 
 */
class task_imu : public TaskBase
{
private:
    /// No private variables or methods for this class
protected:
    bno055_driver* local_bno055_ptr;

public:
    /// This constructor creates a generic task of which many copies can be made
    task_imu (const char*, unsigned portBASE_TYPE, size_t, emstream*, bno055_driver*);

    /// This method is called by the RTOS once to run the task loop for ever and ever.
    void run (void);
    
};
#endif // _TASK_IMU_
