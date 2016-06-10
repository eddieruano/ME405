//*****************************************************************************
/** @file task_shift.h
 *  @brief     This is the header file for the 'task_shift' class which
 *             handles the 'encoder_driver' class.
 *
 *  @details   This class declares all the prototypes for all variables and
 *             methods required in task_shift.
 *
 *  @author Eddie Ruano
 *  @author JR Ridgely
 *
 *  Revisions: @li 4/28/2016 FIXED MASSIVE BUG IN ISR
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

// This define prevents this .h file from being included multiple times in a .cpp file
#ifndef _task_shift_H_
#define _task_shift_H_

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

#include "servo_driver.h"                    // Header for the EHTCL Driver


#define CHECK_TIMES
/**
 * @brief      this is the declaration for 'task_shift' which directly
 *             handles the encoder_driver. Only three variables are required
 *             for this class since the ISR is declared in the driver.
 */

class task_shift : public TaskBase
{
private:

protected:
    /// This variable hold the previous count of encoder ticks since the last time the task was called.
    uint16_t last_count;
    /// This is the diffrence between the global count and the local count
    int32_t count_diff;




public:
    ///This is the constructor prototype, added the pointer for an encoder_driver to be passed in by main.
    task_shift (const char*, unsigned portBASE_TYPE, size_t, emstream*, servo_driver*, uint8_t);

    /// This method is called by the RTOS once to run the task loop for ever and ever.
    void run (void);
    /// This is the declaration for the local copy of the servo_driver pointer.
    servo_driver* p_local_servo_driver;

    uint8_t local_channel_select;

    adc* p_local_adc;

    //void initializeJoystick(int16_t);

    uint16_t local_error_adc;

    //void initJoystick(int16_t);


};

#endif // _task_shift_H_
