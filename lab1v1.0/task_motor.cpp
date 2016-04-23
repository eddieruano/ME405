//**************************************************************************************
/** @file task_motor.cpp
 *    This file contains the code for a task class which controls the brightness of an
 *    LED using a voltage measured from the A/D as input. The fun part: the brightness
 *    that is being controlled can be on another AVR computer, with signals being sent
 *    and received via wireless transceivers.
 *
 *  Revisions:
 *    @li 09-30-2012 JRR Original file was a one-file demonstration with two tasks
 *    @li 10-05-2012 JRR Split into multiple files, one for each task
 *    @li 10-25-2012 JRR Changed to a more fully C++ version with class task_sender
 *    @li 10-27-2012 JRR Altered from data sending task into LED blinking class
 *    @li 11-04-2012 JRR Altered again into the multi-task monstrosity
 *    @li 12-13-2012 JRR Yet again transmogrified; now it controls LED brightness
 *
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

#include "textqueue.h"                      // Header for text queue class
#include "task_motor.h"                // Header for this task
#include "shares.h"                         // Shared inter-task communications



#define BRAKE 0
#define SETPOWER 1
#define FREEWHEEL 2

//-------------------------------------------------------------------------------------
/** This constructor creates a task which controls the brightness of an LED using
 *  input from an A/D converter. The main job of this constructor is to call the
 *  constructor of parent class (\c frt_task ); the parent's constructor the work.
 *  @param a_name A character string which will be the name of this task
 *  @param a_priority The priority at which this task will initially run (default: 0)
 *  @param a_stack_size The size of this task's stack in bytes
 *                      (default: configMINIMAL_STACK_SIZE)
 *  @param p_ser_dev Pointer to a serial device (port, radio, SD card, etc.) which can
 *                   be used by this task to communicate (default: NULL)
 */

task_motor::task_motor (const char* a_name,
                        unsigned portBASE_TYPE a_priority,
                        size_t a_stack_size,
                        emstream* p_ser_dev,
                        motor_driver* motor_inc,
                        uint8_t motor_id_inc
                       )
    : TaskBase (a_name, a_priority, a_stack_size, p_ser_dev)
{
    // Nothing is done in the body of this constructor. All the work is done in the
    // call to the frt_task constructor on the line just above this one
    motor = motor_inc;
    motor_identifier = motor_id_inc;
}


//-------------------------------------------------------------------------------------
/** @ brief This method is called once by the RTOS scheduler.
   @ details Each time around the for (;;) loop, it reads the A/D converter and uses the result to control the brightness of an LED. It also uses the input from the adc object to control the speed and direction of any motor driver object declared.
 */

void task_motor::run (void)
{
    // Make a variable which will hold times to use for precise task scheduling
    TickType_t previousTicks = xTaskGetTickCount ();

    for (;;)
    {
        int8_t LOCAL_motor_directive = motor_directive -> get();
        if (motor_identifier == motor_select -> get())
        {
            if (LOCAL_motor_directive == SETPOWER)
            {
                //*p_serial << PMS("SET POWER")<<endl;
                motor -> set_power(motor_power->get());
            }
            else if (LOCAL_motor_directive == BRAKE)
            {
                //*p_serial << PMS("SET BRAKE")<<endl;
                motor -> brake(motor_power ->get());
            }
            else if (LOCAL_motor_directive == FREEWHEEL)
            {
                //*p_serial << PMS("SET FREE")<<endl;
                motor -> brake();
            }
            else
            {
                //empty on purpose
            }

        }
        else if(motor_select -> get() == 255)
        {
            //reserved for steady state
        }
        else
        {
            //DBG (p_serial, "Error in Motor Selection: "<<motor_select->get()<<"Should be: "<<motor_identifier<<endl);
        }
        //*p_serial << PMS ("Testing inside task")<<endl;
        // Increment the run counter. This counter belongs to the parent class and can
        // be printed out for debugging purposes
        runs++;

        // This is a method we use to cause a task to make one run through its task
        // loop every N milliseconds and let other tasks run at other times
        delay_from_for_ms (previousTicks, 1000);
    }
}

