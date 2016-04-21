//*************************************************************************************
/** @file encoder.cpp
 *    This file contains code for a quadrature encoder handler class.
 *
 *  Revisions:
 *    19 April 2016: Initial draft
 * 
 *  Author(s): Anthony Lombardi
 *
 //*************************************************************************************/
// Include standard library header files
#include <stdlib.h>                         
#include <avr/io.h>
// Include header for serial port class
#include "rs232int.h"                       
// Include header for the class
#include "encoder.h" 


encoder::encoder ( )
{
    // EICRA: [ISC31 | ISC30 | ISC21 | ISC20 | ISC11 | ISC10 | ISC01 | ISC00]
    // external interrupt 3 - 0 sense control bits:
    // n1=0, n0=1: any edge generates asynchronously an interrupt request
    // note: changing ISCn bits can cause interrupts, so first clear Interrupt Enable bit
    // in EIMSK register, then change bit, then write 1 to the Interrupt Flag bit of EIFR 
    // register, before reenabling Interrupt Enable bit.
    
    // EICRB is the same thing, but controls interrupts 7 - 4
    
    // EIMSK: [7 ... 0]
    // bit == interrupt to enable. Set I bit in Status Register SREG to activate interrupts.
    
    // EIFR: [7 ... 0]
    // external interrupt flag register. Cleared with a 1. Don't need this.
    
    /*
    sei(); // globally enable interrupts
    EIMSK &= 0; // clear EIMSK to disable ext interrupts for now
    // set interrupt pins 6 and 7 to interrupt on any logic change
    EICRB = (0 << ISC71) | (1 << ISC70) | (0 << ISC61) | (1 << ISC60);
    EIMSK |= (1 << INT7) | (1 << INT6); // reenable ext interrupts on P6 and P7
    */
    
    // Print a handy debugging message
    DBG (serial_PORT, "Encoder Construced Successfully" << endl);
}

// interrupt function placeholder
void ISR(vec_intA, ISR_NOBLOCK)
{
    
}