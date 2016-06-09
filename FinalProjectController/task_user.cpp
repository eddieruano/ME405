//*****************************************************************************
/** @file task_user.cpp
 *  @brief     This file will serve as main interaction source for the final 
 *             project. 
 *
 *  @details   Probably about 10% done lol
 *
 *  @author Eddie Ruano
 *  @author JR Ridgely
 *
 *  Revisions:  @li 6/1/2016 <<EDD>> I don't even think I'm not sick enough to 
 *                                   code right meow.
 *              @li 6/1/2016 <<EDD>> Created barebones
 *  License:
 *   LegoCar Control Task for use with FreeRTOS
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

#include <avr/io.h>                         // Port I/O for SFR's
#include <avr/wdt.h>
#include "task_user.h"                      // task_user header file
#include "ansi_terminal.h" //testing ansi
#include "task_user_library.h"

//Set these defines in case we want to change things later.

/**  This constant sets how many RTOS ticks the task delays if the user's not
 *   talking The duration is calculated to be about 5 ms.
 */
const TickType_t ticks_to_delay = ((configTICK_RATE_HZ / 1000) * 5);

//-----------------------------------------------------------------------------
/**
 * @brief      Constructor that creates a new task to interact with the user.
 *             All incoming paramters are initialized in the parent class
 *             construtor TaskBase.
 * @details    Added new initializations for private variables added in order
 *             to clean up the clutter in the main case statement in the
 *             'run()' method.
 *
 * @param[in]  a_name         A character string which will be the name of
 *                            this task
 * @param[in]  a_priority     The priority at which this task will initially
 *                            run (default: 0)
 * @param[in]  a_stack_size   The size of this task's stack in bytes
 *                            (default: configMINIMAL_STACK_SIZE)
 * @param      p_ser_dev      Pointer to a serial device (port, radio, SD
 *                            card, etc.
 */
task_user::task_user (const char* a_name,
                      unsigned portBASE_TYPE a_priority,
                      size_t a_stack_size,
                      emstream* p_ser_dev
                     )
    : TaskBase (a_name, a_priority, a_stack_size, p_ser_dev)
{

}


/**
 * @brief      This method runs when the 'task_user' is called.
 *
 * @details    This method runs inifitely when the task is called so all logic
 *             is contained in here. A for(;;) loop runs until it gets to the
 *             end where it delays for 1 ms to allow other lower level
 *             priority tasks a chance to run.
 */
void task_user::run (void)
{
    time_stamp a_time;             // Holds the time so it can be displayed
    number_entered = 0;            // Holds a number being entered by user
    uint8_t count;
    count = 0;
    uint8_t data[16];
    //
    for (;;)
    {
        switch (state)
        {
        // Initialize first task where we wait for
        case (0):
            printMainMenu();
            // If the user typed a character, read
            if (hasUserInput())
            {
                switch (char_in)
                {
                case ('d'):
                    //Enter Drive Mode, Free Drive Mode 

                    break;
                case ('l'):
                    // Enter Straight line mode, Car tries to follow IMU heading
                    break;
                case ('e'):
                    // Enter Circle Mode, car does a figure 8
                    break;
                default:
                    *p_serial << '"' << char_in << PMS ("\": WTF?") << endl;
                    break;
                }; // End switch for characters
            } // End if a character was received

            break; // End of state 0
        case (1):
            break;
        default:
            *p_serial << PMS ("Illegal state! Resetting AVR") << endl;
            wdt_enable (WDTO_120MS);
            for (;;);
            break;

        } // End switch state

        runs++;                             // Increment counter for debugging

        // No matter the state, wait for approximately a millisecond before we
        // run the loop again. This gives lower priority tasks a chance to run
        delay_ms (1);
    }

}
