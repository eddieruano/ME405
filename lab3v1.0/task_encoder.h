/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** task_encoder.h
** Description: This is the header file for our task_encoder class
**
** Author: Eddie Ruano
** Date:   April 15, 2015
** -------------------------------------------------------------------------*/

// This define prevents this .h file from being included multiple times in a .cpp file

#ifndef TASK_ENCODER_H
#define TASK_ENCODER_H

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


//-------------------------------------------------------------------------------------
/** @brief
 *  @details
 */

class task_encoder : public TaskBase
{
private:
	/// No private variables or methods for this class

protected:
	/// No protected variables or methods for this class

public:
	/// This constructor creates a generic task of which many copies can be made
	task_encoder (const char*, unsigned portBASE_TYPE, size_t, emstream*);

	/// This method is called by the RTOS once to run the task loop for ever and ever.
	void run (void);
};

#endif // _TASK_ENCODER_H