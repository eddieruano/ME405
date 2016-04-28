//**************************************************************************************
/** @file task_brightness.cpp
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
#include "task_brightness.h"                // Header for this task
#include "shares.h"                         // Shared inter-task communications


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

task_brightness::task_brightness (const char* a_name,
                                  unsigned portBASE_TYPE a_priority,
                                  size_t a_stack_size,
                                  emstream* p_ser_dev
                                 )
  : TaskBase (a_name, a_priority, a_stack_size, p_ser_dev)
{
  // Nothing is done in the body of this constructor. All the work is done in the
  // call to the frt_task constructor on the line just above this one
  //motor = motor_inc;
}


//-------------------------------------------------------------------------------------
/** @ brief This method is called once by the RTOS scheduler.
   @ details Each time around the for (;;) loop, it reads the A/D converter and uses the result to control the brightness of an LED. It also uses the input from the adc object to control the speed and direction of any motor driver object declared.
 */

void task_brightness::run (void)
{
  // Make a variable which will hold times to use for precise task scheduling
  TickType_t previousTicks = xTaskGetTickCount ();

  // Create an analog to digital converter driver object and a variable in which to
  // store its output. The variable p_my_adc only exists within this run() method,
  // so the A/D converter cannot be used from any other function or method
  //adc* p_my_adc = new adc (p_serial);

  //**  CREATE THE MOTOR DRIVERS  **//

  // create a pointer to a motor driver object and pass it addresses to PORTC, PORTB, OC1B and Pin Values for PC0, PC2, and PB6 as the PWM


  // create a pointer to a motor driver object and pass it addresses to PORTD, PORTB, OC1A and Pin Values for PD5, PD7, and PB5 as the PWM


  // pin is Port E pin 4, which is also OC3B (Output Compare B for Timer 3)
  TCCR3A |= (1 << WGM30) | (1 << COM3B1) | (1 << COM3B0);

  DDRE = (1 << 4);

  // The CS31 and CS30 bits set the prescaler for this timer/counter to run the
  // timer at F_CPU / 64
  TCCR3B |= (1 << WGM32) | (1 << CS31)  | (1 << CS30);






  for (;;)
  {


    //motor -> set_power(motor_power->get());
    //p_motor2 -> set_power(motor_power->get());


    //OCR1B = duty_cycle;
    // Increment the run counter. This counter belongs to the parent class and can
    // be printed out for debugging purposes
    runs++;

    // This is a method we use to cause a task to make one run through its task
    // loop every N milliseconds and let other tasks run at other times
    delay_from_for_ms (previousTicks, 1000);
  }
}

