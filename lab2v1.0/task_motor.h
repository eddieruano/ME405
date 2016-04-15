/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** task_motor.h
** Description: This is the header file for our task_motor class
**
** Author: Eddie Ruano
** Date:   April 15, 2015
** -------------------------------------------------------------------------*/

// This define prevents this .h file from being included multiple times in a .cpp file

#ifndef TASK_MOTOR_H
#define TASK_MOTOR_H

#include <stdlib.h>                         // Prototype declarations for I/O functions
#include <avr/io.h>                         // Header for special function registers

#include "FreeRTOS.h"                       // Primary header for FreeRTOS
#include "task.h"                           // Header for FreeRTOS task functions
#include "queue.h"                          // FreeRTOS inter-task communication queues

#include "taskbase.h"                       // ME405/507 base task class
#include "time_stamp.h"                     // Class to implement a microsecond timer
#include "taskqueue.h"                      // Header of wrapper for FreeRTOS queues
#include "taskshare.h"                      // Header for thread-safe shared data

#include "rs232int.h"                       // ME405/507 library for serial comm.
#include "adc.h"                            // Header for A/D converter driver class
#include "motor_driver.h"                   // Header for Generic Motor driver


//-------------------------------------------------------------------------------------
/** @brief  
 *  @details 
 */

class task_motor : public TaskBase
{
private:
    // No private variables or methods for this class

protected:
    // No protected variables or methods for this class

public:
    // This constructor creates a generic task of which many copies can be made
    task_motor (const char*, unsigned portBASE_TYPE, size_t, emstream*);

    // This method is called by the RTOS once to run the task loop for ever and ever.
    void run (void);
};

#endif // _TASK_MOTOR_H