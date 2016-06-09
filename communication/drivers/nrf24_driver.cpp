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
#include <util/delay.h>
#include "rs232int.h"                       // Include header for serial port class
#include "nrf24_driver.h"
#include "spi_driver.h"
#include "nRF24L01.h"

#define W 1
#define R 0




nrf24_driver::nrf24_driver (emstream* p_serial_port)
{
    p_serial = p_serial_port;

    // Create SPI driver
    local_spi_driver = new spi_driver(p_serial);
    local_spi_driver -> initializeMaster();
    // Make CE and IRQ outputs
    //DDRD |= ((1 << PD0) | (1 << PD1));

    // Enable Pull up Resisitor
    //PORTD |= ((1 << PD0) | (1 << PD1));

}


uint8_t nrf24_driver::readRegister(uint8_t target)
{
    // Drop CSN to low
    PORTB &= ~(1 << PB0);
    // set the nRF to reading mode "target" is read back
    local_spi_driver -> masterTransmit(R_REGISTER + target);
    //*p_serial << PMS("Reading Reg: ") << target << endl;
    target = local_spi_driver -> masterTransmit(NOP);
    // Set CSN back HIGH
    PORTB |= (1 << PB0);
    //*p_serial << PMS("DATA in SPDR: ") << SPDR << endl;
    return target;


}

uint8_t* nrf24_driver::writeRegister(uint8_t ReadWrite, uint8_t target, uint8_t* payload, uint8_t payload_size)
{


    if (ReadWrite == W)
    {
        target = W_REGISTER + target;
    }

    static uint8_t ret[32];

    // Drop CSN to low
    PORTB &= ~(1 << PB0);
    // set the NRF24 to read or write
    local_spi_driver -> masterTransmit(target);

    uint8_t count;
    for (count = 0; count < payload_size; count++)
    {
        if (ReadWrite == R && target != W_TX_PAYLOAD)
        {
            // Send dummies to read data
            ret[count] = local_spi_driver -> masterTransmit(NOP);
            //*p_serial << "Getting Data || " << count << " || " << ret[count]<< endl;

        }
        else
        {
            // send bytes one at a time
            local_spi_driver -> masterTransmit(payload[count]);
        }
    }
    // Set CSN back HIGH
    PORTB |= (1 << PB0);

    return ret;


}


void nrf24_driver::initialize(void)
{
    //Array of ints to send
    uint8_t val[32];

    //Enable auto ack, only works if TRANS has identical RF_ADDRESS on it's channel ex: RX_ADDR_P0 = TX_ADDR
    val[0] = 0x3F; // set a value
    writeRegister(W, EN_AA, val, 1);

    //Choose # of enabled data pipes (1-5)
    val[0] = 0x03;
    writeRegister(W, EN_RXADDR, val, 1);

    //RF_ADDRESS width setup (how many bytes is receiver address)
    val[0] = 0x03;
    writeRegister(W, SETUP_AW, val, 1);

    //RF_CHANNEL 0x01 = 2401 GHZ steps of 1 GHZ (I NEED 118 or 0x76)
    val[0] = 0x76;
    writeRegister(W, RF_CH, val, 1);

    //RF_SETUP
    val[0] = 0x07;
    writeRegister(W, RF_SETUP, val, 1);


    //RX RF_ADDRESS SETUP set reciever address
    val[0] = 0xF0;
    val[1] = 0xF0;
    val[2] = 0xF0;
    val[3] = 0xF0;
    val[4] = 0xF0;

    writeRegister(W, RX_ADDR_P0, val, 5);

    // TX RF_ADDRESS
    val[0] = 0xF0;
    val[1] = 0xF0;
    val[2] = 0xF0;
    val[3] = 0xF0;
    val[4] = 0xF0;

    writeRegister(W, TX_ADDR, val, 5);

    // SET PAYLOAD WIDTH SIZE
    val[0] = 0x04;
    writeRegister(W, RX_PW_P0, val, 1);

    //RETRIES
    val[0] = 0x2F;
    writeRegister(W, SETUP_RETR, val, 1);

    // CONFIGGGGG
    // [0b0001 1110] (0 --> 0) TRANS, (0 --> 1) REC
    val[0] = 0x7f;
    writeRegister(W, CONFIG, val, 1);




}
void nrf24_driver::recPayload(uint8_t size, uint8_t * container)
{
    uint8_t count;
    // Drop CSN to low
    PORTB &= ~(1 << PB0);
    // request the payload
    *p_serial << "PUT INTO RX" << hex << local_spi_driver -> masterTransmit(R_RX_PAYLOAD) << endl;
    //get payload
    for (count = 0; count < size; count++)
    {

        //container[count] = local_spi_driver -> masterTransmit(NOP);
        //*p_serial << hex << container[count] << endl;
        
        
        container[count] = local_spi_driver -> masterTransmit(NOP);
        *p_serial << count << " || " <<container[count]<< endl;
        //*p_serial << "FIFO: " << readRegister(FIFO_STATUS) << endl;


    }
    // Set CSN back HIGH
    PORTB |= (1 << PB0);

    //reset();
    //*p_serial << PMS("DATA in SPDR: ") << SPDR << endl;
}
void nrf24_driver::reset(void)
{
    //Clear the CSN to LOW
    PORTB &= ~(1 << PB0);
    local_spi_driver -> masterTransmit(W_REGISTER + STATUS);
    // reset IRQ
    local_spi_driver -> masterTransmit(0x0E);
    // Set CSN back to HIGH
    PORTB |= (1 << PB0);
}


