// #include "spi.h"
// #include <avr/io.h>
// #include <avr/interrupt.h>

// #define PORT_SPI    PORTB
// #define DDR_SPI     DDRB
// #define DD_MISO     DDB3
// #define DD_MOSI     DDB2
// #define DD_SS       DDB0
// #define DD_SCK      DDB1



// extern "C" void spi_init()
// // Initialize pins for spi communication
// {
//     PRR0 &= ~(1 << PRSPI);

//     // Set SCK and MOSI and CSN as outputs
//     DDRB |= ((1 << PB1) | (1 << PB2) | (1 << PB0));

//     //Set CE as outport
//     DDRB |= (1 << PD1);

//     // Set CSN as HIGH and CE as LOW
//     PORTB |= (1 << PB0);
//     PORTD &= ~(1 << PD1);
//     //SPSR = (1<<SPI2X);              // Double Clock Rate
//     // Enable SPI, set as Master, and Prescaler to 16
//     SPCR |= ((1 << SPE) | (1 << MSTR) | (1 << SPR0));
// }

// extern "C" void spi_transfer_sync (uint8_t * dataout, uint8_t * datain, uint8_t len)
// // Shift full array through target device
// {

//        uint8_t i;      
//        for (i = 0; i < len; i++) {
//              SPDR = dataout[i];
//              while((SPSR & (1<<SPIF))==0);
//              datain[i] = SPDR;
//        }
//  mirf_CSN_hi;
// }

// extern "C" void spi_transmit_sync (uint8_t * dataout, uint8_t len)
// // Shift full array to target device without receiving any byte
// {
//     // Place Data in Data in/out register
//     SPDR = data;
//     while(!(SPSR & (1 << SPIF)))
//     {
//         //wait here until transmission complete
//         //*p_serial << PMS("Master is transmitting.. ") << endl;
//     }

//     return SPDR;
// }

// extern "C" uint8_t spi_fast_shift (uint8_t data)
// // Clocks only one byte to target device and returns the received one
// {

//     SPDR = data;
//     while((SPSR & (1<<SPIF))==0);
//     return SPDR;

// }