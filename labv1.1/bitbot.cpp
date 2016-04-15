/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** bitbot.cpp
** Description: This is the header file for our BITBOT helper.
**
** Author: Eddie Ruano
** Date:   April 14, 2015
** -------------------------------------------------------------------------*/

// Include standard library header files
#include <stdlib.h>
#include <avr/io.h>
// Include header for serial port class
#include "rs232int.h" 

/* -------------------------------------------------------------------------*/
/** @brief 
 *  @details 
 */

bitbot::bitbot (emstream* the_serial_incoming)
{
    // Set the serial port addresses to match up
    the_serial = the_serial_incoming;

    // Print a handy debugging message
    DBG (the_serial, "BITBOT is ready." << endl);
}





