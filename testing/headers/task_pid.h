//*****************************************************************************
/** @file task_pid.h
 *  @brief     This is the header file for the task_pid class.

 *  @details   This header declares the variables and functions used by the 
 *             PID task, implemented in task_pid.cpp. This includes two pointers
 *             to the input and outputs of the control loop, control loop constants,
 *             anti-windup, and maximum and minimum saturation points to be applied
 *             to the output.
 *
 *  @verbatim
 *                              ,-----[KW]<------------------------anti-windup---,
 *                             _v_                                              _|_
 *                            / - \                                            /   \
 *                       ,-->(+    )--err_sum-->[KI]-,               ,------->(+   -)<----------,
 *                       |    \_-_/                  |               |         \___/            |
 *                       |      ^                    |               |                          |
 *                       |      `---{err_sum}        |               |  _____________           |
 *            ___        |                          _v_              |  |       ___  |          |     #######
 *           /   \       |                         / + \             |  |      /     |          |    #       #
 * setpoint>(+    )--err-+------------>[KP]------>(+    )--err_total-+->|     /      |--output--+--> # plant #
 *           \_-_/                                 \_+_/                |    /       |               #       #
 *             ^                                     ^                  | __/        |                #######
 *             |            ___                      |                  |____________|                   |
 *             |           /   \                     |                                                   |
 *             +--------->(+    )--err_deriv-->[KD]--`                                                   |
 *             |           \_-_/                                                                         |
 *             |             ^                                                                           |
 *             |             `--{old_act}                                                                |
 *             |                                                                                         |
 *             |_________________________________________________________________________________________|
 *
 *  @endverbatim
 *  @author Anthony Lombardi
 *
 *  Revisions:  @li @ 5/5/2016 Now uses C.Refvem's satmath library, and Ks are stored as *1024.
                @li @ 5/4/2016 Initial version.
 *  License:
 *    This file is copyright 2016 by Anthony Lombardi and released under the Lesser 
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
#ifndef _TASK_PID_H_
#define _TASK_PID_H_

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
#include "satmath.h"                        // Simple saturated math library.

//-----------------------------------------------------------------------------
/**
 * @brief      This class is a PID controller that can regulate any 16-bit signed value
 *             given a 16-bit setpoint and a set of control loop constants.
 * @details    This is a task class that will control the operation of a
 *             Proportional-Integral-Derivative control loop system using two
 *             16-bit shared values as the input and output. It runs through the loop
 *             every 10 milliseconds and handles integrator windup, as well as 
 *             maximum and minimum value saturation.
 */
class task_pid : public TaskBase
{
private:
    /// No private variables or methods for this class
protected:
    /// Stores a pointer to the shared variable storing the setpoint for the control loop
    TaskShare<int16_t>* setpoint;
    /// Stores a pointer to the shared variable storing the system feedback input
    TaskShare<int16_t>* feedback;
    /// Stores a pointer to the shared variable that serves as the loop output
    TaskShare<int16_t>* output;
    /// stores the previous loop's actual value, for derivative control
    int16_t old_act;
    /// stores the cumulative error, for integral control
    int16_t err_sum;
    /// counters the integral when saturation is hit
    int16_t windup;
    /// proportional constant, stored as 1024x KP for increased accuracy.
    int16_t KP;
    /// integral constant, stored as 1024x KI for increased accuracy.
    int16_t KI;
    /// derivative constant, stored as 1024x KD for increased accuracy.
    int16_t KD;
    /// windup constant, stored as 1024x KW for increased accuracy.
    int16_t KW;
    /// saturation floor
    int16_t MIN;
    /// saturation ceiling
    int16_t MAX;

public:
    /// This constructor takes the default task argument set, plus two pointers (setpoint and output), all the control loop constants (multiplied by 1024) to be used, and two saturation limits.
    task_pid (const char*, unsigned portBASE_TYPE, size_t, emstream*,
    TaskShare<int16_t>*, // p_setpoint,
    TaskShare<int16_t>*, // p_feedback,
    TaskShare<int16_t>*, // p_output,
    int16_t = 1024,      // a_kp,
    int16_t = 1024,      // a_ki,
    int16_t = 1024,      // a_kd,
    int16_t = 1024,      // a_kw,
    int16_t = -32768,    // a_min,
    int16_t = 32767      //a_max
    );

    /// This method is called by the RTOS once to run the task loop forever and ever.
    void run (void);
    
};

#endif // _TASK_PID_H_
