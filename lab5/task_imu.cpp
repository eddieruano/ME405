//*****************************************************************************
/** @file task_imu.cpp
 *  @brief     Task that controls BNO055 driver and updates position.
 *
 *  @details   task_imu updates the shares.h variables [Heading, Pitch, Roll] 
 *             every 100ms
 *
 *  @author Eddie Ruano
 *
 *  Revisions:
        @ 6/1/2016 <<EDD>> finally fixed lagg problem
        @ 5/28/2016 <<EDD>> created barebones

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

#include "textqueue.h"              // Header for text queue class
#include "task_imu.h"               // Header for this task
#include "shares.h"                 // Shared inter-task communications

/**
 * @brief       Constructor for task_imu creates an instance of the task.

 * @param       a_name        String name identifier for the task
 *
 * @param       a_priority    Task priority, 1-5, this particular task runs at 
 *                            [2]
 * @param       a_stack_size  Stack size for this task
 */
task_imu::task_imu (const char* a_name,
                        unsigned portBASE_TYPE a_priority,
                        size_t a_stack_size,
                        emstream* p_ser_dev,
                        bno055_driver* p_bno055_inc
                       )
    : TaskBase (a_name, a_priority, a_stack_size, p_ser_dev)
{
    local_bno055_ptr = p_bno055_inc;
}


/**
 * @brief       Runs to update our shares with Heading, Pitch, Roll
 *
 * @details     We want data that is reliable and that means the most 
 *              IMMEDIATE data in my opinion, So We update every 100ms.
 */
void task_imu::run (void)
{
    TickType_t previousTicks = xTaskGetTickCount ();

    for (;;)
    {
        // Update each individual share, once per cycle
        heading -> put(local_bno055_ptr -> getHeading());
        roll -> put(local_bno055_ptr -> getRoll());
        pitch -> put(local_bno055_ptr -> getPitch());

        // Increment the run counter in the parent class.
        runs++;

        /* This is a method we use to cause a task to make one run through its 
         * task loop every 50 milliseconds and let other tasks run at other 
         * times 
         */
        delay_from_for_ms (previousTicks, 100);
    }
}

