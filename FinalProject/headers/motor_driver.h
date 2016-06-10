//*****************************************************************************
/** @file motor_driver.h
 *  @brief     This is the header file for the 'motor_driver' class
 *
 *  @details   This is a task class that will control the operation of
 *             its own motor driver given to it as a pointer and initiated in
 *             the main() function. it lays out the logic necessary to make
 *             commands from task_user to the motor driver happen.
 *
 *  @author Eddie Ruano
 *
 *  Revisions: @ 4/23/2016 removed bitops.h library
 *             @ 4/16/2016 added bitops.h library for setting unsetting bits
 *             @ 4/12/2016 main structure created
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

/* 
 * in case this file is included multiple times, we want the preprocessor 
 * to remove this code if it already read the define in the next line
 * of code below.
*/
#ifndef MOTOR_DRIVER 

#define MOTOR_DRIVER

#include "emstream.h"                       // Header for serial ports and devices
#include "FreeRTOS.h"                       // Header for the FreeRTOS RTOS
#include "task.h"                           // Header for FreeRTOS task functions
#include "queue.h"                          // Header for FreeRTOS queues
#include "semphr.h"                         // Header for FreeRTOS semaphores


//-------------------------------------------------------------------------------------
/** @brief   This class will run the DC motor.
 *  @details This class sets up a driver for a DC motor to be used with the H bridge on the custom Atmega1281 board. It sets up no private variables, and has multiple protected variables. The protected variables include 6 pointers to registers that hold the correct data for the 
 *  
 */

class motor_driver
{
    private:
        //no private variables needed.
    protected:
        /// The motor_driver class uses this pointer to the serial port to say hello
        emstream* serial_PORT;
        /// pointer to input DDR register
        volatile uint8_t* input_DDR;
        /// pointer to the data input register
        volatile uint8_t* input_PORT;
        /// pointer to diagnostic register
        volatile uint8_t* diag_DDR;
        /// pointer to the diag input register
        volatile uint8_t* diag_PORT;
        /// pointer to DDR pwm register
        volatile uint8_t* pwm_DDR;
        /// pointer to pwm port
        volatile uint8_t* pwm_PORT;
        /// pinA value to set the input register
        uint8_t input_APIN;
        /// pinB value to set the input register
        uint8_t input_BPIN;
        /// pin valye to set the diag register
        uint8_t diag_PIN; 
        /// pin value to set the pwm register
        uint8_t pwm_PIN;

        ///set the public constructor and the public methods
    public:
        motor_driver (
            emstream* serial_PORT_incoming,
            volatile uint8_t* input_PORT_incoming,
            volatile uint8_t* diag_PORT_incoming,
            volatile uint8_t* pwm_PORT_incoming,
            volatile uint16_t* ocr_PORT_incoming,               
            uint8_t input_APIN_incoming,
            uint8_t input_BPIN_incoming,
            uint8_t diag_PIN_incoming,
            uint8_t pwm_PIN_incoming        
                    );
    /// pointer to the comp register
    volatile uint16_t* ocr_PORT;
    /// prototype for set_power method of motor_driver
    void set_power (int16_t);
    /// prototype for the brake method of motor_driver with parameter
    void brake (int16_t);
    /// prototype for brake method w/o parameter
    void brake (void);

}; // end of class motor_driver


/**
 * @brief                     This overloaded operator prints information 
 *                            about a motor driver. 
 *                            
 * @details                   It prints out appropriate information about the 
 *                            motor driver being delivered in the parameter.
 *                            
 * @param      serpt          A pointer to the serial port used.
 * @param      motdrv         A pointer to a driver object
 *
 * @return                    A reference to the same serial device on which
 *                            we write information. This is used to string 
 *                            together things to write with @c << operators
 */
emstream& operator << (emstream&, motor_driver&);
//closes the removal of the code
#endif

