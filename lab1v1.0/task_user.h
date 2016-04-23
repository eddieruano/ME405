//**************************************************************************************
/** @file task_user.h
 *    Header file for task_user in which the prototypes for that class are
 *    created. Added New Methods.
 *
 *  Revisions:
 *    @li 09-30-2012 JRR Original file was a one-file demonstration with two tasks
 *    @li 10-05-2012 JRR Split into multiple files, one for each task
 *    @li 10-25-2012 JRR Changed to a more fully C++ version with class task_user
 *    @li 11-04-2012 JRR Modified from the data acquisition example to the test suite
 *    @li 01-04-2014 JRR Changed base class names to TaskBase, TaskShare, etc` *
 *  License:
 *    This file is copyright 2012 by JR Ridgely and released under the Lesser GNU
 *    Public License, version 2. It intended for educational use only, but its use
 *    is not limited thereto. */
/*    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUEN-
 *    TIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 *    OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *    CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 *    OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */
//**************************************************************************************

// This define prevents this .h file from being included multiple times in a .cpp file
#ifndef _TASK_USER_H_
#define _TASK_USER_H_

#include <stdlib.h>                         // Prototype declarations for I/O functions

#include "FreeRTOS.h"                       // Primary header for FreeRTOS
#include "task.h"                           // Header for FreeRTOS task functions
#include "queue.h"                          // FreeRTOS inter-task communication queues

#include "rs232int.h"                       // ME405/507 library for serial comm.
#include "adc.h"                            // Header for A/D converter class driver
#include "motor_driver.h"
#include "time_stamp.h"                     // Class to implement a microsecond timer
#include "taskbase.h"                       // Header for ME405/507 base task class
#include "taskqueue.h"                      // Header of wrapper for FreeRTOS queues
#include "textqueue.h"                      // Header for a "<<" queue class
#include "taskshare.h"                      // Header for thread-safe shared data

#include "shares.h"                         // Global ('extern') queue declarations


/// This macro defines a string that identifies the name and version of this
/// program.
#define PROGRAM_VERSION PMS ("ME405 Lab 2 Motor Driver Interface")



/**
 * @brief      This task will interact with the user and make the motors
 * 			   operate based on their input. Added some methods so make the
 * 			   printing of information more streamlined.
 */





class task_user : public TaskBase
{
private:
	// No private variables or methods for this class
	char char_in;                           // Character read from serial device
	uint32_t number_entered;            // Holds a number being entered by user
	bool in_main_motor_module;
	bool in_single_motor_module;
	int16_t local_motor1_power;
	int16_t local_motor2_power;
	int8_t local_motor1_directive;
	int8_t local_motor2_directive;
	uint8_t local_motor_select;
protected:

	// This method displays a simple help message telling the user what to do. It's
	// protected so that only methods of this class or possibly descendents can use it
	void print_help_message (void);
	// This method displays information about the status of the system
	void show_status (void);
	
	bool getUserInput (void);
	void getNumberInput(void);
	

	void printMotorMenu (void);
	void printMainMenu (void);
	void printSingleMotorOptions(void);
	void printDashBoard(void);
	

	void setMotor(uint8_t, uint32_t, int8_t);
	//void setBrake(uint8_t motor_id, uint32_t power);
	//void setFreewheel(uint8_t motor_id, uint32_t power);

public:
	// This constructor creates a user interface task object
	task_user (const char*, unsigned portBASE_TYPE, size_t, emstream*);

	/** This method is called by the RTOS once to run the task loop for ever and ever.
	 */
	void run (void);
	void resetMenus(void);
	bool is_menu_visible;
	bool isValidMotor(uint32_t);
	
};


#endif // _TASK_USER_H_
