//**************************************************************************************
/** @file task_controller.h
 *    This file contains the header for a task class that reads the inputs of a wireless
 *    controller- two joysticks and a momentary button- and stores them in shared variables.
 *
 *  Revisions:
 *    @li 06-03-2016 ATL Initial version, building off of template tasks.
 *
 *  License:
 *    This file is copyright 2016 by AT Lombardi and released under the Lesser GNU 
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
#ifndef _TASK_CONTROLLER_H_
#define _TASK_CONTROLLER_H_

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

#define THROTTLE_DZH   576
#define THROTTLE_DZL   448
#define STEER_LIMH     1023
#define STEER_LIML     0
#define GEARBOX_THRESH 512

//-------------------------------------------------------------------------------------
/** @brief   This task reads the inputs of two joysticks and a push button, and feeds them
 *           into shared variables.
 *  @details The A/D converter is run using a driver in files @c adc.h and @c adc.cpp.
 *           Code in this task calculates the values of motor power, steering angle, and
 *           high|low gear based on the inputs.
 */

class task_controller : public TaskBase
{
private:
	/// task share pointer for the motor setpoint.
        TaskShare<uint16_t>* p_motor;
        /// task share pointer for the steering setpoint.
        TaskShare<uint16_t>* p_steer;
        /// task share pointer for the gear shift button value.
        TaskShare<uint8_t>* p_gears;

protected:
	// No protected variables or methods for this class

public:
	// This constructor creates the task and connects it to the shares it will use.
	task_controller (
            const char*,
            unsigned portBASE_TYPE,
            size_t, emstream*,
            TaskShare<uint16_t>* p_motor_share,
            TaskShare<uint16_t>* p_steer_share,
            TaskShare<uint8_t>*  p_gears_share
        );

	// This method is called by the RTOS once to run the task loop forever and ever.
	void run (void);
};

#endif // _TASK_CONTROLLER_H_
