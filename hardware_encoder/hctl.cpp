//*************************************************************************************
/** @file hctl.cpp
 *    This file contains an encoder counter driver, for use with the HCTL-2000 IC.
**/
//*************************************************************************************

#include <stdlib.h>                         // Include standard library header files
#include <avr/io.h>
#include <avr/cpufunc.h>

#include "rs232int.h"                       // Include header for serial port class
#include "hctl.h"                            // Include header for the A/D class

//-------------------------------------------------------------------------------------
/** @brief This constructor sets up an HCTL.
 *  @details This constructor takes the pointer and pins given to it and initializes itself. We also store an initial reading (in case the motor moved during startup) and prepare the bus pins. @b Note: The bus and SEL can be shared across several devices, but not OE.
 *  @param p_serial_port A pointer to the serial port which writes debugging info. 
 *  @param a_data_PORT Pointer to the PORT register of the pins we'll use for the data port.
 *  @param a_oe_PORT Pointer to the PORT register for the !OE pin on the IC, which pushes the current count to the bus.
 *  @param a_oe_pin Pin number of the !OE pin on the PORT given.
 *  @param a_sel_PORT Pointer to the PORT register for the SEL pin on the IC, which sets whether we are reading the high or low byte (@b note: low on the line means high byte).
 *  @param a_sel_pin Pin number of the SEL pin on the PORT given.
 */

hctl::hctl (
	emstream* p_serial_port,
	volatile uint8_t* a_data_PORT,
	volatile uint8_t* a_oe_PORT,
	uint8_t a_oe_pin,
 	volatile uint8_t* a_sel_PORT,
	uint8_t a_sel_pin
	)
{
	ptr_to_serial = p_serial_port;

	data_PORT = a_data_PORT;
	oe_PORT = a_oe_PORT;
	oe_pin = a_oe_pin;
	sel_PORT = a_sel_PORT;
	sel_pin = a_sel_pin;
	
	// set the data bus to be inputs
        volatile uint8_t* ddr_port = data_PORT - 1;
        volatile uint8_t* ddr_oe_port = oe_PORT - 1;
        volatile uint8_t* ddr_sel_port = sel_PORT - 1;
        
        
	*ddr_port = 0; // we use all 8 bits of the port, so this is ok.
	*data_PORT = 0;
	
	// set the SEL and OE pins to be outputs, high logic by default
	*oe_PORT  |= (1 << oe_pin);
	*sel_PORT |= (1 << sel_pin);
	*ddr_oe_port  |= (1 << oe_pin);
	*ddr_sel_port |= (1 << sel_pin);
	
	// Print a handy debugging message
	DBG (ptr_to_serial, "HCTL constructor OK" << endl);
}


//-------------------------------------------------------------------------------------
/** @brief   This method takes one encoder count reading and returns it. 
 *  @details This method prepares the bus by writing 0 (high byte) to the SEL pin on the encoder, then drives !OE low to latch the current count to the 8-bit bus. After recording the value, the method releases the !OE pin just long enough to write a 1 (low byte) to the SEL pin, and reads the data again before releasing !OE. 
 *  @return  Encoder_count, the 16 bit returned value of the concatendated High and Low results.
 */

uint16_t hctl::read ()
{
	//result init
	uint16_t Encoder_count = 0;
        volatile uint8_t* data_PIN = data_PORT-2; // the PIN register is always the PORT register - 2 addresses
        
	*sel_PORT &= ~(1 << sel_pin);             // write a 0 to the byte selection (high byte...)
	*oe_PORT  &= ~(1 << oe_pin);              // drop !OE low to latch the bus
	Encoder_count = *data_PIN;                // the input value register is the port register -2 addresses
	highbits = *data_PIN;
	Encoder_count = (Encoder_count << 8);     // move the data to the high byte of the result
        
	*sel_PORT |= (1 << sel_pin);              // move on to the low byte (high input...)
	Encoder_count |= *data_PIN;               // read in the low byte, combine the two bytes
	lowbits = *data_PIN;
	*oe_PORT |= (1 << oe_pin);                // release the bus

	return Encoder_count;
}


uint8_t hctl::get_low (void)
{
    return lowbits;
}

uint8_t hctl::get_high (void)
{
    return highbits;
}
//-------------------------------------------------------------------------------------
/** \brief   This overloaded operator "prints the HCTL." 
 *  \details This prints out the bus pins being used.
 *  @param   serpt Reference to a serial port to which the printout will be printed
 *  @param   hc20   Reference to the hctl-2000 which is being printed
 *  @return  A reference to the same serial device on which we write information.
 */

emstream& operator << (emstream& serpt, hctl& hc20)
{
	// Prints info to the serial port
	serpt << PMS ("This HCTL-2000 is using these pins:")<< dec << endl 
	      << PMS ("OE pin: ")<< hc20.get_oe_port() << ":" << hc20.get_oe_pin() << endl
	      << PMS ("Sharable pins:") << endl
	      << PMS ("SEL pin: ")<< hc20.get_sel_port() << ":" << hc20.get_sel_pin() << endl
	      << PMS ("Data bus port: ") << hc20.get_data_port() << endl;

	return (serpt);
}

