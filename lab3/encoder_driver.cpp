//*****************************************************************************
/** @file encoder_driver.cpp
 *  @brief     This class is the encoder driver class.
 *
 *  @details   This class creates an instance of a motor assuming caller gives
 *             all the correct and legal addresses and pin values, driver is
 *             able to map a motor to one of the two H bridge modules on the
 *             Custom ME 405 board.
 *
 *  @author Eddie Ruano
 *
 *  Revisions: @ 4/27/2016 finished fixing bug in ISR
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
// Include standard library header files
#include <stdlib.h>
#include <avr/io.h>
// Include the interrupt library
#include <avr/interrupt.h>
// Include header for serial port class
#include "rs232int.h"
// Include header for the driver class
#include "encoder_driver.h"
#include "queue.h"                          // FreeRTOS inter-task communication queues

#include "rs232int.h"                       // ME405/507 library for serial comm.
#include "adc.h"                            // Header for A/D converter class driver
#include "motor_driver.h"
#include "time_stamp.h"                     // Class to implement a microsecond timer
#include "taskbase.h"                       // Header for ME405/507 base task class
#include "taskqueue.h"                      // Header of wrapper for FreeRTOS queues
#include "textqueue.h"                      // Header for a "<<" queue class
#include "taskshare.h"                      // Header for thread-safe shared data

#include "shares.h"                         // Global ('extern') queue declarations
// Include our Bit Operation Library
//#include "bitops.

// Set the define constants
#define SHIFT_PE6 64                        // 1 shifted left by 6 (64)
#define SHIFT_PE7 128                        // 1 shifted left by 6 (64)

//-----------------------------------------------------------------------------
encoder_driver::encoder_driver (
    //enable interrupts


    emstream* serial_PORT_incoming,
    volatile uint8_t* interrupt_control_inc,
    volatile uint8_t* interrupt_enable_mask_inc,
    volatile uint8_t* encoder_DDR_PORT_inc,
    uint8_t a_directive_pin0_inc,
    uint8_t b_directive_pin0_inc,
    uint8_t a_interrupts_activate_inc,
    uint8_t b_interrupts_activate_inc,
    uint8_t a_set_as_input_inc,
    uint8_t b_set_as_input_inc
)
{
    sei();
    //** SET REGISTER POINTERS **//
    serial_PORT = serial_PORT_incoming;
    interrupt_control = interrupt_control_inc;
    interrupt_enable_mask = interrupt_enable_mask_inc;
    encoder_DDR_PORT = encoder_DDR_PORT_inc;
    a_directive_pin0 = a_directive_pin0_inc;
    b_directive_pin0 = b_directive_pin0_inc;
    a_interrupts_activate = a_interrupts_activate_inc;
    b_interrupts_activate = b_interrupts_activate_inc;
    a_set_as_input = a_set_as_input_inc;
    b_set_as_input = b_set_as_input_inc;

    //ENABLE THE CORRECT REGISTERS
    *interrupt_control_inc |= (1 << a_directive_pin0) | (1 << b_directive_pin0);

    *interrupt_control_inc &= ~(1 << (a_directive_pin0 + 1)) |
                              (1 << (b_directive_pin0 + 1));

    *interrupt_enable_mask |= (1 << a_interrupts_activate) |
                              (1 << b_interrupts_activate);
    *encoder_DDR_PORT &= ~(1 << a_set_as_input) | ~(1 << (b_set_as_input));
    encoder_DATA_PORT = encoder_DDR_PORT + 1;
    // set the pins with pull up resistor so button low
    *encoder_DATA_PORT |= (1 << a_set_as_input) | (1 << (b_set_as_input));


    // Print a handy debugging message
    DBG (serial_PORT, "Encoder Driver Construced Successfully" << endl);
    DBG (serial_PORT, "Interrupts Enabled Successfully" << endl);

}


// Set up ISR for PINE6
ISR(INT6_vect)
{
    //check to see if both square waves have equal outputs
    //this only occurs when B is leading A and the encoder is going CCW
    if ((PINE & (1 << PE6)) == (PINE & (1 << PE7)))
    {
        encoder_count -> ISR_put((encoder_count -> ISR_get()) - 1);
    }
    else
    {
        encoder_count -> ISR_put((encoder_count -> ISR_get()) + 1);
    }

    //the_state -> ISR_put(this_state);
}
// Set up ISR for PINE7
ISR(INT7_vect)
{
    if ((PINE & (1 << PE6)) != (PINE & (1 << PE7)))
    {
        encoder_count -> ISR_put((encoder_count -> ISR_get()) - 1);
    }
    else
    {
        encoder_count -> ISR_put((encoder_count -> ISR_get()) + 1);
    }
}




//-------------------------------------------------------------------------------------
/** @brief   This overloaded operator prints information about a encoder driver.
 *  @details It prints out appropriate information about the encoder driver being delivered in the parameter.
 *  @param   serpt Reference to a serial port to which the printout will be printed
 *  @param   a2d   Reference to the encoder driver which is being accessed
 *  @return  A reference to the same serial device on which we write information.
 *           This is used to string together things to write with @c << operators
 */
emstream& operator << (emstream & serpt, encoder_driver & encdrv)
{
    // Prints info to serial
    serpt << PMS ("Encoder Driver Says Hi.") << endl;


    return (serpt);
}

