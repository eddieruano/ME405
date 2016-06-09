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

    emstream* serial_PORT_inc,
    volatile uint8_t* interrupt_control_inc,
    volatile uint8_t* interrupt_enable_mask_inc,
    volatile uint8_t* PORT_register_inc,
    uint8_t interrupt_directive_pin_inc,
    uint8_t interrupt_enable_pin_inc,
    uint8_t interrupt_pin_inc
)
{
    // Enable interrupts
    sei();
    // Set locals
    /// Sets serial
    local_serial_port = serial_PORT_inc;
    /// Set the interrupt control registers
    local_interrupt_control = interrupt_control_inc;
    local_interrupt_enable_mask = interrupt_enable_mask_inc;
    /// Sets the PORT pointer and DDR pointer which is 1 less
    local_PORT_register = PORT_register_inc;
    local_DDR_register = local_PORT_register - 1;
    local_interrupt_directive_pin = interrupt_directive_pin_inc;
    local_interrupt_enable_pin = interrupt_enable_pin_inc;
    local_interrupt_pin = interrupt_pin_inc;

    initializeShiftDriver();
}

void shift_driver::initializeShiftDriver(void)
{
    *local_interrupt_control &= ~(1 << local_interrupt_directive_pin);
    *local_interrupt_control |= (1 << local_interrupt_directive_pin + 1);

    *local_interrupt_enable_mask |= (1 << local_interrupt_enable_pin);

    //Enable the pin as an input by setting DDR to 0
    *local_PORT_register &= ~(1 << local_interrupt_pin);
    // Enable internal pull up resistor so pin stays high
    *local_PORT_register |= (1 << local_interrupt_pin);

    // Print Message to acknowledge construction
    DBG (local_serial_port, "Shift Driver Construced Successfully. " << endl);
    return;


}

// Set up ISR for PINE6
ISR(INT5_vect)
{
    int8_t temp_gear_state = gear_state -> ISR_get();
    if(gear_state -> ISR_get() == 1)
    {
        //temp_gear_state = 0;
        gear_state -> ISR_put(0);
    }
    else
    {
        //temp_gear_state = 1;
        gear_state -> ISR_put(1);
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

