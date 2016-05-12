//*****************************************************************************
/** @file task_pid.cpp
 *  @brief     This class implements a generic PID controller that works on
 *             16-bit values and features anti-windup and saturation points.
 *
 *  @details   This is a task class that will iterate through a Proportional,
 *             Integral, Derivative control loop using a 16-bit signed setpoint
 *             and output value, control loop constants, and saturation points.
 *             The task sleeps for 10 milliseconds between evaluations of the
 *             control loop.
 *
 *  @author Anthony Lombardi
 *
 *  Revisions:  @li @ 5/5/2016 Now uses C.Refvem's satmath library, and Ks are stored as *1024.
 *              @li @ 5/4/2016 Initial version.
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
//*****************************************************************************

#include "textqueue.h"                 // Header for text queue class
#include "task_pid.h"                  // Header for this task
#include "shares.h"                    // Shared inter-task communications
#include "task_pid.h"

/**
 * @brief      This constructor builds an instance of a PID controller.
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
 * @param      p_setpoint     The pointer to the controller target value.
 * @param      p_feedback     The pointer to the system feedback input.
 * @param      p_output       The pointer to the value output by the controller.
 * @param      a_kp           Proportional gain constant multiplied by 1024. Defaults to 1024 (Kp = 1).
 * @param      a_ki           Integral gain constant multiplied by 1024. Defaults to 1024 (Ki = 1).
 * @param      a_kd           Derivative gain constant multiplied by 1024. Defaults to 1024 (Kd = 1).
 * @param      a_kw           Integrator anti-windup gain constant multiplied by 1024. Defaults to 1024 (Kw = 1).
 * @param      a_min          Saturation limit minimum. Defaults to -32768.
 * @param      a_max          Saturation limit minimum. Defaults to 32767.
 */

task_pid::task_pid (
    const char* a_name,
    unsigned portBASE_TYPE a_priority,
    size_t a_stack_size,
    emstream* p_ser_dev,
    TaskShare<int16_t>* p_setpoint,
    TaskShare<int32_t>* p_feedback,
    TaskShare<int16_t>* p_output,
    int16_t a_kp,
    int16_t a_ki,
    int16_t a_kd,
    int16_t a_kw,
    int16_t a_min,
    int16_t a_max
) : TaskBase (a_name, a_priority, a_stack_size, p_ser_dev)
{
    //DBG (serial_PORT, "" << endl);
    setpoint = p_setpoint;
    feedback = p_feedback;
    output = p_output;
    KP = a_kp;
    KI = a_ki;
    KD = a_kd;
    KW = a_kw;
    MIN = a_min;
    MAX = a_max;
}

/**
 * @brief      This method is called once by the RTOS scheduler.
 * @details    Each time that this method is run it initializes a tickcount and
 *             loops in an infinite for(;;) that evaluates the PID control loop
 *             and then sleeps for 10 milliseconds.
 */

void task_pid::run (void)
{
    // Make a variable which will hold times to use for precise task scheduling
    TickType_t previousTicks = xTaskGetTickCount ();

    for (;;)
    {
        int16_t ref = setpoint->get();
        //*p_serial << PMS("SetPoint: ") << ref << endl;
        int16_t act = feedback->get();
        // proportional
        int16_t err = ref - act;
        // integral
        err_sum = sssub( ssadd(err, err_sum), ssdiv(ssmul(KW, windup), 1024) );
        // derivative
        int16_t err_deriv = act - old_act;
        old_act = act;

        // since the Ks are *1024, we divide by that after multiplying each error by its constant and summing them.
        *p_serial << PMS("SSMUL KI ERRSUM: ") << ssmul(KI, err_sum) <<endl;
        *p_serial << PMS("SSMUL KD ERR_D: ") << ssmul(KD, err_sum) <<endl;
        *p_serial << PMS("SSADD ^^: ") << 
            ssadd(
                ssdiv(ssmul(KI,err_sum),1024),
                ssdiv(ssmul(KD,err_deriv),1024)
            ) <<endl;
        *p_serial << PMS("SSMUL KP ERR: ") << ssmul(KP, err) <<endl;
        *p_serial << PMS("SSADD ^^: ") << 
            ssadd(
                ssdiv(ssmul(KP,err),1024),
                ssadd(
                    ssdiv(ssmul(KI,err_sum),1024),
                    ssdiv(ssmul(KD,err_deriv),1024)
                )
            ) <<endl;
        int16_t err_tot = 
            ssadd(
                ssdiv(ssmul(KP,err),1024),
                ssadd(
                    ssdiv(ssmul(KI,err_sum),1024),
                    ssdiv(ssmul(KD,err_deriv),1024)
                )
            );
        *p_serial << PMS("Error: ") <<err_tot <<endl;

        int16_t out;
        // saturation limits
        if (err_tot > MAX)
        {
            out = MAX;
        }
        else if (err_tot < MIN)
        {
            out = MIN;
        }
        else out = err_tot;

        windup = sssub(err_tot, out);

        output->put(out);;;
        *p_serial << PMS ("OUTPUT: ") << out << endl;

        // This is a method we use to cause a task to make one run through its task
        // loop every N milliseconds and let other tasks run at other times
        delay_from_for_ms (previousTicks, 1000);
    }
}

