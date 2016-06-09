//Eddie Ruano

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
#include <avr/interrupt.h>
#include "rs232int.h"                       // Include header for serial port class
#include "spi_driver.h"


spi_driver::spi_driver (emstream* p_serial_port)
{
    p_serial = p_serial_port;
    
    DBG (p_serial, "SPI Driver Engaged " << endl);
}

void spi_driver::initializeMaster(void)
{
    // Activate SPI in Power Reduction
    PRR0 &= ~(1 << PRSPI);

    // Set SCK and MOSI and CSN as outputs
    DDRB |= ((1 << PB1) | (1 << PB2) | (1 << PB0));

    //Set CE as outport
    DDRB |= (1 << PD1);

    // Set CSN as HIGH and CE as LOW
    PORTB |= (1 << PB0);
    PORTD &= ~(1 << PD1);

    

    // Enable SPI, set as Master, and Prescaler to 16
    SPCR |= ((1 << SPE) | (1 << MSTR) | (1 << SPR0));
}

char spi_driver::masterTransmit(unsigned char data)
{
    // Place Data in Data in/out register
    SPDR = data;
    while(!(SPSR & (1 << SPIF)))
    {
        //wait here until transmission complete
        //*p_serial << PMS("Master is transmitting.. ") << endl;
    }

    return SPDR;
}

void spi_driver::initializeSlave(void)
{
    // Set Master Input Slave Output to OUTPUT
    DDRB |= (1 << PB3);

    //Enable SPI
    SPCR |= (1 << SPE);
}

volatile uint8_t* spi_driver::slaveReceive(void)
{
    while(!(SPSR & (1 << SPIF)))
    {
        //wait here until reception complete
        //
    }
    return &SPDR;
}


emstream& operator << (emstream& serpt, spi_driver& spidrv)
{
    // Prints info to the serial port
    serpt << PMS ("SPI CONTROL");
    return (serpt);
}


