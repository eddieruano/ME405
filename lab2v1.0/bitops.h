/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** bitops.h
** Description: This is the header file for our Bit Ops Library
**
** Author: Eddie Ruano
** Date:   April 14, 2015
** -------------------------------------------------------------------------*/

// This define prevents this .H file from being included multiple times in a .CPP file
#ifndef BITOPS
#define BITOPS
//-----------------------------------------------------------------------------

/// Clear an entire register
void clear_reg (volatile uint8_t*);
/// Clear a specific bit in a regsiter
void clear_bit (volatile uint8_t*, uint8_t);
/// Set an entire register
void set_reg (volatile uint8_t*);
/// Set a specific bit in a register
void set_bit (volatile uint8_t*, uint8_t);
/// Set a pin to be an input
void set_input_DDR (volatile uint8_t*, uint8_t);
/// Set a pin to be an output
void set_output_DDR (volatile uint8_t*, uint8_t);


/* -------------------------------------------------------------------------*/
/** @brief  This method clears an entire register.
 *  @details This method clears and entire register given a pointer to the register by ANDING it with 0
 *  @param reg A pointer to a register
 *  @return  
 */

void clear_reg (volatile uint8_t* reg)
{
       //derefrence the "reg" pointer and clear the entire 8 bit register w/ AND ZERO
    *reg &= 0;
}

/* -------------------------------------------------------------------------*/
/** @brief  This method clears a bit in a register.
 *  @details This method clears a bit in a register given a pointer to the register and the pin value location. 
 *  @param reg A pointer to a register
 *  @param pin A value that holds the amount of location of pin
 *  @return no return
 */

void clear_bit (volatile uint8_t* reg, uint8_t pin)
{
    //deref reg and clear a single pin by AND INVERSE of PIN
    *reg &= ~(1 << pin);
}
/* -------------------------------------------------------------------------*/
/** @brief  This method clears a register.
 *  @details This method clears a register given a pointer to the register.
 *  @param reg A pointer to a register
 *  @return no return
 */

void set_reg (volatile uint8_t* reg)
{
    //set an entire register to 1s
    *reg |= 0xFF;
}
/* -------------------------------------------------------------------------*/
/** @brief  This method sets a bit in a register.
 *  @details This method sets a bit in a register given a pointer to the register and the pin value location. 
 *  @param reg A pointer to a register
 *  @param pin A value that holds the amount of location of pin
 *  @return no return
 */

void set_bit (volatile uint8_t* reg, uint8_t pin)
{
    // deref reg and set a 1 using OR EQUALS and shift 1 into pin amount of places right.
    *reg |= (1 << pin);
}
/* -------------------------------------------------------------------------*/
/** @brief  This method sets a pin to be an input by making it ZERO.
 *  @details This method sets a pin to be an input given a DDR register pointer
 *  @param reg A pointer to a DDR register
 *  @param pin A value that holds the amount of location of pin
 *  @return no return
 */

void set_input_DDR (volatile uint8_t* reg, uint8_t pin)
{
    // for input the pin must be set to 0, so we call the clear bit fucntion and pass it the incoming parameters.
    clear_bit(reg, pin);
}

/* -------------------------------------------------------------------------*/
/** @brief  This method sets a pin to be an output by making it ONE.
 *  @details This method sets a pin to be an output given a DDR register pointer
 *  @param reg A pointer to a DDR register
 *  @param pin A value that holds the amount of location of pin
 *  @return no return
 */

void set_output_DDR (volatile uint8_t* reg, uint8_t pin)
{
    // for output the pin must be set to 1, so we call the set bit fucntion and pass it the incoming parameters.
    set_bit(reg,pin);
}
#endif // BITOPS
