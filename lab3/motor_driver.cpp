//*****************************************************************************
/** @file motor_driver.cpp
 *  @brief     This class is the motor driver class. 
 *
 *  @details   This class creates an instance of a motor assuming caller gives 
 *             all the correct and legal addresses and pin values, driver is
 *             able to map a motor to one of the two H bridge modules on the 
 *             Custom ME 405 board.
 *
 *  @author Eddie Ruano
 *
 *  Revisions: @ 4/23/2016 removed bitops.h library
 *             @ 4/16/2016 added bitops.h library for setting unsetting bits
 *             @ 4/12/2016 main structure created
 *  License:
 *    This file is copyright 2016 by Eddie Ruano and released under the Lesser
 *    GNU
 *    Public License, version 2. It intended for educational use only, but its
 *    use
 *    is not limited thereto. */
/*    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 *    IS"
 *    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *    PURPOSE
 *    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *    CONSEQUEN-
 *    TIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 *    GOODS
 *    OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 *    HOWEVER
 *    CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *    LIABILITY,
 *    OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 *    THE USE
 *    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */
//*****************************************************************************

// Include standard library header files
#include <stdlib.h>                         
#include <avr/io.h>
// Include header for serial port class
#include "rs232int.h"                       
// Include header for the driver class
#include "motor_driver.h" 

/**
 * @brief      This is the constuctor for the motor_driver class.
 * @details    This constructor takes in several addresses and delivers them 
 *             to the protected variables declared in our header file.
 *
 * @param      serial_PORT_incoming  This holds the address to the incoming data 
 *           register 
 * @param      input_PORT_incoming   This holds the address to the incoming 
 *                                   data register
 * @param      diag_PORT_incoming    This holds address port to incoming diag
 *                                   port reg
 * @param      pwm_PORT_incoming     This holds the address to the incoming 
 *                                   data register
 * @param      ocr_PORT_incoming     This holds the address to the incoming 
 *                                   ocr register
 * @param[in]  input_APIN_incoming   This holds defined value for INA pin
 * @param[in]  input_BPIN_incoming   This holds defined value for INB pin
 * @param[in]  diag_PIN_incoming     This holds defined value for appropriate 
 *                                   diag ping
 * @param[in]  pwm_PIN_incoming      This holds defined value for pwm pin
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
    // DBG (serial_PORT, "Printing Addresses... "<<endl);
    // DBG (serial_PORT, "input_PORT_ADDRESS: " <<input_PORT<<endl);
    // DBG (serial_PORT, "input_DDR_ADDRESS: " <<input_DDR<<endl);
    // DBG (serial_PORT, "pwm_PORT_ADDRESS: " <<pwm_PORT<<endl);
    // DBG (serial_PORT, "pwm_DDR_ADDRESS: " <<pwm_DDR<<endl);
    // DBG (serial_PORT, "Printing Binary Values... "<<endl);
    // DBG (serial_PORT, "input_PORT: " <<*input_PORT<<bin<<endl);
    // DBG (serial_PORT, "input_DDR: " <<*input_DDR<<bin<<endl);
    // DBG (serial_PORT, "pwm_PORT: " <<*pwm_PORT<<bin<<endl);
    // DBG (serial_PORT, "pwm_DDR: " <<*pwm_DDR<<bin<<endl);
    // DBG (serial_PORT, "END PRINTING MOTOR CONSTRUCTOR." <<endl);
}

/**
 * @brief      This method sets the appropriate input direction based on the 
 *             incoming pwm parameter labeled power. 
 *
 * @param[in]  sig   the incoming signed input that tells our pwm signal going
 *                   into the Output/Compare how strong the motor will be
 *                   moving
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
        //DBG (serial_PORT, "FORWARDS! @ "<<speed<<endl);
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
        //DBG (serial_PORT, "BACKWARDS! @ "<<speed<<endl);
    }
    else
    {
        // it should never reach this condition but in case
     // DBG (serial_PORT, "Uhhhhh.. Sig problems"<<endl);
    }
    
    //DBG (serial_PORT, "input_PORT: " <<*input_PORT <<endl);
    //DBG (serial_PORT, "OCR_PORT: " <<*ocr_PORT <<endl);
}

//-----------------------------------------------------------------------------
/**
 * @brief      This method actively brakes by setting both pins high. This is 
 *             stronger braking than just letting it freewheel
 *
 * @param[in]  f_brake  The f brake is a signal that will indicate brake power
 */
void motor_driver::brake (int16_t f_brake)
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
    if(f_brake < 0)
    {
        f_brake = f_brake * -1;
    }


}

//-----------------------------------------------------------------------------

 /**
  * @brief      This method sets both H Drive pins low to let the motor
  *             freewheel.
  */
 void motor_driver::brake (void)
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

/**
 * @brief                     This overloaded operator prints information 
 *                            about a motor driver. 
 *                            
 * @details                   It prints out appropriate information about the 
 *                            motor driver being delivered in the parameter.
 *                            
 * @param      serpt          A pointer to the serial port used.
 * @param      motdrv         A pointer to a driver object
 *
 * @return                    A reference to the same serial device on which
 *                            we write information. This is used to string 
 *                            together things to write with @c << operators
 */

emstream& operator << (emstream& serpt, motor_driver& motdrv)
{
	// Prints info to serial
	serpt << PMS ("Motor Driver Input: ")<<*motdrv.ocr_PORT<<endl;


	return (serpt);
}

