//*****************************************************************************
/** @file encoder_driver.h
 *  @brief     This is the header file for the encoder driver class. 
 *
 *  @details   This class sets up the prototypes for encoder_driver class.
 *
 *  @author Eddie Ruano
 *
 *  Revisions: @ 4/27/2016 finished fixing bug in ISR
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


//-----------------------------------------------------------------------------
/** @brief  This class will enable an interruputs on pins given by the main()
 *          function.
 *  @details This class sets up the correct registers so that interrupts are 
 *           enabled on the given pins in the given registers and in the given
 *           configuation. The Interrupt Service Routines happen here as well 
 *           and the logic for that is explained within them.
 *
 */

class encoder_driver
{
private:
   /// No privates
   volatile uint8_t* encoder_DATA_PORT;

protected:
   /// pointer to external iterrupt control register
   volatile uint8_t* interrupt_control;
   /// pointer to the interrupt enable register EIMSK
   volatile uint8_t* interrupt_enable_mask;
   /// pointer to the data direction register of DDRE in this case
   volatile uint8_t* encoder_DDR_PORT;
   /// pinA value which is the first half of the directive for the control reg
   uint8_t a_directive_pin0;
   /// pinB value is the first half of the second directive for the control reg
   uint8_t b_directive_pin0;
   /// these values set the register to activate the correct pins to interrupts.
   uint8_t a_interrupts_activate;
   /// set b to also an intterupt pin
   uint8_t b_interrupts_activate;
   // these values represent which pins we want to set as inputs in order to drive them high or low.
   uint8_t a_set_as_input;
   /// variable to set b to high or low to allow a constant high pull up or constant low pull down
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


}; // end of class encoder_driver


/// This operator prints useful info about particular given motor driver
emstream& operator << (emstream&, encoder_driver&);
// closes the removal of the code
#endif

