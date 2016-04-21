//======================================================================================
/** @file motor_driver.h
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
#ifndef MOTOR_DRIVER 

#define MOTOR_DRIVER

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

class motor_driver
{
   private:
      int8_t motor_direction;
      uint8_t motor_speed;

   protected:
      /// The motor_driver class uses this pointer to the serial port to say hello
      emstream* serial_PORT;
      /// pointer to input DDR register
      volatile uint8_t* input_DDR;
      /// pointer to the data input register
      volatile uint8_t* input_PORT;
      /// pointer to diagnostic register
      volatile uint8_t* diag_DDR;
      /// pointer to the diag input register
      volatile uint8_t* diag_PORT;
      /// pointer to DDR pwm register
      volatile uint8_t* pwm_DDR;
      /// pointer to pwm port
      volatile uint8_t* pwm_PORT;
      /// pointer to the comp register
      volatile uint16_t* ocr_PORT;
      /// pinA value to set the input register
      uint8_t input_APIN;
      /// pinB value to set the input register
      uint8_t input_BPIN;
      /// pin valye to set the diag register
      uint8_t diag_PIN; 
      /// pin value to set the pwm register
      uint8_t pwm_PIN;

      ///set the public constructor and the public methods
    public:
      motor_driver (
         emstream* serial_PORT_incoming,
         volatile uint8_t* input_PORT_incoming,
         volatile uint8_t* diag_PORT_incoming,
         volatile uint8_t* pwm_PORT_incoming,
         volatile uint16_t* ocr_PORT_incoming,              
         uint8_t input_APIN_incoming,
         uint8_t input_BPIN_incoming,
         uint8_t diag_PIN_incoming,
         uint8_t pwm_PIN_incoming    
                  );

   /// prototype for set_power method of motor_driver
   void set_power (int16_t);
   /// prototype for the brake method of motor_driver with parameter
   void brake (int16_t);
   /// prototype for brake method w/o parameter
   void brake (void);
   /// prototype for a status printer that will give us useful information
   const char* status(void);
   /// prototype for a status printer that will give us useful information
   void set_direction(int8_t);
   /// prototype for a status printer that will give us useful information
   void set_pwm(int8_t);

}; // end of class motor_driver


/// This operator prints useful info about particular given motor driver
emstream& operator << (emstream&, motor_driver&);
// closes the removal of the code
#endif

