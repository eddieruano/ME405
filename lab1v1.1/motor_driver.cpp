/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** motor_driver.cpp
** Description: This is the motor driver class.
**
** Author: Eddie Ruano
** Date:   April 7, 2015
** Rev:
**    1.0 -> completed main work.
**    1.1 -> added more varables, special helper library, clean up.
** -------------------------------------------------------------------------*/
// Include standard library header files
#include <stdlib.h>                         
#include <avr/io.h>
// Include header for serial port class
#include "rs232int.h"                       
// Include header for the driver class
#include "motor_driver.h"
// Include our Bit Operation Library
#include "bitops.h"

// Set the define constants to work with set_direction() method
#define FORWARDS 1 //any positive constant works
#define REVERSE -2 //any negative constant BUT NOT -1
#define FREEWHEEL -1 // only -1 will make this work
#define STOP 0 //only 0 will make this work

//-----------------------------------------------------------------------------
/** @brief   This is the constuctor for the motor_driver class.
 *  @details This constructor takes in several addresses and delivers them to the protected variables declared in our header file. 
 *  @param   serial_PORT_incoming This holds the address to the incoming data register 
 *  @param   diag_PORT_incoming This holds the address to the incoming data register
 *  @param   pwm_PORT_incoming This holds the address to the incoming data register
 *  @param   ocr_PORT_incoming This holds the address to the incoming ocr register
 *  @param   input_APIN_incoming This holds defined value for INA pin
 *  @param   input_BPIN_incoming This holds defined value for INB pin
 *  @param   diag_PIN_incoming This holds defined value for appropriate diag pin
 *  @param   pwm_BPIN_incoming This holds defined value for pwm pin
 */

motor_driver::motor_driver (
   emstream* serial_PORT_incoming,
   volatile uint8_t* input_PORT_incoming,
   volatile uint8_t* diag_PORT_incoming,
   volatile uint8_t* pwm_PORT_incoming,
   volatile uint16_t* ocr_PORT_incoming,               
   uint8_t input_APIN_incoming,
   uint8_t input_BPIN_incoming,
   uint8_t diag_PIN_incoming,
   uint8_t pwm_PIN_incoming        
                           )
{
   //** SET REGISTER POINTERS **//

   //set the pointer to incoming serial port to local serial port pointer
   serial_PORT = serial_PORT_incoming;
   //set incoming INPUT port reg ptr to local INPUT port reg ptr
   // e.g. &PORTC
   input_PORT = input_PORT_incoming;
   //set incoming DIAGNOSTIC port reg ptr to local DIAGNOSTIC port reg ptr
   // e.g. &PORTC
   diag_PORT = diag_PORT_incoming;
   //set incoming PWM port reg ptr to local PWM port reg ptr
   // e.g. &PORTB
   pwm_PORT = pwm_PORT_incoming;
   //set incoming OUTPUT COMPARE reg ptr to local OUTPUT COMPARE port reg ptr
   // e.g. &OCR1B
   ocr_PORT = ocr_PORT_incoming;

   //** SET PIN VALUES **//
   input_APIN = input_APIN_incoming;
   input_BPIN = input_BPIN_incoming;
   diag_PIN = diag_PIN_incoming;
   pwm_PIN = pwm_PIN_incoming;

   //** SET THE DATA DIRECTION POINTERS **//
      // PORT A DATA ADDRESS --> 0x22
      // PORT A DDR  ADDRESS --> 0x21
      // PORT B DATA ADDRESS --> 0x25
      // PORT B DDR  ADDRESS --> 0x24
      // PORT C DATA ADDRESS --> 0x28
      // PORT C DDR  ADDRESS --> 0x27
      // PORT D DATA ADDRESS --> 0x2B
      // PORT D DDR  ADDRESS --> 0x2A

   //SET DDR Pointers subtract 1 to get correct address
   input_DDR = (input_PORT_incoming - 1);
   diag_DDR = (diag_PORT_incoming - 1);
   pwm_DDR = (pwm_PORT_incoming - 1);

   // Print a handy debugging message
   DBG (serial_PORT, "Motor Driver Construced Successfully" << endl);
   DBG (serial_PORT, "Printing Addresses... "<<endl);
   DBG (serial_PORT, "input_PORT_ADDRESS: " <<input_PORT<<endl);
   DBG (serial_PORT, "input_DDR_ADDRESS: " <<input_DDR<<endl);
   DBG (serial_PORT, "pwm_PORT_ADDRESS: " <<pwm_PORT<<endl);
   DBG (serial_PORT, "pwm_DDR_ADDRESS: " <<pwm_DDR<<endl);
   DBG (serial_PORT, "Printing Binary Values... "<<endl);
   DBG (serial_PORT, "input_PORT: " <<*input_PORT<<bin<<endl);
   DBG (serial_PORT, "input_DDR: " <<*input_DDR<<bin<<endl);
   DBG (serial_PORT, "pwm_PORT: " <<*pwm_PORT<<bin<<endl);
   DBG (serial_PORT, "pwm_DDR: " <<*pwm_DDR<<bin<<endl);
   DBG (serial_PORT, "END PRINTING MOTOR CONSTRUCTOR." <<endl);
}



