//*************************************************************************************
/** \file main.cpp
 *    This file contains the main() code for a program which runs the ME405 board for
 *    ME405 lab 1. This program currently uses obfuscated code to use an A/D converter
 *    to convert an analog signal into an LED brightness via pulse width modulation.
 *    \b This \b comment \b is \b quite \b messed \b up \b and \b the \b student
 *    \b should \b fix \b it \b so \b that \b it \b accurately \b reflects \b the
 *    \b function \b of \b the \b program \b which \b is \b handed \b in \b for \b this
 *    \b assignment.
 *
 *  Revisions:
 *    \li 09-30-2012 JRR Original file was a one-file demonstration with two tasks
 *    \li 10-05-2012 JRR Split into multiple files, one for each task plus a main one
 *    \li 10-30-2012 JRR A hopefully somewhat stable version with global queue
 *                       pointers and the new operator used for most memory allocation
 *    \li 11-04-2012 JRR FreeRTOS Swoop demo program changed to a sweet test suite
 *    \li 01-05-2012 JRR Program reconfigured as ME405 Lab 1 starting point
 *    \li 03-28-2014 JRR Pointers to shared variables and queues changed to references
 *    @li 01-04-2015 JRR Names of share & queue classes changed; allocated with new now
 *
 *  License:
 *		This file is copyright 2015 by JR Ridgely and released under the Lesser GNU
 *		Public License, version 2. It intended for educational use only, but its use
 *		is not limited thereto. */
/*		THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *		AND	ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * 		IMPLIED 	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * 		ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * 		LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUEN-
 * 		TIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * 		OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * 		CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * 		OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * 		OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */
//*************************************************************************************


#include <stdlib.h>                         // Prototype declarations for I/O functions
#include <avr/io.h>                         // Port I/O for SFR's
#include <avr/wdt.h>                        // Watchdog timer header
#include <string.h>                         // Functions for C string handling

#include "FreeRTOS.h"                       // Primary header for FreeRTOS
#include "task.h"                           // Header for FreeRTOS task functions
#include "queue.h"                          // FreeRTOS inter-task communication queues
#include "croutine.h"                       // Header for co-routines and such

#include "rs232int.h"                       // ME405/507 library for serial comm.
#include "time_stamp.h"                     // Class to implement a microsecond timer
#include "taskbase.h"                       // Header of wrapper for FreeRTOS tasks
#include "textqueue.h"                      // Wrapper for FreeRTOS character queues
#include "taskqueue.h"                      // Header of wrapper for FreeRTOS queues
#include "taskshare.h"                      // Header for thread-safe shared data
#include "shares.h"                         // Global ('extern') queue declarations
#include "task_brightness.h"                // Header for the data acquisition task
#include "task_user.h"                      // Header for user interface task
#include "task_motor.h"                                            // 


// Declare the queues which are used by tasks to communicate with each other here.
// Each queue must also be declared 'extern' in a header file which will be read
// by every task that needs to use that queue. The format for all queues except
// the serial text printing queue is 'frt_queue<type> name (size)', where 'type'
// is the type of data in the queue and 'size' is the number of items (not neces-
// sarily bytes) which the queue can hold

/** This is a print queue, descended from \c emstream so that things can be printed
 *  into the queue using the "<<" operator and they'll come out the other end as a
 *  stream of characters. It's used by tasks that send things to the user interface
 *  task to be printed.
 */
TextQueue* p_print_ser_queue;


/// This declares a taskshare which indicates that this variable is a shared 
/// variable. This variable will hold the duty cycle of a motor. 
TaskShare<int16_t>* motor_power;

/// This declares a taskshare which indicates that this variable is a shared 
/// variable. This variable will hold the command that the user wishes the motor
/// to do.
/// -1 Indicates CHANGE_DIRECTION
///  0 Indicates STOP W/ brake_power
///  1 Indicates SET POWER
///  2 Indicates FREEWHEEL
TaskShare<int8_t>* motor_directive;


/// This declares a taskshare which indicates that this variable is a shared 
/// variable. This variable will hold the select for each motor.
/// 0 Indicates Motor 0
/// 1 Indicates Motor 1
TaskShare<uint8_t>* motor_select;


//=====================================================================================
/** The main function sets up the RTOS.  Some test tasks are created. Then the
 *  scheduler is started up; the scheduler runs until power is turned off or there's a
 *  reset.
 *  @return This is a real-time microcontroller program which doesn't return. Ever.
 */

int main (void)
{
	// Disable the watchdog timer unless it's needed later. This is important because
	// sometimes the watchdog timer may have been left on...and it tends to stay on
	MCUSR = 0;
	wdt_disable ();

	   // Configure counter/timer 1 as a PWM for Motor Drivers. 
   // COM1A1/COM1B1 to set to non inverting mode. 
   // WGM10 to set to Fast PWM mode (only half ughhhhh)
   TCCR1A |= (1 << WGM10) | (1 << COM1A1) | (1 << COM1B1);
   // This is the second Timer/Counter Register
   // WGM12 (other half of Fast PWM) 
   // CS11 Sets the presacler to 8 (010)
   TCCR1B |= (1 << WGM12) | (1 << CS11);

	// Configure a serial port which can be used by a task to print debugging infor-
	// mation, or to allow user interaction, or for whatever use is appropriate.  The
	// serial port will be used by the user interface task after setup is complete and
	// the task scheduler has been started by the function vTaskStartScheduler()
	rs232* p_ser_port = new rs232 (9600, 1);
	*p_ser_port << clrscr << PMS ("ME405 Lab 2 Motor Controller Program") << endl;

	// Create the queues and other shared data items here
	p_print_ser_queue = new TextQueue (32, "Print", p_ser_port, 10);

	motor_directive = new TaskShare<int8_t> ("Motor Directive");
    motor_power = new TaskShare<int16_t> ("Motor Power");
    motor_select = new TaskShare<uint8_t> ("Motor Select");

    motor_select -> put(255);

    motor_driver* p_motor1 = new motor_driver(p_ser_port, &PORTC, &PORTC, &PORTB, &OCR1B, PC0, PC1, PC2, PB6);

    motor_driver* p_motor2 = new motor_driver(p_ser_port, &PORTD, &PORTD, &PORTB, &OCR1A, PD5, PD6, PD7, PB5);


    *p_ser_port << motor_power->get()<<endl;
    *p_ser_port << motor_directive->get()<<endl;
    *p_ser_port << motor_select->get()<<endl;


	// The user interface is at low priority; it could have been run in the idle task
	// but it is desired to exercise the RTOS more thoroughly in this test program
	new task_user ("UserInt", task_priority (1), 260, p_ser_port);

	// Create a task which reads the A/D and adjusts an LED's brightness accordingly
	
	new task_motor ("Motor1", task_priority (2), 280, p_ser_port, p_motor1, 1);

    new task_motor ("Motor2", task_priority (2), 280, p_ser_port, p_motor2, 2);

	// Here's where the RTOS scheduler is started up. It should never exit as long as
	// power is on and the microcontroller isn't rebooted
	vTaskStartScheduler ();
}

