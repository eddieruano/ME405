//*****************************************************************************
/** @file main.cpp
 *  @brief     Contains main function in which RTOS libraries are loaded and 
 *             the task scheduler is started.
 *
 *  @details   The main function contains the instance of the bno055 we're 
 *             going to test an the task IMU which currently only updates the
 *             shares with the values in the bno055. If we want to change the 
 *             pins for the BNO055, we unfortunately can't because the only 
 *             i2c pins on the board are wired to PD0, and PD1 which in turn 
 *             are IRQ an CE on the MiRf socket.
 *
 *  @author Eddie Ruano
 *
 *  Revisions:
        @ 6/1/2016  EDD patched up comments
        @ 6/1/2016  EDD finally fixed lagg problem
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
#include <stdlib.h>             // Prototype declarations for I/O functions
#include <avr/io.h>             // Port I/O for SFR's
#include <avr/wdt.h>            // Watchdog timer header
#include <string.h>             // Functions for C string handling

#include "FreeRTOS.h"           // Primary header for FreeRTOS
#include "task.h"               // Header for FreeRTOS task functions
#include "queue.h"              // FreeRTOS inter-task communication queues
#include "croutine.h"           // Header for co-routines and such
#include "rs232int.h"           // ME405/507 library for serial comm.
#include "ansi_terminal.h"      // Ansi terminal library for data printing
#include "time_stamp.h"         // Class to implement a microsecond timer
#include "taskbase.h"           // Header of wrapper for FreeRTOS tasks
#include "textqueue.h"          // Wrapper for FreeRTOS character queues
#include "taskqueue.h"          // Header of wrapper for FreeRTOS queues
#include "taskshare.h"          // Header for thread-safe shared data
#include "shares.h"             // Global ('extern') queue declarations

#include "task_user.h"          // Header for task user
#include "task_imu.h"           // Header for the IMU task
#include "i2c_driver.h"         // Header for the i2c driver used for comms
#include "bno055_driver.h"      // Header for bno055_driver used in IMU task



/// prototypes of TextQueues
/// Text queue for printing text
TextQueue* p_print_ser_queue;
/*Start IMU variables */

///Holds current heading tasken from BNO055 chip on BNO055 driver every 100 ms
TaskShare<int16_t>* heading;
/// Holds current roll taken from BNO055 chip on BNO055 driver every 100 ms 
TaskShare<int16_t>* roll;
/// Holds current pitch taken from BNO055 chip on BNO055 driver every 100 ms 
TaskShare<int16_t>* pitch;


/**
 * @brief      Main function runs until board is powered off
 *
 */
int main (void)
{
    // Disable the watchdog timer unless it's needed later. This is important because
    // sometimes the watchdog timer may have been left on...and it tends to stay on
    MCUSR = 0;
    wdt_disable ();

    rs232* p_ser_port = new rs232 (9600, 1);
    // print this identifier line.
    *p_ser_port << clrscr << PMS ("ME405 Lab 5 IMU Controller") << endl;

    // Create the queues and other shared data items here
    p_print_ser_queue = new TextQueue (32, "Print", p_ser_port, 10);

    // Start IMU Position variables that will updated in task
    heading         = new TaskShare<int16_t> ("Vehicle Heading");
    roll            = new TaskShare<int16_t> ("Vehicle Roll");
    pitch           = new TaskShare<int16_t> ("Vehicle Pitch");

    // create pointer to instance of bno055_driver
    bno055_driver* bno055_ptr = new bno055_driver(p_ser_port, 0x29);

    // create task to visually test IMU
    new task_user ("UserInt", task_priority (1), 260, p_ser_port, bno055_ptr);

    // create task to update the IMU shares, given priority over the task_user
    new task_imu ("IMU Sensor Task", task_priority(2), 280, p_ser_port, bno055_ptr);

    // starts the task schduler for RTOS
    vTaskStartScheduler ();
}

