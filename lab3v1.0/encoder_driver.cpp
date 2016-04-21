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
// Include our Bit Operation Library
#include "bitops.h"

// Set the define constants 


//-----------------------------------------------------------------------------


encoder_driver::encoder_driver (
  //enable interrupts
  sei();

   emstream* serial_PORT_incoming,
   volatile uint8_t* interrupt_control,
   volatile uint8_t* interrupt_enable,
   volatile uint8_t* temp,
   uint8_t a_pin_enable,
   uint8_t b_pin_enable,
   
           
                           )
{
   //** SET REGISTER POINTERS **//


   // Print a handy debugging message
   DBG (serial_PORT, "Encoder Driver Construced Successfully" << endl);
   DBG (serial_PORT, "Interrupts Enabled Successfully" << endl);

}


// Set up ISR for PINB6
ISR(vector_a)
{
  while(1){}
}
// Set up ISR for PINB7
ISR()
{
  while(1){}
{


//-------------------------------------------------------------------------------------
/** @brief   This overloaded operator prints information about a encoder driver. 
 *  @details It prints out appropriate information about the encoder driver being delivered in the parameter.
 *  @param   serpt Reference to a serial port to which the printout will be printed
 *  @param   a2d   Reference to the encoder driver which is being accessed
 *  @return  A reference to the same serial device on which we write information.
 *           This is used to string together things to write with @c << operators
 */
emstream& operator << (emstream& serpt, encoder_driver& encdrv)
{
   // Prints info to serial
   serpt << PMS ("Encoder Driver Says Hi.")<<endl;


   return (serpt);
}

