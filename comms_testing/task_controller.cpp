//**************************************************************************************
/** @file task_controller.cpp
 *    This file contains the code for a task class that reads the inputs of a wireless
 *    controller- two joysticks and a momentary button- and stores them in shared variables.
 *
 *  Revisions:
 *    @li 06-03-2016 ATL Initial revision, based on template tasks.
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

#include "textqueue.h"                      // Header for text queue class
#include "task_controller.h"                // Header for this task
#include "shares.h"                         // Shared inter-task communications


//-------------------------------------------------------------------------------------
/** This constructor creates a task which assigns the value of shares based on the
 *  input from an A/D converter. The main job of this constructor is to call the
 *  constructor of parent class (\c frt_task ); the parent's constructor the work.
 *  @param a_name A character string which will be the name of this task
 *  @param a_priority The priority at which this task will initially run (default: 0)
 *  @param a_stack_size The size of this task's stack in bytes 
 *                      (default: configMINIMAL_STACK_SIZE)
 *  @param p_ser_dev Pointer to a serial device (port, radio, SD card, etc.) which can
 *                   be used by this task to communicate (default: NULL)
 */

task_controller::task_controller (const char* a_name, 
	unsigned portBASE_TYPE a_priority, 
	size_t a_stack_size,
	emstream* p_ser_dev,
        TaskShare<uint16_t>* p_motor_share,
        TaskShare<uint16_t>* p_steer_share,
        TaskShare<uint8_t>*  p_gears_share
) : TaskBase (a_name, a_priority, a_stack_size, p_ser_dev)
{
	p_motor = p_motor_share;
        p_steer = p_steer_share;
        p_gears = p_gears_share;
}


//-------------------------------------------------------------------------------------
/** This method is called once by the RTOS scheduler. Each time around the for (;;)
 *  loop, it reads the A/D converter and uses the result to set motor power, steering
 *  angle, and gearbox state.
 */

void task_controller::run (void)
{
	// Make a variable which will hold times to use for precise task scheduling
	TickType_t previousTicks = xTaskGetTickCount ();

	// Create an analog to digital converter driver object and a variable in which to
	// store its output. The variable p_my_adc only exists within this run() method,
	// so the A/D converter cannot be used from any other function or method
	adc* p_my_adc = new adc (p_serial);
        bool gearbox_high = false;

	// This is the task loop for the controller task. This loop runs until the
	// power is turned off or something equally dramatic occurs
	for (;;)
	{
		// Read the A/D converter from channels 0-2
		uint16_t a2d_reading_c0 = p_my_adc->read_once(0);
		uint16_t a2d_reading_c1 = p_my_adc->read_once(1);
                uint16_t a2d_reading_c2 = p_my_adc->read_once(2);
    
                /*
                int16_t speed = 0;
                if (a2d_reading_c0 > THROTTLE_DZH)
                {
                    speed = (int16_t)( (a2d_reading_c0 - THROTTLE_DZH)/4 );
                }
                else if (a2d_reading_c0 < THROTTLE_DZL)
                {
                    speed = -(int16_t)( (THROTTLE_DZL - a2d_reading_c0)/4 );
                }
                
                int16_t steer = 0;
                if (a2d_reading_c1 > STEER_LIMH)
                {
                    steer = STEER_LIMH;
                }
                else if (a2d_reading_c1 < STEER_LIML)
                {
                    steer = STEER_LIML;
                }
                else steer = a2d_reading_c1;
                */
                
                int8_t state = 0;
                if (a2d_reading_c2 > GEARBOX_THRESH)
                {
                    state = 1;
                }
                else state = 0;
                
                p_motor->put(a2d_reading_c0);
                p_gears->put(a2d_reading_c1);
                p_gears->put(a2d_reading_c2);
                
		// Increment the run counter. This counter belongs to the parent class and can
		// be printed out for debugging purposes
		runs++;

		// This is a method we use to cause a task to make one run through its task
		// loop every N milliseconds and let other tasks run at other times
		delay_from_for_ms (previousTicks, 1000);
	}
}

