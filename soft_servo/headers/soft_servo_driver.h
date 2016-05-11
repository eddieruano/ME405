//*****************************************************************************
/** @file soft_servo_driver.h
 *  @brief     This is the header file for the software servo driver task class.
 *
 *  @details   This is the header for a class that will control the operation of
 *             servo motors using a software PWM task.
 *
 *  @author Anthony Lombardi
 *
 *  Revisions: @ 5/5/2016 Initial version
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

/* 
 * in case this file is included multiple times, we want the preprocessor 
 * to remove this code if it already read the define in the next line
 * of code below.
*/
#ifndef SOFT_SERVO

#define SOFT_SERVO

#include "emstream.h"                       // Header for serial ports and devices
#include "FreeRTOS.h"                       // Header for the FreeRTOS RTOS
#include "task.h"                           // Header for FreeRTOS task functions
#include "queue.h"                          // Header for FreeRTOS queues
#include "semphr.h"                         // Header for FreeRTOS semaphores

//-------------------------------------------------------------------------------------
/** @brief   This class will create a servo motor driver task.
 *  @details This class sets up a driver for software-driven servos.
 *           It stores the servos internally as an array, and addresses them as needed.
 */

class soft_servo_driver
{
    private:
        class sserv
        {
            public:
                volatile uint8_t* port;
                uint8_t  pin;
                uint16_t cycle_prog, dcy, max, min;
                sserv(uint8_t*, uint8_t, uint16_t, uint16_t, uint16_t, uint16_t);
                // represents a single servo object
                sserv (
                    volatile uint8_t* a_port,
                    uint8_t  a_pin,
                    uint16_t a_duty_cycle,
                    uint16_t a_angle_max,
                    uint16_t a_angle_min
                )
                {
                    port = a_port;
                    pin = a_pin;
                    cycle_prog = 0;
                    dcy = a_duty_cycle;
                    max = a_angle_max;
                    min = a_angle_min;
                }
                // parameterless constructor
                sserv ()
                {
                }
        };
    protected:
        /// The servo driver class uses this pointer to the serial port to say hello
        emstream* serial;
        /// The array that stores the list of attached servos.
        sserv* servs;
        /// Tracks the first open index in the array.
        uint8_t servo_ins;
        /// Remembers the maximum size of the array.
        uint8_t servo_max;

        ///set the public constructor and the public methods
    public:
        soft_servo_driver (
            emstream*,
            uint8_t
        );
        
        uint8_t attach (
            volatile uint8_t*,
            uint8_t,
            uint16_t,
            uint16_t,
            uint16_t
        );
        uint8_t get_count () { return servo_ins; }
        uint8_t get_max () { return servo_max; }
        
    /*
    /// pointer to the comp register
    volatile uint16_t* ocr_PORT;
    /// prototype for set_power method of motor_driver
    void set_power (int16_t);
    /// prototype for the brake method of motor_driver with parameter
    void brake (int16_t);
    /// prototype for brake method w/o parameter
    void brake (void);
    */

}; // end of class soft_servo


/**
 * @brief                     This overloaded operator prints information 
 *                            about a software servo driver. 
 *                            
 * @details                   It prints out appropriate information about the 
 *                            servo driver being delivered in the parameter.
 *                            
 * @param      serpt          A pointer to the serial port used.
 * @param      srvdrv         A pointer to a driver object
 *
 * @return                    A reference to the same serial device on which
 *                            we write information. This is used to string 
 *                            together things to write with @c << operators
 */
emstream& operator << (emstream& serpt, soft_servo_driver& srvdrv);
//closes the removal of the code
#endif // SOFT_SERVO

