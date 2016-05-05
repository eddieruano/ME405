//**************************************************************************************
/** @file task_hctl_2000.cpp
 *    This file contains the code for a task class which controls an encoder counter IC
 *    which tracks an attached encoder asynchronously, and provides 16-bit resolution.
**/
//**************************************************************************************

#include "textqueue.h"                      // Header for text queue class
#include "task_hctl_2000.h"                 // Header for this task
#include "shares.h"                         // Shared inter-task communications


//-------------------------------------------------------------------------------------
/** This constructor creates a task which controls reading an encoder counter IC and
 *  prints the data being read to the serial device.
 *  @param a_name A character string which will be the name of this task
 *  @param a_priority The priority at which this task will initially run (default: 0)
 *  @param a_stack_size The size of this task's stack in bytes 
 *                      (default: configMINIMAL_STACK_SIZE)
 *  @param p_ser_dev Pointer to a serial device (port, radio, SD card, etc.) which can
 *                   be used by this task to communicate (default: NULL)
 */

task_hctl_2000::task_hctl_2000 (
  const char* a_name, 
  unsigned portBASE_TYPE a_priority, 
  size_t a_stack_size,
  emstream* p_ser_dev,
  hctl* target
) : TaskBase (a_name, a_priority, a_stack_size, p_ser_dev)
{
	counter = target; // the hctl we're using will be given from main
}


//-------------------------------------------------------------------------------------
/** This method is called once by the RTOS scheduler. Each time around the while(1)
 *  loop, it reads the encoder counter and prints the result.
 */

void task_hctl_2000::run (void)
{
  TickType_t prev = xTaskGetTickCount();
  while (1)
  {
    uint16_t val = counter->read();  
    uint8_t low = counter->get_low();
    uint8_t high = counter -> get_high();
    *p_serial << PMS("Current encoder reading: ") << dec << val << " = " << bin << val << endl;
    *p_serial << PMS("High Bits: ") << dec << high << " = " << bin << high << endl;
    *p_serial << PMS("Low Bits: ") << dec << low << " = " << bin << low << endl;
    delay_from_for_ms(prev, 1);
  }
}