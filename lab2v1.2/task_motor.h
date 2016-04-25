//*****************************************************************************
/** @file task_motor.h
 *  @brief     This is the header file for the task_motor class. 

 *  @details   It lays out 
 *             prototypes for the task_motor constructor and the run class 
 *             which is constantly called during operation. It also declared 3
 *             local variables that hold a 'motor_driver' pointer, a unsigned 
 *             byte holding the task identifier (1 or 2) which is checked for 
 *             at the beginning of the task logic and a pointer to an 'adc' 
 *             A/D converter class.
 *
 *  @author Eddie Ruano
 *
 *  Revisions: @ 4/20/2016 added main structure
 *             @ 4/22/2016 added pointers and correct logic
 *  License:
 *    This file is copyright 2016 by Eddie Ruano and released under the Lesser 
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
//****************************************************************************

// This define prevents this .h file from being included multiple times in a .cpp file
#ifndef _TASK_MOTOR_H_
#define _TASK_MOTOR_H_

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


//-----------------------------------------------------------------------------
/**
 * @brief      This class is the task motor class that handles a single motor
 *             operation.
 * @details    This is a task class that will control the operation of 
 *             its own motor driver given to it as a pointer and initiated in
 *             the main() function. it lays out the logic necessary to make 
 *             commands from task_user to the motor driver happen.
 */
class task_motor : public TaskBase
{
private:
    /// No private variables or methods for this class
protected:
    /// Holds identifier for this class, if it matches the global motor_select then this entire class and its motor becomes changeable. 
    uint8_t motor_identifier;
    /// This holds the pointer to the motor that this task will control
    motor_driver* motor;
    /// variable pointer that points to the A/D Converter given by main()
    adc* p_adc;

public:
    /// This constructor creates a generic task of which many copies can be made
    task_motor (const char*, unsigned portBASE_TYPE, size_t, emstream*,motor_driver*, adc*, uint8_t);

    /// This method is called by the RTOS once to run the task loop for ever and ever.
    void run (void);
    
};

#endif // _TASK_MOTOR_H_
