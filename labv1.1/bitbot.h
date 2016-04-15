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

// This define prevents this .H file from being included multiple times in a .CPP file
#ifndef BITBOT
#define BITBOT

#include "emstream.h"                       // Header for serial ports and devices
#include "FreeRTOS.h"                       // Header for the FreeRTOS RTOS
#include "task.h"                           // Header for FreeRTOS task functions
#include "queue.h"                          // Header for FreeRTOS queues
#include "semphr.h"                         // Header for FreeRTOS semaphores


//-------------------------------------------------------------------------------------
/** @brief   
 *  @details 
 */

class bitbot
{
    protected:
        /// Pointer to the serial port
        emstream* the_serial;

    public:
        /// The constructor for a bitbot is pretty simple. A single serial port parameter (for debugging purposes), gives you access to all the following methods with a mere pointer and instance of this bot.

        bitbot (emstream*);

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
        /// Say hello
        void say_hello (void);
};


#endif // BITBOT
