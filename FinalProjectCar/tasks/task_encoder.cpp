//*****************************************************************************
/** @file task_encoder.cpp
 *  @brief     This is the file for the 'task_encoder' class which handles the
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
#include "task_encoder.h"                   // Header for this task
#include "shares.h"                       // Shared inter-task communications
#include "time_stamp.h"

#define MAX_SPREAD 1700
#define LIMIT 4095

/**
 * @brief      This is the constructor for the task_encoder class.
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

task_encoder::task_encoder (
   const char* a_name,
   unsigned portBASE_TYPE a_priority,
   size_t a_stack_size,
   emstream* p_ser_dev,
   hctl_driver* p_hctl_inc
): TaskBase (a_name, a_priority, a_stack_size, p_ser_dev)
{
   /// Initialize pointer passed from main() to local variable to work with
   p_hctl = p_hctl_inc;
   previous_encoder_count = 0;
   //previous_encoder_difference = 0; 
   this_difference = 0;
   this_count = 0;
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

void task_encoder::run (void)
{
   // Make a variable which will hold times to use for precise task scheduling
   TickType_t previousTicks = xTaskGetTickCount ();

   
   // The loop to continully check the encoder
   while (1)
   { 
      //read value from chip
      this_count = p_hctl -> read();
      // find raw difference w/o taking into account overflow
      this_difference = this_count - previous_encoder_count;

      //fix the difference to account for overflow
      if(my_abs(this_difference) > MAX_SPREAD)
      {
         //an overflow occurred, must find correct ticks difference
         //if this reading is smaller e.g 375 and prev was like 3800 we know
         //we are counting up and overflowed
         if(this_count < previous_encoder_count)
         {
            this_difference = ((LIMIT - previous_encoder_count) + this_count);
         }
         else // we know we are counting down so neg
         {
            this_difference = -1 * ((LIMIT - this_count) + previous_encoder_count);
         }
         //*p_serial <<endl<<endl<<this_difference<<endl<<this_count<<endl<<previous_encoder_count<<endl<<endl;

      }
      // add the correct difference to the encoder_count variable
      encoder_count -> put((encoder_count -> get()) + (int32_t)this_difference);
      //place ticks per MS in this case in the correct shares variable
      encoder_ticks_per_task -> put(this_difference);

      // store the current count as previous
      previous_encoder_count = this_count;


   // Increment the run counter. This counter belongs to the parent class and can
   // be printed out for debugging purposes
   runs++;
   // This is a method we use to cause a task to make one run through its task
   // loop every N milliseconds and let other tasks run at other times
   delay_from_for_ms (previousTicks, 1);
   }
   
}

uint16_t task_encoder::my_abs(int16_t the_number)
{
   if(the_number < 0)
   {
      return ((uint16_t)(the_number * -1));
   }
   else
   {
      return (uint16_t)the_number;
   }
}
