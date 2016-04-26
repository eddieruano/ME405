//======================================================================================
/** @file encoder_driver.h
 *    This file contains a header file for the motor driver for a regular DC
 *    motor.
 *
 *  Revisions: n/a
 *  Author(s): Eddie Ruano
*/


/*
 * in case this file is included multiple times, we want the preprocessor
 * to remove this code if it already read the define in the next line
 * of code below.
*/
#ifndef _ENCODER

#define _ENCODER

#include "emstream.h"               // Header for serial ports and devices
#include "FreeRTOS.h"               // Header for the FreeRTOS RTOS
#include "task.h"                   // Header for FreeRTOS task functions
#include "queue.h"                  // Header for FreeRTOS queues
#include "semphr.h"                 // Header for FreeRTOS semaphores
//added


//-------------------------------------------------------------------------------------
/** @brief   This class will run the DC motor.
 *  @details This class sets up a driver for a DC motor to be used with the H bridge on the custom Atmega1281 board. It sets up no private variables, and has multiple protected variables. The protected variables include 6 pointers to registers that hold the correct data for the
 *
 */

class encoder_driver
{
private:
   /// No privates
   volatile uint8_t* encoder_DATA_PORT;

protected:
   /// The encoder_driver class uses this pointer to the serial port to say hello
   
   /// pointer to external iterrupt control register
   volatile uint8_t* interrupt_control;
   /// pointer to the data input register
   volatile uint8_t* interrupt_enable_mask;
   /// pointer to diagnostic register
   volatile uint8_t* encoder_DDR_PORT;
   /// pinA value to set the input register
   uint8_t a_directive_pin0;
   uint8_t b_directive_pin0;

   uint8_t a_interrupts_activate;
   uint8_t b_interrupts_activate;

   uint8_t a_set_as_input;
   uint8_t b_set_as_input;

   ///set the public constructor and the public methods
public:
   encoder_driver (
      //enable interrupts
      emstream*,
      volatile uint8_t*,
      volatile uint8_t*,
      volatile uint8_t*,
      uint8_t,
      uint8_t,
      uint8_t,
      uint8_t,
      uint8_t,
      uint8_t
   );

///NO METHODS YET
emstream* serial_PORT;

void sayHello(const char*);

}; // end of class encoder_driver


/// This operator prints useful info about particular given motor driver
emstream& operator << (emstream&, encoder_driver&);
// closes the removal of the code
#endif

