//*****************************************************************************
/** @file main.cpp
 *  @brief     This is the file for the 'main()' function. This function 
 *             initialzies everything.
 *
 *  @details   Built on top of JR Ridgely's architecture, this modified main() 
 *             function initializes two motor_driver* objects, an A/D object. 
 *             and various taskShares and tasks. Everything is initialized 
 *             here and then propogated to other classes.
 *
 *  @author Eddie Ruano
 *  @author JR Ridgely
 *
 *  Revisions: @li 4/21/2016 created new motor tasks and gave them the correct 
 *             pointer values.
 *             @li 4/19/2016 added new shared variables
 *             @li 09-30-2012 JRR Original file was a one-file demonstration
 *             with two tasks
 *             @li 10-05-2012 JRR Split into multiple files, one for each task 
 *             plus a main one
 *             @li 10-30-2012 JRR A hopefully somewhat stable version with 
 *             global queue pointers and the new operator used for most memory
 *             allocation
 *             @li 11-04-2012 JRR FreeRTOS Swoop demo program changed to a 
 *             sweet test suite
 *             @li 01-05-2012 JRR Program reconfigured as ME405 Lab 1 starting 
 *             point
 *             @li 03-28-2014 JRR Pointers to shared variables and queues 
 *             changed to references
 *             @li 01-04-2015 JRR Names of share & queue classes changed
               allocated with new now
 *  License:
 *    This file is copyright 2012 by JR Ridgey and released under the Lesser
 *    GNU
 *    Public License, version 2. It intended for educational use only, but its
 *    use
 *    is not limited thereto. */
/*    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 *    IS"
 *    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *    PURPOSE
 *    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *    CONSEQUEN-
 *    TIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 *    GOODS
 *    OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 *    HOWEVER
 *    CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *    LIABILITY,
 *    OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 *    THE USE
 *    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */
//*****************************************************************************
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
#include "task_motor.h"                     // Header for motor_driver task
#include "task_encoder.h"
#include "encoder_driver.h"


// Set all defines
/// Set the initital motor selector to something neutral
#define NULL_MOTER 255



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
TaskShare<uint8_t>* motor_directive;


/// This declares a taskshare which indicates that this variable is a shared
/// variable. This variable will hold the select for each motor.
/// 0 Indicates Motor 0
/// 1 Indicates Motor 1
TaskShare<uint8_t>* motor_select;


/// This variable holds the encoder count and is positive or negative relative to the rotation of the counts accumulated either ClockWise(+), or CounterClockWise (-)
TaskShare<int32_t>* encoder_count;
/// This variable holds the ticks per seconds so that other tasks like task_user may access it.
TaskShare<int32_t>* count_per_sec;
/// This holds the total number of errors detected by the ISR when setting the counts
TaskShare<uint32_t>* encoder_errors;
/// Holds the previous state of the last interrupt called
/// Holds either 0 for 00
///              1 for 01
///              2 for 10
///              3 for 11
TaskShare<uint8_t>* the_state;

TaskShare<uint8_t>* error_state;
TaskShare<int32_t>* error_pos


//=============================================================================
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
    // time_stamp the_time_now;

    // Configure a serial port which can be used by a task to print debugging infor-
    // mation, or to allow user interaction, or for whatever use is appropriate.  The
    // serial port will be used by the user interface task after setup is complete and
    // the task scheduler has been started by the function vTaskStartScheduler()
    rs232* p_ser_port = new rs232 (9600, 1);
    // print this identifier line.
    *p_ser_port << clrscr << PMS ("ME405 Lab 3 Encoder Controller Program") << endl;

    //initialize the A/D converter for potentiometer control
    adc* p_main_adc = new adc (p_ser_port);

    // Create the queues and other shared data items here
    p_print_ser_queue = new TextQueue (32, "Print", p_ser_port, 10);
    motor_directive = new TaskShare<uint8_t> ("Motor Directive");
    motor_power = new TaskShare<int16_t> ("Motor Power");
    motor_select = new TaskShare<uint8_t> ("Motor Select");

    // start encoder variables 
    encoder_count = new TaskShare<int32_t> ("Encoder Pulse Count");
    count_per_sec = new TaskShare<int32_t> ("count/sec");
    encoder_errors = new TaskShare<uint32_t> ("errors");
    error_state = new TaskShare<uint8_t> ("error_state");
    error_pos = new TaskShare<int32_t> ("error_pos");
    the_state = new TaskShare<uint8_t> ("state");

    //initialize to special value so no motor is affected yet
    motor_select -> put(NULL_MOTER);

    //initilaize two different motor driver pointers to pass into two tasks
    motor_driver* p_motor1 = new motor_driver(p_ser_port, &PORTC, &PORTC, &PORTB, &OCR1B, PC0, PC1, PC2, PB6);

    // motor_driver* p_motor2 = new motor_driver(p_ser_port, &PORTD, &PORTD, &PORTB, &OCR1A, PD5, PD6, PD7, PB5);

    // make instance of encoder
    encoder_driver* p_encoder1 = new encoder_driver(p_ser_port, &EICRB, &EIMSK, &DDRE, ISC60, ISC70, INT6, INT7, PE6, PE7);
    // The user interface is at low priority; it could have been run in the idle task
    // but it is desired to exercise the RTOS more thoroughly in this test program
    new task_user ("UserInt", task_priority (1), 260, p_ser_port);

    // Create tasks to control motors, given individual p_motors

    new task_motor ("Motor1", task_priority (2), 280, p_ser_port, p_motor1, p_main_adc, 1);

        // new task_motor ("Motor2", task_priority (3), 280, p_ser_port, p_motor2, p_main_adc, 2);

    //start encoder and give the highest priority
    new task_encoder ("Encoder1", task_priority(5), 280, p_ser_port, p_encoder1);


    // Here's where the RTOS scheduler is started up. It should never exit as long as
    // power is on and the microcontroller isn't rebooted
    vTaskStartScheduler ();
}

