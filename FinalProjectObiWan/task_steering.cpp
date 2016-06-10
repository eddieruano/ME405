//*****************************************************************************
/** @file task_steering.cpp
 *  @brief     This is the file for the 'task_steering' class which handles the
 *             encoder_driver class.
 *
 *  @details   This class is given a pointer to an 'encoder_driver' class
 *             which then handles the initialization of all the registers. It
 *             holds the highest priority since the encoder has valuable
 *             information which we cannot miss. it runs every 10 ms, and
 *             calculates how many ticks have elapsed, giving us a per second
 *             reading.
 *
 *  @author Eddie Ruano
 *  @author JR Ridgely
 *
 *  Revisions: @li 5/3/2016 <<EDD>> INCORPORATE NEW HTCL CHIP CODE
 *             @li 4/28/2016 FIXED MASSIVE BUG IN ISR
 *             @li 4/26/2016 started ISR code alogorithm used is a half wave
 *             checker
 *             @li 09-30-2012 JRR Original file was a one-file demonstration
 *             with two tasks
 *             @li 10-05-2012 JRR Split into multiple files, one for each task
 *             @li 10-25-2012 JRR Changed to a more fully C++ version with
 *             class task_user
 *             @li 11-04-2012 JRR Modified from the data acquisition example
 *             to the test suite
 *             @li 01-04-2014 JRR Changed base class names to TaskBase,
 *             TaskShare, etc.
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
//****************************************************************************
#include "textqueue.h"                    // Header for text queue class
#include "task_steering.h"                   // Header for this task
#include "shares.h"                       // Shared inter-task communications
#include "time_stamp.h"
#include "servo_driver.h"

#define X_ERROR_OFFSET -13
#define Y_ERROR_OFFSET 25
#define RIGHT_HARD_LIMIT 850
#define LEFT_HARD_LIMIT 1930
#define CENTER_ADJUSTMENT -122
//#define CHECK_TIMES 10


/**
 * @brief      This is the constructor for the task_steering class.
 * @details    It initialzes all the variables given to the TaskBase super
 *             class and then initializes the encoder pointer given and the
 *             last_count variable.
 *
 * @param[in]  a_name         TA character string which will be the name of
 *                            this task.
 * @param[in]  a_priority     The priority at which this task will initially
 *                            run (default: 0)
 * @param[in]  a_stack_size   The size of this task's stack in bytes
 *                            (default: configMINIMAL_STACK_SIZE)
 * @param      p_ser_dev      Pointer to a serial device (port, radio, SD
 *                            card, etc.) which can be used by this task to
 *                            communicate (default: NULL)
 * @param      p_encoder_inc  This holds the pointer to the HTCL_driver passed
 *                            in from the main() fucntion
 */

task_steering::task_steering (
   const char* a_name,
   unsigned portBASE_TYPE a_priority,
   size_t a_stack_size,
   emstream* p_ser_dev,
   servo_driver* p_servo_inc,
   uint8_t channel_select_inc

): TaskBase (a_name, a_priority, a_stack_size, p_ser_dev)
{
   /// Initialize pointer passed from main() to local variable to work with
   p_local_servo_driver = p_servo_inc;
   local_channel_select = channel_select_inc;

   initJoystick(local_channel_select);
}


/**
 * @brief      This method is called once by the RTOS scheduler.
 *
 * @details    Each time that this method is run it initializes a tickcount and
 *             loops in an infinite for(;;) loop that compares the last tick
 *             count stored locally with the most current reading of encoder
 *             count which lies in the global TaskShare variable
 *             'encoder_count'. It then waits 10 ms to run again and since it
 *             has the highest priority we know that it will certainly run
 *             every 10ms. This means if we multiply the difference in encoder
 *             counts (local vs. global) and use the constant time difference
 *             we can obtain an estimate of the counts per sec.
 */

void task_steering::run (void)
{
   // Make a variable which will hold times to use for precise task scheduling
   TickType_t previousTicks = xTaskGetTickCount ();

   // The loop to contunially run the motors
   while (1)
   {
      // steering angle is in degrees, with 0 being straight forward
      int16_t adc_reading = 512+(-9*steering_angle->get());
      //adc_reading = adc_reading + local_error_adc;
      int16_t adjust;


      if (adc_reading < 512)
      {
         adjust = (512 - adc_reading) * -1;
      }
      else
      {
         adjust = adc_reading - 512;
      }

      int16_t corrected_value = adc_reading + 1000 + adjust + CENTER_ADJUSTMENT;

      if(corrected_value < RIGHT_HARD_LIMIT)
      {
         corrected_value = RIGHT_HARD_LIMIT;
      }
      else if(corrected_value > LEFT_HARD_LIMIT)
      {
         corrected_value = LEFT_HARD_LIMIT;
      }
      else
      {

      }


      if (local_channel_select == 1)
      {
         x_joystick -> put(corrected_value);
      }

      int16_t adc_y = (int16_t) p_local_adc -> read_once(0);

      y_joystick -> put(adc_y);


      p_local_servo_driver -> setServoAngle(corrected_value);

      steering_power -> put(corrected_value);

      //*p_serial << PMS("corrected_angle: ") << corrected_angle << endl;



      // This is a method we use to cause a task to make one run through its task
      // loop every N milliseconds and let other tasks run at other times
      delay_from_for_ms (previousTicks, 5);
   }



}


void task_steering::initJoystick (int16_t channel_select)
{
   int8_t count = 0; 
   local_error_adc = 0;
   while (count < 10)
   {
      local_error_adc = local_error_adc + (p_local_adc -> read_once(channel_select));
      count++;
   }

   local_error_adc = (512 - (local_error_adc / 10));
   *p_serial << local_channel_select << PMS(" ERROR: ") << local_error_adc << endl;
}


