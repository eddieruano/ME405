//**************************************************************************************
/** @file task_motor.cpp
 *    This is the task_motor class
 *
 *  Revisions:
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

/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** task_motor.cpp
** Description: This is the task_motor class.
**
** Author: Eddie Ruano
** Date:   April 15, 2015
** -------------------------------------------------------------------------*/

#include "textqueue.h"                    // Header for text queue class
#include "task_motor.h"                   // Header for this task
#include "shares.h"                       // Shared inter-task communications

// Set the define constants to work with set_direction() method
#define DIR_CHANGE -1 //any positive constant works
#define STOP 0 //any negative constant BUT NOT -1
#define SETPOWER 1 // only -1 will make this work
#define FREEWHEEL 2 //only 0 will make this work

/**
 * @brief      This constructor creates a motor that
 *
 * @param[in]  a_name         A character string which is the name of the task.
 *
 * @param[in]  a_priority       The priority at which this task will initially
 *                            run (default: 0).
 * @param[in]  a_stack_size   The size of this task's stack in bytes
 *                           (default: configMINIMAL_STACK_SIZE)
 *
 * @param      p_ser_dev      The pointer to the correct motor driver.
 * @param      motor_driver   The pointer to the correct motor driver.
 */
task_motor::task_motor (const char* a_name,
                        unsigned portBASE_TYPE a_priority,
                        size_t a_stack_size,
                        emstream* p_ser_dev,
                        motor_driver* motor_inc,
                        uint8_t identifier_inc)
   : TaskBase (a_name, a_priority, a_stack_size, p_ser_dev)
{
   // Only sets this param
   identifier = identifier_inc;

   //
   motor = motor_inc;
}


void task_motor::run(void)
{
   *p_serial << PMS ("In Task Motor:")<<endl;
   *p_serial << PMS ("Diag")<<endl 
                << PMS ("ID")<< identifier <<dec<< endl
                << PMS ("ADDRESS OF M1")<< motor <<dec<< endl;


                adc* p_adc = new adc (p_ser_port);

   TickType_t previousTicks = xTaskGetTickCount ();

   for(;;){
      // make sure we can choose between motors
      if (motor_select->get() == identifier)
      {
         //Start setting motor to shared variables.
         //
         if (motor_command->get() == DIR_CHANGE)
         {
            motor->set_direction(motor_direction->get());
         }
         else if (motor_command->get() == STOP)
         {
            motor->brake(brake_power-> get ());
         }
         else if (motor_command->get() == SETPOWER)
         {
            motor->set_power(motor_power-> get ());
         }
         else if (motor_command->get() == FREEWHEEL)
         {
            motor->brake();
         }
         else
         {
            //error occurs
            //DBG (serial_PORT, "ERROR IN MOTOR SETTING" << endl);
         }
      }

      // Increment the run counter. This counter belongs to the parent class and can
      // be printed out for debugging purposes
      runs++;

      // This is a method we use to cause a task to make one run through its task
      // loop every N milliseconds and let other tasks run at other times
      delay_from_for_ms (previousTicks, 1000);
   }
}

