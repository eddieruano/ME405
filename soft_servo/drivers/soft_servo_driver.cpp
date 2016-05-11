//*****************************************************************************
/** @file soft_servo_driver.cpp
 *  @brief     This class represents a software servo driver task.
 *
 *  @details   This class creates an instance of a servo motor driver that controls
 *             attached servos in software instead of hardware PWM. This allows it
 *             to be used on any output pin, at the cost of risking precision loss 
 *             if other tasks interfere. This class also contains a nested class
 *             that represents a single servo.
 *
 *  @author Anthony Lombardi
 *
 *  Revisions: @ 5/5/2016 Initial version.
 * 
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
// Include standard library header files
#include <stdlib.h>
#include <avr/io.h>
// Include the interrupt library
#include <avr/interrupt.h>
// Include header for serial port class
#include "rs232int.h"

#include "FreeRTOS.h"                       // FreeRTOS base library
#include "queue.h"                          // FreeRTOS inter-task communication queues

#include "rs232int.h"                       // ME405/507 library for serial comm.
#include "time_stamp.h"                     // Class to implement a microsecond timer
#include "taskbase.h"                       // Header for ME405/507 base task class
#include "taskqueue.h"                      // Header of wrapper for FreeRTOS queues
#include "textqueue.h"                      // Header for a "<<" queue class
#include "taskshare.h"                      // Header for thread-safe shared data

#include "shares.h"                         // Global ('extern') queue declarations

#include "soft_servo_driver.h"

//-----------------------------------------------------------------------------
/** @brief   This is the constructor for a software-driven servo driver task.
 *  @details The constructor sets up the servo driver itself, not the attached servos.
 *  @param a_serial_port Pointer to the serial port that debug statements will go to.
 *  @param a_capacity Attached-servo capacity of the driver. Defaults to 8.
 */
soft_servo_driver::soft_servo_driver (
    emstream* a_serial_port,
    uint8_t a_capacity = 8
)
{
    serial = a_serial_port;
    servo_max = a_capacity;
    servs = new sserv[servo_max];
    servo_ins = 0;
    DBG(serial, "Servo driver ready." << endl);
}

uint8_t soft_servo_driver::attach (
    volatile uint8_t*  a_port,
    uint8_t   a_pin,
    uint16_t  a_duty_cycle = 50,
    uint16_t  a_angle_max = 90,
    uint16_t  a_angle_min = 0
)
{
    *a_port &= ~(1<<a_pin); // set output low
    *(a_port-1) |= (1<<a_pin); // set as output
    
    // store the new servo in the list
    if (servo_ins == servo_max)
    {
        DBG(serial, "Servo attachment limit reached! Try increasing capacity." << endl);
        return -1;
    }
    servs[servo_ins] = sserv(a_port,a_pin,a_duty_cycle,a_angle_max,a_angle_min);
    servo_ins++;
    
    return servo_ins-1; // return the index of the new servo
}

//-------------------------------------------------------------------------------------
/** @brief   This overloaded operator prints information about a software servo driver.
 *  @details It prints out appropriate information about the servo driver being delivered in the parameter.
 *  @param   serpt Reference to a serial port to which the printout will be printed
 *  @param   servo Reference to the servo driver which is being accessed
 *  @return  A reference to the same serial device on which we write information.
 *           This is used to string together things to write with @c << operators
 */
emstream& operator << (emstream &serpt, soft_servo_driver &servo)
{
    // Prints info to serial
    serpt   << PMS ("Software Servo Driver Says hi with ") << servo.get_count() 
            << PMS (" of ") << servo.get_max() << PMS ("servos attached.") << endl;
    
    return (serpt);
}

