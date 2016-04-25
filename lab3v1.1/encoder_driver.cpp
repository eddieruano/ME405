/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** encoder_driver.cpp
** Description: This is the encoder driver class.
**
** Author: Eddie Ruano
** Date:   April 7, 2015
** Rev:
**    1.0 -> completed main work.
**    1.1 -> TODO
** -------------------------------------------------------------------------*/
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
//#include "bitops.h"

// Set the define constants


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
    // 0000 0000
    // 11011111 223 on 6 high & PE6
    // 10011111 159 on 6 low
    encoder_reg -> ISR_put(PORTE);
    encoder_previous_state -> ISR_put(PORTE >> PE6);
    encoder_pulses_per_sec -> ISR_put(encoder_pulses_per_sec->get() + 1);
}
// Set up ISR for PINE7
ISR(INT7_vect)
{
    uint8_t reg = PORTE;
    encoder_reg -> ISR_put(reg);
    encoder_previous_state -> ISR_put(reg >> PE7);
    encoder_pulses_per_sec -> ISR_put(encoder_pulses_per_sec->get() - 1);
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