void nrf24_driver::printNRF(emstream* p_ser_port, nrf24_driver* p_nrf24)
{
    uint8_t *data;
    data[0] = 0;

    data = p_nrf24 -> writeRegister(R, STATUS, data, 1);
    *p_ser_port << "STATUS\t\t: " << hex << data[0] << endl;

    data = p_nrf24 -> writeRegister(R, CONFIG, data, 1);
    *p_ser_port << "CONFIG\t\t: " << hex << data[0] << endl;

    data = p_nrf24 -> writeRegister(R, EN_AA, data, 1);
    *p_ser_port << "EN_AA\t\t: " << hex << data[0] << endl;

    data = p_nrf24 -> writeRegister(R, EN_RXADDR, data, 1);
    *p_ser_port << "EN_RXADDR\t: " << hex << data[0] << endl;

    data = p_nrf24 -> writeRegister(R, SETUP_AW, data, 1);
    *p_ser_port << "SETUP_AW\t: " << hex << data[0] << endl;

    data = p_nrf24 -> writeRegister(R, SETUP_RETR, data, 1);
    *p_ser_port << "SETUP_RETR\t: " << hex << data[0] << endl;

    data = p_nrf24 -> writeRegister(R, RF_CH, data, 1);
    *p_ser_port << "RF_CH\t\t: " << hex << data[0] << endl;

    data = p_nrf24 -> writeRegister(R, RF_SETUP, data, 1);
    *p_ser_port << "RF_SETUP\t: " << hex << data[0] << endl;

    data = p_nrf24 -> writeRegister(R, CD, data, 1);
    *p_ser_port << "POWER_RECEIVED\t: " << hex << data[0] << endl;

    data = p_nrf24 -> writeRegister(R, RX_ADDR_P0, data, 5);
    *p_ser_port << "RX_ADDR_P0\t: " << hex << data[0] << " " << data[1] << " " << data[2] << " " << data[3] << " " << data[4] << " " << endl;

    data = p_nrf24 -> writeRegister(R, RX_ADDR_P1, data, 5);
    *p_ser_port << "RX_ADDR_P1\t: " << hex << data[0] << " " << data[1] << " " << data[2] << " " << data[3] << " " << data[4] << " " << endl;

    data = p_nrf24 -> writeRegister(R, TX_ADDR, data, 5);
    *p_ser_port << "TX_ADDR\t\t: " << hex << data[0] << " " << data[1] << " " << data[2] << " " << data[3] << " " << data[4] << " " << endl;

    data = p_nrf24 -> writeRegister(R, RX_PW_P0, data, 6);
    *p_ser_port << "RX_PW_P0-6\t: " << hex << data[0] << " " << data[1] << " " << data[2] << " " << data[3] << " " << data[4] << " " << endl;

    // data = p_nrf24 -> writeRegister(R, RX_PW_P1, data, 5);
    // *p_ser_port << "RX_PW_P1\t: " << hex << data[0] << " " << data[1] << " " << data[2] << " " << data[3] << " " << data[4] << " " << endl;

    data = p_nrf24 -> writeRegister(R, FIFO_STATUS, data, 5);
    *p_ser_port << "FIFO_STATUS\t: " << hex << data[0] << endl;
}

emstream& operator << (emstream& serpt, nrf24_driver& nrfdrv)
{
    // Prints info to the serial port
    serpt << PMS ("SPI CONTROL");
    return (serpt);
}

