//*****************************************************************************
/** @file task_user.cpp
 *  @brief     This is the file for the 'task_user' class that is what any
 *             user will mainly interact with
 *
 *  @details   Built on top of JR Ridgely's architecture, this task uses case
 *             statements that infinitely loop in order to perform certain
 *             operations. In this version of this class (v1.1) the only
 *             module avaliable is the 'Main Motor Module' which allows the
 *             user to control both motors in on either HBridge
 *             simultaneously.
 *
 *  @author Eddie Ruano
 *  @author JR Ridgely
 *
 *  Revisions: @li 4/26/2016 ERR added a new module for testing of the encoder
 *             task
 *             @li 4/23/2016 added a bunch of helper methods to make testing
 *             easier in the future
 *             @li 4/21/2016 overhauled entire case structure
 *             @li 09-30-2012 JRR Original file was a one-file demonstration
 *             with two tasks
 *             @li 10-05-2012 JRR Split into multiple files, one for each task
 *             @li 10-25-2012 JRR Changed to a more fully C++ version with
 *             class task_user
 *             @li 11-04-2012 JRR Modified from the data acquisition example
 *             to the test suite
 *             @li 01-04-2014 JRR Changed base class names to TaskBase,
 *             TaskShare, etc.
 *  License:
 *    This file is copyright 2012 by JR Ridgey and released under the Lesser
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

#include <avr/io.h>                         // Port I/O for SFR's
#include <avr/wdt.h>
#include "task_user.h"                      // task_user header file
#include "ansi_terminal.h" //testing ansi
#include "adc.h"
#include "task_user_library.h"
//#include "mirf.h"
//#include "spi.h"
#include "spi_driver.h"
#include "nrf24_driver.h"
#include "nRF24L01.h"
//Set these defines in case we want to change things later.
#define W 1
#define R 0

/**  This constant sets how many RTOS ticks the task delays if the user's not
 *   talking The duration is calculated to be about 5 ms.
 */
const TickType_t ticks_to_delay = ((configTICK_RATE_HZ / 1000) * 5);

//-----------------------------------------------------------------------------
/**
 * @brief      Constructor that creates a new task to interact with the user.
 *             All incoming paramters are initialized in the parent class
 *             construtor TaskBase.
 * @details    Added new initializations for private variables added in order
 *             to clean up the clutter in the main case statement in the
 *             'run()' method.
 *
 * @param[in]  a_name         A character string which will be the name of
 *                            this task
 * @param[in]  a_priority     The priority at which this task will initially
 *                            run (default: 0)
 * @param[in]  a_stack_size   The size of this task's stack in bytes
 *                            (default: configMINIMAL_STACK_SIZE)
 * @param      p_ser_dev      Pointer to a serial device (port, radio, SD
 *                            card, etc.
 */
task_user::task_user (const char* a_name,
                      unsigned portBASE_TYPE a_priority,
                      size_t a_stack_size,
                      emstream* p_ser_dev
                     )
    : TaskBase (a_name, a_priority, a_stack_size, p_ser_dev)
{

    *p_serial << ATERM_BKG_WHITE;
}


/**
 * @brief      This method runs when the 'task_user' is called.
 *
 * @details    This method runs inifitely when the task is called so all logic
 *             is contained in here. A for(;;) loop runs until it gets to the
 *             end where it delays for 1 ms to allow other lower level
 *             priority tasks a chance to run.
 */
void task_user::run (void)
{
    time_stamp a_time;             // Holds the time so it can be displayed
    number_entered = 0;            // Holds a number being entered by user
    uint8_t count;
    count = 0;
    uint8_t data[16];
    //
    for (;;)
    {
        switch (state)
        {
        // Initialize first task where we wait for
        case (0):
            printMainMenu();
            // If the user typed a character, read
            if (hasUserInput())
            {
                switch (char_in)
                {
                case ('p'):
                    *p_serial << PMS("PORTA: ") << bin << PINA << endl;
                    *p_serial << PMS("PORTB: ") << bin << PINB << endl;
                    *p_serial << PMS("PORTC: ") << bin << PINC << endl;
                    *p_serial << PMS("PORTD: ") << bin << PIND << endl;
                    *p_serial << PMS("PORTE: ") << bin << PINE << endl;


                    break;
                case ('c'):
                    transition_to(1);
                    break;

                default:
                    *p_serial << '"' << char_in << PMS ("\": WTF?") << endl;
                    break;
                }; // End switch for characters
            } // End if a character was received

            break; // End of state 0
        case (1):
            nrf24_driver* main_nrf24;
            main_nrf24 = new nrf24_driver(p_serial);


            main_nrf24 -> printNRF(p_serial, main_nrf24);

            main_nrf24 -> initialize();

            //Space
            *p_serial << endl << endl << endl;

            main_nrf24 -> printNRF(p_serial, main_nrf24);

            PORTD |= (1 << PD1);
            //delay(500);
            while (main_nrf24 -> readRegister(STATUS) != 0x40)
            {
                //main_nrf24 -> printNRF(p_serial, main_nrf24);
            }

            *p_serial << "Broke Loop! " << endl;
            uint8_t data_read[32];

            main_nrf24 -> recPayload(4, data_read);

            // *p_serial << endl << endl << "DATA READ: "<< hex << data_read[0] << " " << data_read[1] << " " << data_read[2] << " " << data_read[3] << " " << data_read[4] << " " << endl;
            *p_serial << "Data Read" << endl;
            uint8_t count;
            for (count = 0; count < 4; count++)
            {
                *p_serial << hex << count << ": " << data_read[count] << endl;
            }

            PORTD &= ~(1 << PD1);
            break;
        default:
            *p_serial << PMS ("Illegal state! Resetting AVR") << endl;
            wdt_enable (WDTO_120MS);
            for (;;);
            break;

        } // End switch state

        runs++;                             // Increment counter for debugging

        // No matter the state, wait for approximately a millisecond before we
        // run the loop again. This gives lower priority tasks a chance to run
        delay_ms (1);
    }

}
