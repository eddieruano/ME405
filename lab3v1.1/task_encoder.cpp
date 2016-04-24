/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** task_encoder.cpp
** Description: This is the task_encoder class.
**
** Author: Eddie Ruano
** Date:   April 15, 2015
** -------------------------------------------------------------------------*/

#include "textqueue.h"                    // Header for text queue class
#include "task_encoder.h"                   // Header for this task
#include "shares.h"                       // Shared inter-task communications


/** This constructor creates a task which controls the brightness of an LED using input from an A/D converter. The main job of this constructor is to call the constructor of parent class (\c frt_task ); the parent's constructor the work.
   @param a_name A character string which will be the name of this task
   @param a_priority The priority at which this task will initially run (default: 0)
   @param a_stack_size The size of this task's stack in bytes (default: configMINIMAL_STACK_SIZE)
   @param p_ser_dev Pointer to a serial device (port, radio, SD card, etc.) which can be used by this task to communicate (default: NULL)
 */

task_encoder::task_encoder (
   const char* a_name,
   unsigned portBASE_TYPE a_priority,
   size_t a_stack_size,
   emstream* p_ser_dev,
   encoder_driver* p_encoder_inc
): TaskBase (a_name, a_priority, a_stack_size, p_ser_dev)
{
   p_encoder = p_encoder_inc;
}


//-------------------------------------------------------------------------------------
/** @ brief This method is called once by the RTOS scheduler.
   @ details Each time around the while (1) loop, it reads the A/D converter and uses the result to control the brightness of an LED. It also uses the input from the adc object to control the speed and direction of any motor driver object declared.
 */

void task_encoder::run (void)
{
   // Make a variable which will hold times to use for precise task scheduling
   TickType_t previousTicks = xTaskGetTickCount ();

   // Configure counter/timer 1 as a PWM for Motor Drivers.
   // COM1A1/COM1B1 to set to non inverting mode.
   // WGM10 to set to Fast PWM mode (only half ughhhhh)
   //TCCR1A |= (1 << WGM10) | (1 << COM1A1) | (1 << COM1B1);
   // This is the second Timer/Counter Register
   // WGM12 (other half of Fast PWM)
   // CS11 Sets the presacler to 8 (010)
   //TCCR1B |= (1 << WGM12) | (1 << CS11);

   // Create an analog to digital converter driver object and a variable in which to
   // store its output. The variable p_my_adc only exists within this run() method,
   // so the A/D converter cannot be used from any other function or method
   //adc* p_adc = new adc (p_serial);

   //**  CREATE THE MOTOR DRIVERS  **//

   // create a pointer to a motor driver object and pass it addresses to PORTC, PORTB, OC1B and Pin Values for PC0, PC2, and PB6 as the PWM
   //motor_driver* p_motor1 = new motor_driver(p_serial, &PORTC, &PORTC, &PORTB, &OCR1B, PC0, PC1, PC2, PB6);

   // create a pointer to a motor driver object and pass it addresses to PORTD, PORTB, OC1A and Pin Values for PD5, PD7, and PB5 as the PWM
   //motor_driver* p_motor2 = new motor_driver(p_serial, &PORTD, &PORTD, &PORTB, &OCR1A, PD5, PD6, PD7, PB5);

   // The loop to contunially run the motors
   while (1)
   {
      // *p_serial << PMS("PORTE : ") << PORTE <<bin<< PMS("   : ") << PORTE << dec<<endl;
      // *p_serial << PMS("PE5   : ") << (1<<PE5) <<bin<< PMS("   : ") << (1<<PE5) << dec<<endl;
      // uint8_t temp = PORTE & (1<<PE5);
      // *p_serial << PMS("&     : ") << temp <<bin<< PMS("   : ") << temp <<endl;
      // *p_serial << PMS("=     : ")<<(temp == (1<<PE5)) <<endl;
      // uint8_t temp2 = (temp == (1<<PE5));
      // *p_serial << temp2 << endl;
      *p_serial << PMS ("In Encoder : ") <<encoder_pulses_per_sec->get()<< endl;
      *p_serial << PMS ("STATE : ") <<encoder_previous_state->get()<< endl;

                //<< PMS ("Power_Signal: ") << power << dec << endl;

      // Increment the run counter. This counter belongs to the parent class and can
      // be printed out for debugging purposes
      runs++;

      // This is a method we use to cause a task to make one run through its task
      // loop every N milliseconds and let other tasks run at other times
      delay_from_for_ms (previousTicks, 1000);
   }
}