//-----------------------------------------------------------------------------
/** @brief   This method sets the appropriate input direction based on the incoming pwm parameter labeled power. 
 *  @param   sig int16_t the incoming signed input that tells our pwm signal going into the Output/Compare how strong the motor will be moving
 *  @return  no return
 */

void motor_driver::set_power (int16_t sig)
{

   //* SETTING THE DATA DIRECTION *//

   // Need to set DDR to outputs for the input port
   // *input_DDR |= ((1 << input_APIN) | (1 << input_BPIN));

   set_output_DDR(input_DDR, input_APIN);
   set_output_DDR(input_DDR, input_BPIN);
   // Need to set the diagnostic as an (IN)
   // *diag_DDR &= ~(1 << diag_PIN);

   set_input_DDR(diag_DDR, diag_PIN);
   // Need to set the Pulse Width Modulation Data Direction as an (OUT)
   // *pwm_DDR |= (1 << pwm_PIN);

   set_output_DDR(pwm_DDR, pwm_PIN);
   // Need to set the pull up resistor on diag port 
   // *diag_PORT |= (1 << diag_PIN);

   set_bit(diag_PORT, diag_PIN);

   //** NOW SET LOGIC SO MOTOR WORKS W/ POTENTIOMETER **//
   // initialize a variable for speed
   int16_t speed = 0;
   if(sig >= 0)
   {
        
      // PINA = HIGH, PINB = LOW
      set_direction(FORWARDS);
      // put mod signal into speed reg (comparator)
      // make sure value isn't negative
      speed = sig;
      set_speed(speed);
      DBG (serial_PORT, "FORWARDS! @ "<<speed<<endl);
        
      }
      else if(sig < 0)
      {
        
         // PINB = HIGH, PINA = LOW
         // *input_PORT |= (1 << input_BPIN);
         // *input_PORT &= ~(1 << input_APIN);
         set_direction(REVERSE);
         // put mod signal into speed reg (comparator)
         // make sure value isn't negative
         speed = sig;
         set_speed(speed);
         DBG (serial_PORT, "BACKWARDS! @ "<<speed<<endl);
      }
      else
      {
         // it should never reach this condition but in case
         DBG (serial_PORT, "IMPOSSIBLE SET POWER CONDITION"<<endl);
      }
      // Print some debugging messages.
      DBG (serial_PORT, "input_PORT: " <<*input_PORT <<endl);
      DBG (serial_PORT, "OCR_PORT: " <<*ocr_PORT <<endl);

}
//-----------------------------------------------------------------------------
/** @brief This method actively brakes by setting both pins high. This is stronger braking than just letting it freewheel
 *  @param brake int16_t is a signal that if true will indicate brakes
 *  @return no return
 */

void motor_driver::brake (int16_t force)
{   
    //* SETTING THE DATA DIRECTION *//

    // Need to set DDR to outputs for the input port
    // *input_DDR |= ((1 << input_APIN) | (1 << input_BPIN));
    set_output_DDR(input_DDR, input_APIN);
    set_output_DDR(input_DDR, input_BPIN);
    // Need to set the diagnostic as an (IN)
    // *diag_DDR &= ~(1 << diag_PIN);
    set_input_DDR(diag_DDR, diag_PIN);
    // Need to set the Pulse Width Modulation Data Direction as an (OUT)
    // *pwm_DDR |= (1 << pwm_PIN);
    set_output_DDR(pwm_DDR, pwm_PIN);
    // Need to set the pull up resistor on diag port because i don't really know
    // *diag_PORT |= (1 << diag_PIN);
    set_bit(diag_PORT, diag_PIN);

    //* SETTING BRAKES *//
    //set_direction();
    // PINA = HIGH, PINB = HIGH
    // *input_PORT |= (1 << input_APIN);
    // *input_PORT |= (1 << input_BPIN);
    set_direction(STOP);

    //control brake dampening **TODO
    if(force < 0)
    {
      force = force * -1;
    }
    *ocr_PORT = force;


}

