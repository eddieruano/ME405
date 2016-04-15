/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** bitbot.h
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


/* -------------------------------------------------------------------------*/
/** @brief   
 *  @details .
 *  @param   
 *  @return  
 */

void bitbot::clear_reg (volatile uint8_t*)
{
    
}

/* -------------------------------------------------------------------------*/
/** @brief   
 *  @details .
 *  @param   
 *  @return  
 */

void bitbot::clear_bit (volatile uint8_t*, uint8_t)
{
    
}
/* -------------------------------------------------------------------------*/
/** @brief   
 *  @details .
 *  @param   
 *  @return  
 */

void bitbot::set_reg (volatile uint8_t*)
{
    
}
/* -------------------------------------------------------------------------*/
/** @brief   
 *  @details .
 *  @param   
 *  @return  
 */

void bitbot::set_bit (volatile uint8_t*, uint8_t)
{
    
}
/* -------------------------------------------------------------------------*/
/** @brief   
 *  @details .
 *  @param   
 *  @return  
 */

void bitbot::set_input_DDR (volatile uint8_t*, uint8_t)
{
    
}

/* -------------------------------------------------------------------------*/
/** @brief   
 *  @details .
 *  @param   
 *  @return  
 */

void bitbot::set_output_DDR (volatile uint8_t*, uint8_t)
{
    
}
/* -------------------------------------------------------------------------*/
/** @brief   
 *  @details .
 *  @param   
 *  @return  
 */

void bitbot::say_hello (void)
{
    *the_serial << PMS ("Hello World.") <<endl;
    cout<< "Hello World";
    return true;
}



