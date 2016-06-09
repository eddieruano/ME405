//**************************************************************************************
/** @file task_transmitter.cpp
 *    This file contains the code for a task class which handles reading control inputs and
 *    encoding them into a serial transmission to be sent to @b task_reciever.
 *
 *  Revisions:
 *    @li 09-30-2012 JRR Original file was a one-file demonstration with two tasks
 *    @li 10-05-2012 JRR Split into multiple files, one for each task
 *    @li 10-25-2012 JRR Changed to a more fully C++ version with class task_sender
 *    @li 10-27-2012 JRR Altered from data sending task into LED blinking class
 *    @li 11-04-2012 JRR Altered again into the multi-task monstrosity
 *    @li 12-13-2012 JRR Yet again transmogrified; now it controls LED brightness
 *    @li 05-31-2016 ATL task_brightness used as template for task_transmitter
 *
 *  License:
 *    This file is copyright 2016 by AT Lombardi and released under the Lesser GNU
 *    Public License, version 2. It intended for educational use only, but its use
 *    is not limited thereto. */
/*    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUEN-
 *    TIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 *    OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *    CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 *    OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */
//**************************************************************************************

#include "textqueue.h"                      // Header for text queue class
#include "task_transmitter.h"               // Header for this task
#include "shares.h"                         // Shared inter-task communications
#include <util/delay.h>


// Declare Letter constants based on ASCII
#define UPPERCASE_A     65
#define UPPERCASE_Z     90
#define LOWERCASE_A     97
#define LOWERCASE_Z     122

// Declare all modes of operation
#define DRIVE_MODE      0x02
#define ECHO_MODE       0x03
#define ERROR           0xFF

//-------------------------------------------------------------------------------------
/** This constructor creates a task which handles translating control inputs into serial
 *  messages to be sent over the serial port on E0 and E1. The main job of this constructor
 *  is to call the constructor of parent class (\c frt_task ); the parent's constructor
 *  does most of the work.
 *  @param a_name A character string which will be the name of this task
 *  @param a_priority The priority at which this task will initially run (default: 0)
 *  @param a_stack_size The size of this task's stack in bytes
 *                      (default: configMINIMAL_STACK_SIZE)
 *  @param p_ser_dev Pointer to a serial device (port, radio, SD card, etc.) which can
 *                   be used by this task for debug messages. (DEFAULT = null)
 */

/**
 * @brief      { function_description }
 *
 * @param[in]  a_name              The a name
 * @param[in]  portBASE_TYPE       The port base type
 * @param[in]  a_stack_size        The a stack size
 * @param      p_ser_dev           The ser development
 * @param[in]  p_con_drv_incoming  The con driver incoming
 */
task_transmitter::task_transmitter (
    const char* a_name,
    unsigned portBASE_TYPE a_priority,
    size_t a_stack_size,
    emstream* p_ser_dev,
    controller_driver* p_con_drv_incoming
): TaskBase (a_name, a_priority, a_stack_size, p_ser_dev)
{
    // Connect pointers
    p_local_controller_driver = p_con_drv_incoming;
    memset(reader_data, 0, 3);
    token = true;
    in_drive = false;
    count = 0;
    entry_token = true;
    memset(buffer, 0, 7);
    mode = 0;

    /// Start at 1000ms for pairing
    task_speed = 10;
    paired = false;
    // set up USART0 on E0 and E1 for external comms
    p_ser_bt = new rs232(0, 0);
    UCSR0A |= (1 << U2X0); // set the double-speed bit
    UBRR0 = 16; // set baud rate to 115200

}

char ack[] = "_ACK";
/**
 * @brief      This method is called once by the RTOS scheduler, and runs
 *             forever.
 *
 * @details    Sets up the buffers and a serial line on pins E0 and E1, then
 *             enters an infinite loop. Inside the loop, the transmitter
 *             calculates what values to send for power and steering, as well
 *             as whether to be in high gear or low gear, and command mode. It
 *             then calculates a checksum using the 'modular sum' method, and
 *             adds that to the buffer.
 */
void task_transmitter::run (void)
{
    // Make a variable which will hold times to use for precise task scheduling
    TickType_t previousTicks = xTaskGetTickCount ();
    // bool in_cmd_mode = true;
    // char debug_buf[20];
    
    for (; (!paired);)
    {
        *p_ser_bt << PMS("_CON") << endl;
        if (p_ser_bt -> check_for_char())
        {
            char_in = p_ser_bt -> getchar();

            if (char_in == '_')
            {
                getCommand();
                *p_serial << "Rec: " << buffer << endl;
                if (strcmp(buffer, ack) == 0)
                {
                    *p_serial << "Paired !" << endl;
                    *p_ser_bt << "_DRV" << endl;
                    paired = true;
                }
            }
        }
        delay_from_for_ms(previousTicks, 500);
    }

    for (; !in_drive;)
    {
        *p_ser_bt << PMS("_DRV") << endl;
        if (p_ser_bt -> check_for_char())
        {
            char_in = p_ser_bt -> getchar();

            if (char_in == '_')
            {
                getCommand();
                *p_serial << "Rec: " << buffer << endl;
                if (strcmp(buffer, ack) == 0)
                {
                    *p_serial << "In Drive !" << endl;
                    in_drive = true;
                }
            }
        }
        delay_from_for_ms(previousTicks, 500);
    }

    for (;;)
    {

        // get the data
        p_local_controller_driver -> read(reader_data);
        // encode everything in buffer
        encodeData();
        while (!send());
        *p_serial << "Sent" << endl;
        delay_from_for_ms(previousTicks, 500);
    }


}

bool task_transmitter::getCommand(void)
{

    buffer[0] = char_in;
    count = 1;
    while (p_ser_bt -> check_for_char())
    {

        char_in = p_ser_bt -> getchar();
        buffer[count] = char_in;
        if (count == 3)
        {
            buffer[count + 1] = '\0';
            *p_serial << "buf: " << buffer << endl;
            return true;
        }
        count++;
    }
    return false;
}

bool task_transmitter::send(void)
{
    count = 0;
    for(count = 0; count < 8; count++)
    {
        *p_ser_bt << outbuffer[count] << endl;
    }
    // check for ack
    while (p_ser_bt -> check_for_char())
    {
        char_in = p_ser_bt -> getchar();
        if (char_in == '_')
        {
            getCommand();
            
            printBuffer();
            if (strcmp(buffer, ack) == 0)
            {
                *p_serial << "Payload Received" << endl;
                return true;
            }
        }
    }
    return false;
}

void task_transmitter::encodeData()
{
    for(count = 0; count < 8; count++)
    {
        outbuffer[count] = 'F';
    }
    return;
}

void task_transmitter::printBuffer()
{
    for(count = 0; count < 8; count++)
    {
        *p_serial << "Buffer[" << count << "]: " << outbuffer[count] << endl;
    }
    return;
}