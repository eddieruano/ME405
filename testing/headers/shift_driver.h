//*****************************************************************************
/** @file shift_driver.h
 *  @brief     This is the header file for the 'shift_driver' class
 *
 *  @details   This is a task class that will control the operation of
 *             its own imu driver given to it as a pointer and initiated in
 *             the main() function. it lays out the logic necessary to make
 *             commands from task_user to the imu driver happen.
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
#ifndef SHIFT_DRIVER_H

#define SHIFT_DRIVER_H

#include "emstream.h"                       // Header for serial ports and devices
#include "FreeRTOS.h"                       // Primary header for FreeRTOS
#include "task.h"                           // Header for FreeRTOS task functions
#include "queue.h"                          // FreeRTOS inter-task communication queues

#include "taskbase.h"                       // ME405/507 base task class
#include "time_stamp.h"                     // Class to implement a microsecond timer
#include "taskqueue.h"                      // Header of wrapper for FreeRTOS queues
#include "taskshare.h"                      // Header for thread-safe shared data

#include "rs232int.h"                       // ME405/507 library for serial comm.
#include "adc.h"                            // Header for A/D converter driver class
#include "textqueue.h"                 // Header for text queue class
#include "shares.h"                    // Shared inter-task communications





/** @brief   This class will setup the imu.
 *  @details This class sets up a driver for an imu Adafruit Bosch BNO055
 *
 */

class shift_driver
{
private:
    //no private variables needed.
protected:
    /// The shift_driver class uses this pointer to the serial port to say hello
    emstream* local_serial_port;
    /// Set the interrupt control registers
    volatile uint8_t* local_interrupt_control;
    volatile uint8_t* local_interrupt_enable_mask;
    /// Sets the PORT pointer and DDR pointer which is 1 less
    volatile uint8_t* local_PORT_register;
    volatile uint8_t* local_DDR_register;
    uint8_t local_interrupt_directive_pin;
    uint8_t local_interrupt_enable_pin;
    uint8_t local_interrupt_pin;



    ///set the public constructor and the public methods
public:
shift_driver (
    emstream*,
    volatile uint8_t*,
    volatile uint8_t*,
    volatile uint8_t*,
    uint8_t,
    uint8_t,
    uint8_t
);

    void initializeShiftDriver(void);



}; // end of class shift_driver


/**
 * @brief                     This overloaded operator prints information
 *                            about a imu driver.
 *
 * @details                   It prints out appropriate information about the
 *                            imu driver being delivered in the parameter.
 *
 * @param      serpt          A reference to the emstream used.
 * @param      shift_driver   A reference to a shift_driver object
 *
 * @return                    A reference to the same serial device on which
 *                            we write information. This is used to string
 *                            together things to write with @c << operators
 */
emstream& operator << (emstream&, shift_driver&);
//closes the removal of the code
#endif

