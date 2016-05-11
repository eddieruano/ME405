//**************************************************************************************
/**
 @file task_hctl_2000.h
 * This file contains the header for a task that updates the encoder information
 * by reading it from the dedicated hardware - an HCTL-2000 encoder counter
 * with a 10 MHz oscillator attached as the clock.
**/
//**************************************************************************************

#ifndef _TASK_HCTL_
// {
#define _TASK_HCTL_

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
#include "hctl.h"                           // Header for encoder driver class

//-------------------------------------------------------------------------------------
/** @brief   This task handles updating the encoder data by retrieving it from
 *           the HCTL-2000 hardware.
 *  @details The encoder counter is read using a driver in files @c hctl.h and @c hctl.cpp.
 *           Code in this task tracks the values and prints them.
 */

class task_hctl_2000 : public TaskBase
{
private:
	hctl* counter;

protected:
	// No protected variables or methods for this class

public:
	task_hctl_2000 (const char*, unsigned portBASE_TYPE, size_t, emstream*, hctl* target);

	// This method is called by the RTOS once to run the task loop for ever and ever.
	void run (void);
};

// }
#endif