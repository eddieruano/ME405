//*****************************************************************************
/** @file task_motor.cpp
 *  @brief     This class is the task motor class that handles a single motor
 *             operation.
 *
 *  @details   This is a task class that will control the operation of
 *             its own motor driver given to it as a pointer and initiated in
 *             the main() function. it lays out the logic necessary to make
 *             commands from task_user to the motor driver happen.
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
//*****************************************************************************

#include "textqueue.h"                 // Header for text queue class
#include "task_motor.h"                // Header for this task
#include "shares.h"                    // Shared inter-task communications


//Set these defines in case we want to change things later.

/// Value for BRAKE directive
#define BRAKE 0
/// Value for SETPOWER directive
#define SETPOWER 1
/// Value for FREEWHEEL directive
#define FREEWHEEL 2
/// Value for POTENTIOMETER directive
#define POTENTIOMETER 3


/**
 * @brief      This constructor builds an instance of a task_motor that will
 *             be used to control a motor unique to the task.
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
 * @param      motor_inc      The motor pointer that this task is tied to,
 *                            since it's only one, each motor needs a task in
 *                            order to operate
 * @param      adc_inc        The A/D converter pointer that is given to the
 *                            task in case of Potentiometer Mode
 * @param[in]  motor_id_inc
 */
task_motor::task_motor (const char* a_name,
                        unsigned portBASE_TYPE a_priority,
                        size_t a_stack_size,
                        emstream* p_ser_dev,
                        motor_driver* motor_inc,
                        adc* adc_inc,
                        uint8_t motor_id_inc
                       )
    : TaskBase (a_name, a_priority, a_stack_size, p_ser_dev)
{
    // Initialize local variables unique to this task.

    /// Need a motor pointer
    motor = motor_inc;
    /// Need an adc pointer
    p_adc = adc_inc;
    /// Need a motor identifier
    motor_identifier = motor_id_inc;
}

/**
 * @brief      This method is called once by the RTOS scheduler.
 * @details    Each time that this method is run it initializes a tickcount and
 *             loops in an infinite for(;;) loop that calls shared variables
 *             that contain the directive, the power, and the selector for the
 *             motor that need to be affected. If the selector matches local
 *             selector assigned by the main function, then the local motor is
 *             affected which in turn affects the motor assigned to it by main.
 */

void task_motor::run (void)
{
    // Make a variable which will hold times to use for precise task scheduling
    TickType_t previousTicks = xTaskGetTickCount ();

    // Configure counter/timer 1 as a PWM for Motor Drivers.
    // COM1A1/COM1B1 to set to non inverting mode.
    // WGM10 to set to Fast PWM mode (only half ughhhhh)
    TCCR1A |= (1 << WGM10) | (1 << COM1A1) | (1 << COM1B1);
    // This is the second Timer/Counter Register
    // WGM12 (other half of Fast PWM)
    // CS11 Sets the presacler to 8 (010)
    TCCR1B |= (1 << WGM12) | (1 << CS11);

    for (;;)
    {
        // creates local variable to reduce the number of calls to
        // 'motor_directive'
        uint8_t LOCAL_motor_directive = motor_directive -> get();
        // if(2000 < (encoder_count -> get()))
        // {
        //     LOCAL_motor_directive = FREEWHEEL;
        // }
        //begin logic checks
        if (motor_identifier == motor_select -> get())
        {
            //if Directive = 0, set the power of the motor.
            if (LOCAL_motor_directive == SETPOWER)
            {
                motor -> set_power(motor_power->get());
            }
            //if Directive = 1, apply the motor brake
            else if (LOCAL_motor_directive == BRAKE)
            {
                motor -> brake(motor_power ->get());
            }
            //if Directive = 2, freewheel the motor
            else if (LOCAL_motor_directive == FREEWHEEL)
            {
                motor -> brake();
            }
            //if the Directive = 3, we enter potentiometer mode
            else if (LOCAL_motor_directive == POTENTIOMETER)
            {
                //create variable to hold the reading of the adc. 1023
                uint16_t duty_cycle = p_adc -> read_once(1);

                //convert the duty cycle into a signed variable and divide by
                int16_t power = ((int16_t)duty_cycle / 4);
                //*p_serial << PMS("Poweer: ") << power<<PMS(". ")<<endl;
                //*p_serial << PMS("PDuty: ") << power << PMS(". ") << endl;
                //if (duty_cycle == 0) {duty_cycle++;}
                if (power < 128)
                {
                    //forwards start from high speed to go to low speed
                    power = 255 - power * 2;
                    motor->set_power(power);
                    motor_power -> put(power);
                }
                else if (power > 128)
                {
                    //backwards start from low speed to go to high speed
                    power = ((128 - power) * 2);

                    motor->set_power(power);
                    motor_power -> put(power);
                }
                //place power in the shared variable in case it need to be accessed by the task_user
                
                //char temp = p_serial->puts("");
                // *p_serial << PMS("Power: ") << power << PMS(". ") << endl;
                //*p_serial << power;
                //OCR3B = duty_cycle;
            }


        }
        // Increment the run counter in the parent class.
        runs++;

        // This is a method we use to cause a task to make one run through its task
        // loop every N milliseconds and let other tasks run at other times
        delay_from_for_ms (previousTicks, 10);
        //delay_ms(5);
    }
}

