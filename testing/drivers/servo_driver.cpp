//*****************************************************************************
/** @file imu_driver.cpp
 *  @brief     This class is the imu driver class.
 *
 *  @details   This class creates an instance of a imu assuming caller gives
 *             all the correct and legal addresses and pin values, driver is
 *             able to communicate with IMU via I2C.
 *
 *  @author Eddie Ruano
 *
 *  Revisions: @ 5/4/2016 <<EDD>> created.
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
#include "servo_driver.h"




#define STOP 0
#define CONTINUE 1
/**
 * @brief      This is the constuctor for the imu_driver class.
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
 
servo_driver::servo_driver (

    emstream* serial_PORT_incoming,
    volatile uint8_t* timer_reg_A_inc,
    volatile uint8_t* timer_reg_B_inc,
    volatile uint16_t* ICR_reg_inc,
    volatile uint16_t* OCR_reg_inc,
    uint8_t prescaler_inc,
    uint16_t top_ICR_inc,
    uint8_t OCR_pin_inc
)
{
    //Set locals
    serial_PORT = serial_PORT_incoming;
    local_timer_reg_A = timer_reg_A_inc;
    local_timer_reg_B = timer_reg_B_inc;
    local_ICR_reg = ICR_reg_inc;
    local_OCR_reg = OCR_reg_inc;
    local_prescaler = prescaler_inc;
    local_top_ICR = top_ICR_inc;
    local_OCR_pin = OCR_pin_inc;

    // Do all the cool stuff in this method.

    initializeServo();


    // Print a handy debugging message
    DBG (serial_PORT, "Servo Driver Construced Successfully. " << endl);

}






void servo_driver::initializeServo(void)
{
    /****** Begin alteration of TIMER/COUNTER 3 ******/

    //Power Reduction Register 0 PPR1 (0x65)
    //PPR0   -  |  -  | PRTIM5 |PRTIM4| PRTIM3 | PRUSART3 | PRUSART2 | PRUSART1
    //PPR0 --7--|--6--|---5----|---4--|---3----|----2---  |---1----|---0--
    //PRTIM3
    //Normally set to 1, we need to set it to 0 to enable timer 3

    PRR1 &= ~(1 << PRTIM3);
    //change this to generic

    //  For the next two registers consider this formula:
    //  SCL Freq = (CPU Internal Clock Frequency) / 16 + 2 (TWBR) * 4 ^ TWPS
    //  It sets the SCL

    //  TWSR (TWI Status Register (Prescaler Bits)) -
    //  TWSR - TWS7 | TWS6 | TWS5 | TWS4 | TWS3 | –---- | TWPS1 | TWPS0
    //  TWSR - --7--|---6--|---5--|---4--|---3--|---2---|---1---|---0-----
    //  TWSR - (----------Status Bits-----------)-------(-Prescaler Bits-)
    //  TWSR will be 0x08 after a start condition
    //  The SCL clock should be 100kHz
    
    //No prescaler so clean bottom two bits
    // Configure counter/timer 3 as a PWM for Motor Drivers.
    // COM1A1/COM1B1 to set to non inverting mode.
    *local_timer_reg_A |= (1 << COM3A1) | (1 << COM3B1);
    // This is the second Timer/Counter Register
    // WGM13 only bit we set for freq and phase correct PWM w/ ICR as TOP
    // CS11 Sets the presacler to 8 (010)
    *local_timer_reg_B |= (1 << WGM33) | (1 << CS31);

    //  TWBR (Bit Rate Generator Unit)
    //  TWBR (TWI Bit Rate Register) - will be 2 for a 50kHz SCL
    //  TWBR - TWBR7 | TWBR6 | TWBR5 | TWBR4 | TWBR3 | TWBR2 | TWBR1 | TWBR0
    //  TWBR - --7---|---6---|---5---|---4---|---3---|---2---|---1---|---0----
    //  Set bitrate to either 12 to make SCL got to 400kHz which is max
    //  OR set to 72 and get 100kHz on SCL
    *local_ICR_reg = 20000;
    //*local_OCR_pin = 1500;

    //set pin as output
    DDRE |= (1 << local_OCR_pin);



    // /// now need to initialize and get correct values of the joysticks
    // uint8_t count = 0;
    // uint32_t total = 0;
    // while( count < 20)
    // {
    //     total+= 
    // }
}

void servo_driver::setServoAngle(int16_t angle)
{
    //going to change PWM
    //need to change OCR but keep between 1000 and 2000 for 1ms and 2ms pulses
    *local_OCR_reg = angle;
    return;
}


/**
 * @brief                     This overloaded operator prints information
 *                            about a imu driver.
 *
 * @details                   It prints out appropriate information about the
 *                            imu driver being delivered in the parameter.
 *
 * @param      serpt          A pointer to the serial port used.
 * @param      motdrv         A pointer to a driver object
 *
 * @return                    A reference to the same serial device on which
 *                            we write information. This is used to string
 *                            together things to write with @c << operators
 */

emstream& operator << (emstream& serpt, servo_driver& servodrv)
{
    // Prints info to serial
    serpt << PMS ("Servo Driver Input: ") << endl;


    return (serpt);
}

