#ifndef _MIRF_H_
#define _MIRF_H_

#include <avr/io.h>

// Mirf settings
#define mirf_CH         0x76
#define mirf_PAYLOAD    32
#define mirf_CONFIG     ( (1<<MASK_RX_DR) | (1<<EN_CRC) | (0<<CRCO) )

// Pin definitions for chip select and chip enabled of the MiRF module
#define CE  PD1
#define CSN PB0

// Definitions for selecting and enabling MiRF module
#define mirf_CSN_hi     PORTB |=  (1<<CSN);
#define mirf_CSN_lo     PORTB &= ~(1<<CSN);
#define mirf_CE_hi      PORTD |=  (1<<CE);
#define mirf_CE_lo      PORTD &= ~(1<<CE);

// Public standart functions
extern void mirf_init();
extern void mirf_config();
extern void mirf_send(uint8_t * value, uint8_t len);
extern void mirf_set_RADDR(uint8_t * adr);
extern void mirf_set_TADDR(uint8_t * adr);
extern uint8_t mirf_data_ready();
extern void mirf_get_data(uint8_t * data);

// Public extended functions
extern void mirf_config_register(uint8_t reg, uint8_t value);
extern void mirf_read_register(uint8_t reg, uint8_t * value, uint8_t len);
extern void mirf_write_register(uint8_t reg, uint8_t * value, uint8_t len);

#endif /* _MIRF_H_ */