//-------------------------------------------------------------------------------------
/** @brief This method actively brakes by setting both pins high. This is stronger braking than just letting it freewheel
 *  @param brake int16_t is a signal that if true will indicate brakes
 *  @return no return
 */

 void motor_driver::brake (void)
{ 
      // Need to set DDR to outputs for the input port
      // *input_DDR |= ((1 << input_APIN) | (1 << input_BPIN));
      set_output_DDR(input_DDR, input_APIN);
      set_output_DDR(input_DDR, input_BPIN);
      // Need to set the diagnostic as an (IN)
      // *diag_DDR &= ~(1 << diag_PIN);
      set_input_DDR(diag_DDR, diag_PIN);
      // Need to set the Pulse Width Modulation Data Direction as an (OUT)
      // *pwm_DDR |= (1 << pwm_PIN);
      set_output_DDR(pwm_DDR, pwm_PIN);
      // Need to set the pull up resistor on diag port because i don't really know
      // *diag_PORT |= (1 << diag_PIN);
      set_bit(diag_PORT, diag_PIN);

      //* UNSETTING BRAKES *//
      // *input_PORT &= ~(1 << input_APIN);
      // *input_PORT &= ~(1 << input_BPIN);
      set_direction(FREEWHEEL);

}

//-------------------------------------------------------------------------------------
/** @brief   This overloaded operator prints information about a motor driver. 
 *  @details It prints out appropriate information about the motor driver being delivered in the parameter.
 *  @param   serpt Reference to a serial port to which the printout will be printed
 *  @param   a2d   Reference to the motor driver which is being accessed
 *  @return  A reference to the same serial device on which we write information.
 *           This is used to string together things to write with @c << operators
 */
emstream& operator << (emstream& serpt, motor_driver& motdrv)
{
   // Prints info to serial
   serpt << PMS ("Motor Driver Input: ")<<endl;


   return (serpt);
}


/** @brief Prints useful information about the motor.
 *  @details Prints the local private values (direction and speed) of this motor driver class.
 *  @param no parameters
 *  @return no return
 */
const char* motor_driver::status (void)
{
   // char* information;
   // const char* speed = ((const char*)(*ocr_PORT));
   // char* direction;
   // if(motor_direction > 0)
   // {
   //    direction = "forward";
   // }
   // else if(motor_direction < 0)
   // {
   //    direction = "reverse";
   // }
   // else if(motor_direction == 0)
   // {
   //    direction = "stopped";
   // }
   // else
   // {
   //    direction = "motor direction error";
   // }
   // strcpy(information, "Motor Information: \n");
   // strcat(information, "Direction: ");
   // strcat(information, direction);
   // strcat(information, "\n");
   // strcat(information, "Speed: ");
   // strcat(information, speed);
   // strcat(information, "\n");
   // return information;
   return NULL;
}
/** @brief  This method sets the direction of the motor.
 *  @details This helper method sets the direction of the motor to either
 *  forwards or backwards depending on whther the input is positive or negative
 *  if the value is 0 for the input it'll set both pins to high. if a -1 is 
 *  passed in the it'll set both pins low and enter a freewheel mode.
 *   
 *  @param input This holds a #defined value that will determine the direction
 *  @return no return
 */
void motor_driver::set_direction (int8_t input)
{ 
    //initial condition for fourth case freewheel
    if(input == -1)
    {
        clear_bit(input_PORT, input_APIN);
        clear_bit(input_PORT, input_BPIN);
        // need to return now before the other logic takes place and overwrites this^
        motor_direction = -1;
        return;
    }
    // if input is negative lets set the correct bits to go backwards, so PINA HIGH and PINB LOW
    if(input < 0)
    {
        // forwards, APIN ON, BPIN OFF
        set_bit(input_PORT, input_APIN);
        clear_bit(input_PORT, input_BPIN);
        motor_direction = 1;
    }
    else if(input < 0)
    {
        // reverse, APIN OFF, BPIN ON
        clear_bit(input_PORT, input_APIN);
        set_bit(input_PORT, input_BPIN);
        motor_direction = -2;
    }
    else
    {
        // brakes under damping, BOTH PINS ON
        set_bit(input_PORT, input_APIN);
        set_bit(input_PORT, input_BPIN);
        motor_direction = 0;
    }
}
/** @brief Sets the speed of the motor.
 *  @details This method is a helper method that places the speed parameter into the OCR register.
 *  @param speed The value that was sent into our set_power() method.
 *  @return no return
 */
void motor_driver::set_speed(int8_t speed)
{
    // if the power signal is negative, we set it positive because only positive values can be sent to OCR
    if(speed < 0){ speed *= -1; }

    *ocr_PORT = speed;
}
