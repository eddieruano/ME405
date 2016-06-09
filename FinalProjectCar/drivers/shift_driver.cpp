//*****************************************************************************
/** @file imu_driver.cpp
 *  @brief     This class is the imu driver class.
 *
 *  @details   This class creates an instance of a imu assuming caller gives
 *             all the correct and legal addresses and pin values, driver is
 *             able to communicate with IMU via I2C.
 *
 *  @author Eddie Ruano
 *
 *  Revisions: @ 5/4/2016 <<EDD>> created.
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

// Include standard library header files
#include <stdlib.h>
#include <avr/io.h>
// Include header for serial port class
#include "rs232int.h"
// Include header for the driver class
#include "shift_driver.h"




#define STOP 0
#define CONTINUE 1
/**
 * @brief      This is the constuctor for the imu_driver class.
 * @details    This constructor takes in several addresses and delivers them
 *             to the protected variables declared in our header file.
 *
 * @param      serial_PORT_incoming  This holds the address to the incoming data
 *           register
 * @param      input_PORT_incoming   This holds the address to the incoming
 *                                   data register
 * @param      diag_PORT_incoming    This holds address port to incoming diag
 *                                   port reg
 * @param      pwm_PORT_incoming     This holds the address to the incoming
 *                                   data register
 * @param      ocr_PORT_incoming     This holds the address to the incoming
 *                                   ocr register
 * @param[in]  input_APIN_incoming   This holds defined value for INA pin
 * @param[in]  input_BPIN_incoming   This holds defined value for INB pin
 * @param[in]  diag_PIN_incoming     This holds defined value for appropriate
 *                                   diag ping
 * @param[in]  pwm_PIN_incoming      This holds defined value for pwm pin
 */

shift_driver::shift_driver (

    emstream* serial_PORT_incoming,
    volatile uint8_t* PORT_register_incoming,
    uint8_t PIN_shift_incoming
)
{
    //Set locals
    serial_PORT = serial_PORT_incoming;
    local_PORT_register = PORT_register_incoming;
    local_PIN_shift = PIN_shift_incoming;

    // Do all the cool stuff in this method.

    //local_DDR_register = local_PORT_register - 1;
    //*local_DDR_register &= ~(1 << local_PIN_shift);
    DDRE &= ~(1 << PINE6);
    PORTE |= (1 << PINE6);

    checkForShift();


    // Print a handy debugging message
    DBG (serial_PORT, "Shift Driver Construced Successfully. " << endl);

}






void shift_driver::checkForShift(void)
{
    /****** Begin alteration of TIMER/COUNTER 3 ******/

    //Power Reduction Register 0 PPR1 (0x65)
    //PPR0   -  |  -  | PRTIM5 |PRTIM4| PRTIM3 | PRUSART3 | PRUSART2 | PRUSART1
    //PPR0 --7--|--6--|---5----|---4--|---3----|----2---  |---1----|---0--
    //PRTIM3
    //Normally set to 1, we need to set it to 0 to enable timer 3

    if (!(PORTE & (1 << PINE6)))
    {
        *serial_PORT << PMS("PORT: ") << *local_PORT_register <<endl;
        int8_t current_state = gear_state -> get();
        if (current_state == 1)
        {
            gear_state -> put(0);
        }
        else
        {
            gear_state -> put(1);
        }
    }
}


/**
 * @brief                     This overloaded operator prints information
 *                            about a imu driver.
 *
 * @details                   It prints out appropriate information about the
 *                            imu driver being delivered in the parameter.
 *
 * @param      serpt          A pointer to the serial port used.
 * @param      motdrv         A pointer to a driver object
 *
 * @return                    A reference to the same serial device on which
 *                            we write information. This is used to string
 *                            together things to write with @c << operators
 */

emstream& operator << (emstream& serpt, shift_driver& shiftdrv)
{
    // Prints info to serial
    serpt << PMS ("SHift Driver Input: ") << endl;


    return (serpt);
}

