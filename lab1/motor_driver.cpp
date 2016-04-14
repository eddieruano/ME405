//*************************************************************************************
/** @file motor_driver.cpp
 *    This file contains a very simple A/D converter driver. This driver should be
 *
 *  Revisions: n/a
 *
 //*************************************************************************************/
// Include standard library header files
#include <stdlib.h>                         
#include <avr/io.h>
// Include header for serial port class
#include "rs232int.h"                       
// Include header for the driver class
#include "motor_driver.h" 


//-------------------------------------------------------------------------------------
/** @brief   This is the constuctor for the motor_driver class.
 *  @details This constructor takes in several addresses and delivers them to the protected variables declared in our header file. 
 *  @param   serial_PORT_incoming This holds the address to the incoming data register 
 *  @param   ch The A/D channel which is being read must be from 0 to 7
 *  @param   ch The A/D channel which is being read must be from 0 to 7
 *  @param   ch The A/D channel which is being read must be from 0 to 7
 *  @return  The result of the A/D conversion
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



//-------------------------------------------------------------------------------------
/** @brief   This method sets the appropriate input direction based on the incoming pwm parameter labeled power. 
 *  @param   sig int16_t the incoming signed input that tells our pwm signal going into the Output/Compare how strong the motor will be moving
 *  @return  no return
 */

void motor_driver::set_power (int16_t sig)
{

    //* SETTING THE DATA DIRECTION *//

    // Need to set DDR to outputs for the input port
    *input_DDR |= ((1 << input_APIN) | (1 << input_BPIN));
    // Need to set the diagnostic as an (IN)
    *diag_DDR &= ~(1 << diag_PIN);
    // Need to set the Pulse Width Modulation Data Direction as an (OUT)
    *pwm_DDR |= (1 << pwm_PIN);
    
    // Need to set the pull up resistor on diag port because i don't really know
    *diag_PORT |= (1 << diag_PIN);


    //** NOW SET LOGIC SO MOTOR WORKS W/ POTENTIOMETER **//
    // signal > 0 = forwards!
    // initialize a variable for speed
    int16_t speed = 0;
    if(sig >= 0)
    {
        // set_direction();
        // PINA = HIGH, PINB = LOW
        *input_PORT |= (1 << input_APIN);
        *input_PORT &= ~(1 << input_BPIN);
        // set_speed();
        // put mod signal into speed reg (comparator)
        // make sure value isn't negative
        speed = sig;
        if(speed < 0)
        {
            speed = speed * -1;
        }

        *ocr_PORT = speed;
        DBG (serial_PORT, "FORWARDS! @ "<<speed<<endl);
    }
    else if(sig < 0)
    {
        // set_direction();
        // PINB = HIGH, PINA = LOW
        *input_PORT |= (1 << input_BPIN);
        *input_PORT &= ~(1 << input_APIN);
        // set_speed();
        // put mod signal into speed reg (comparator)
        // make sure value isn't negative
        speed = sig;
        if(speed < 0)
        {
            speed = speed * -1;
        }

        *ocr_PORT = speed;
        DBG (serial_PORT, "BACKWARDS! @ "<<speed<<endl);
    }
    else
    {
        // it should never reach this condition but in case
      DBG (serial_PORT, "Uhhhhh.. Sig problems"<<endl);
    }
    
    DBG (serial_PORT, "input_PORT: " <<*input_PORT <<endl);
    DBG (serial_PORT, "OCR_PORT: " <<*ocr_PORT <<endl);
}
//-------------------------------------------------------------------------------------
/** @brief This method actively brakes by setting both pins high. This is stronger braking than just letting it freewheel
 *  @param brake int16_t is a signal that if true will indicate brakes
 *  @return no return
 */

void motor_driver::brake (int16_t brake)
{   
    //* SETTING THE DATA DIRECTION *//

    // Need to set DDR to outputs for the input port
    *input_DDR |= ((1 << input_APIN) | (1 << input_BPIN));
    // Need to set the diagnostic as an (IN)
    *diag_DDR &= ~(1 << diag_PIN);
    // Need to set the Pulse Width Modulation Data Direction as an (OUT)
    *pwm_DDR |= (1 << pwm_PIN);
    
    // Need to set the pull up resistor on diag port because i don't really know
    *diag_PORT |= (1 << diag_PIN);

    //* SETTING BRAKES *//
    //set_direction();
    // PINA = HIGH, PINB = HIGH
    *input_PORT |= (1 << input_APIN);
    *input_PORT |= (1 << input_BPIN);
    if(brake < 0)
    {
        brake = brake * -1;
    }
    *ocr_PORT = brake;


}

 void motor_driver::brake ()
{ 
  // Need to set DDR to outputs for the input port
    *input_DDR |= ((1 << input_APIN) | (1 << input_BPIN));
    // Need to set the diagnostic as an (IN)
    *diag_DDR &= ~(1 << diag_PIN);
    // Need to set the Pulse Width Modulation Data Direction as an (OUT)
    *pwm_DDR |= (1 << pwm_PIN);
    
    // Need to set the pull up resistor on diag port because i don't really know
    *diag_PORT |= (1 << diag_PIN);

    //* UNSETTING BRAKES *//
    *input_PORT &= ~(1 << input_APIN);
    *input_PORT &= ~(1 << input_BPIN);

}

//------------------
/**
 *
 */
emstream& operator << (emstream& serpt, motor_driver& motdrv)
{
	// Prints info to serial
	serpt << PMS ("I will print useful things. ")<<endl;


	return (serpt);
}

