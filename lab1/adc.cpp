//*************************************************************************************
/** @file adc.cpp
 *    This file contains a very simple A/D converter driver. This driver should be
 *
 *  Revisions:
 *    @li 01-15-2008 JRR Original (somewhat useful) file
 *    @li 10-11-2012 JRR Less original, more useful file with FreeRTOS mutex added
 *    @li 10-12-2012 JRR There was a bug in the mutex code, and it has been fixed
 *
 *  License:
 *    This file is copyright 2015 by JR Ridgely and released under the Lesser GNU 
 *    Public License, version 2. It intended for educational use only, but its use
 *    is not limited thereto. */
/*    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 *    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 *    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 *    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
 *    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUEN-
 *    TIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS 
 *    OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
 *    CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
 *    OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 *    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */
//*************************************************************************************

#include <stdlib.h>                         // Include standard library header files
#include <avr/io.h>

#include "rs232int.h"                       // Include header for serial port class
#include "adc.h"                            // Include header for the A/D class


//-------------------------------------------------------------------------------------
/** @brief This constructor sets up an A/D converter. 
 *  @details This constructor takes the pointer given to it and initializes itself. Also, here we set the voltage refrence and enable the A/D converter as well as set the prescaler to 32, through some bitshifting.
 *  @param p_serial_port A pointer to the serial port which writes debugging info. 
 */

adc::adc (emstream* p_serial_port)
{
	ptr_to_serial = p_serial_port;

	//enables voltage refrence AVCC and sets AREF
	ADMUX |= 1 << REFS0;
	// enable + set the prescaler to 32
	ADCSRA |= ((1<<ADEN)|(1<<ADSC)|(1<<ADPS2)|(1<<ADPS0));


	// Print a handy debugging message
	DBG (ptr_to_serial, "adc successfully constructed. " << endl);
	DBG (ptr_to_serial, "ADMUX: "  <<bin<<ADMUX<<endl);
	DBG (ptr_to_serial, "ADCSRA: " <<bin<<ADCSRA<<endl);
}


//-------------------------------------------------------------------------------------
/** @brief   This method takes one A/D reading from the given channel and returns it. 
 *  @details Given a channel, this method first checks to make sure only valid channels are accepted by setting any outside parameter to channel 0. Then we set the correct register to enable the A/D and use a while loop to wait until the completion bit is 0. At that point, the loop breaks and the results stored in ADCL and ADCH are concatenated into the 16 bit return value.
 *  @param   ch The A/D channel which is being read must be from 0 to 7
 *  @return  ADC_value, the 16 bit returned value of the concatendated High and Low results.
 */

uint16_t adc::read_once (uint8_t ch)
{
	//result init
	uint16_t ADC_value = 0;

	//channel entry valid check
	if(ch > 7){ ch = 0; }
	//clear the mux reg we're sure that it starts at 000
	ADMUX &= ~(7<<MUX0);
	//set the mux
	ADMUX |= ch<<MUX0;
	//begin the converstion
	ADCSRA |= 1<<ADSC; 
	// Wait until ADSC is zero
	while (1) 
	{
		if(!(ADCSRA & 1<<ADSC))
		{
			break;
		}
	}

	//combine the high and low bits using casting technique shown in class
	ADC_value = ((uint16_t) ADCL | (uint16_t) ADCH<<8);
	return ADC_value;
}


//-------------------------------------------------------------------------------------
/** @brief   This method takes multiple readings specified by an input parameter from a particular channel and return the average of those readings.
 *  @param   channel this parameter specifies which channel we're going to be averaging
 *  @param   samples this specifies how many samples we're going to be taking and serves as the basis for our average division.
 *  @return  average_result, this is the average value read over that channel per those samples.
 */

uint16_t adc::read_oversampled (uint8_t channel, uint8_t samples)
{
	// DBG (ptr_to_serial, "All your readings are belong to us" << endl);

	// 16 bits for the average result 
	uint16_t total_result =0;
	uint16_t average_result =0;

	//setting the limit to not overflow our 16 bit variable
	if(samples > 64){ samples = 64; }

	//sum the values of each channel read over the entire sample area
    for(uint8_t i=0; i < samples; i++)
    {
    	// Read and add
		total_result += read_once(channel); 
	}

	//calculate the average
	average_result = total_result / samples;
	return average_result;
	
}


//-------------------------------------------------------------------------------------
/** \brief   This overloaded operator "prints the A/D converter." 
 *  \details The precise meaning of print is left to the user to interpret; it should 
 *           be something useful. For example, one might print the values of the 
 *           configuration registers or show current readings from all the A/D 
 *           channels.
 *  @param   serpt Reference to a serial port to which the printout will be printed
 *  @param   a2d   Reference to the A/D driver which is being printed
 *  @return  A reference to the same serial device on which we write information.
 *           This is used to string together things to write with @c << operators
 */

emstream& operator << (emstream& serpt, adc& a2d)
{
	// Prints info to the serial port
	serpt << PMS ("Current Channel Readings: ")<<endl
		
		  << PMS ("End Print Data")<<endl;



	return (serpt);
}

