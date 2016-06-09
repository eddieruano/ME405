//**************************************************************************************
/** @file task_reciever.cpp
 *    This file contains the code for a task class which handles reading a serial input
 *    and translates it into commands to be handled, like a remote version of task_user.
 *
 *  Revisions:
 *    @li 09-30-2012 JRR Original file was a one-file demonstration with two tasks
 *    @li 10-05-2012 JRR Split into multiple files, one for each task
 *    @li 10-25-2012 JRR Changed to a more fully C++ version with class task_sender
 *    @li 10-27-2012 JRR Altered from data sending task into LED blinking class
 *    @li 11-04-2012 JRR Altered again into the multi-task monstrosity
 *    @li 12-13-2012 JRR Yet again transmogrified; now it controls LED brightness
 *    @li 05-25-2016 ATL task_brightness used as template for task_reciever
 *    @li 05-27-2016 ATL added parsing of hex values into controls in drive mode, and imitation watchdog
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
#include "task_receiver.h"                  // Header for this task
#include "shares.h"                         // Shared inter-task communications

// Declare Letter constants based on ASCII
#define UPPERCASE_A     65
#define UPPERCASE_Z     90
#define LOWERCASE_A     97
#define LOWERCASE_Z     122

// Declare all modes of operation
#define DRIVE_MODE      0x02
#define ECHO_MODE       0x03
#define ERROR           0xFF

#define CMD_BUF_LEN     5   // size for command buffer
#define DRIVE_BUF_LEN   8   // size for drive control buffer

#define THREAD_DELAY    1000 // msec
// #define WDT_TIMEOUT  (50000 / THREAD_DELAY) // 20 sec / delay = # of loops before timeout

//-------------------------------------------------------------------------------------
/** This constructor creates a task which handles translating incoming commands from the
 *  serial port on E0 and E1. The main job of this constructor is to call the
 *  constructor of parent class (\c frt_task ); the parent's constructor the work.
 *  @param a_name A character string which will be the name of this task
 *  @param a_priority The priority at which this task will initially run (default: 0)
 *  @param a_stack_size The size of this task's stack in bytes
 *                      (default: configMINIMAL_STACK_SIZE)
 *  @param p_ser_dev Pointer to a serial device (port, radio, SD card, etc.) which can
 *                   be used by this task for debug messages. (DEFAULT = null)
 */

task_receiver::task_receiver (
    const char* a_name,
    unsigned portBASE_TYPE a_priority,
    size_t a_stack_size,
    emstream* p_ser_dev
): TaskBase (a_name, a_priority, a_stack_size, p_ser_dev)
{
    token = false;
    in_drive = false;
    task_speed = 100;
    count = 0;
    entry_token = true;
    memset(buffer, 0, 7);
    mode = 0;
    paired = false;
    // set up USART0 on E0 and E1 for external comms
    p_ser_bt = new rs232(0, 0);
    UCSR0A |= (1 << U2X0); // set the double-speed bit
    UBRR0 = 16; // set baud rate to 115200
}


//-------------------------------------------------------------------------------------
/** This method is called once by the RTOS scheduler, and runs forever. Sets up the
 *  buffers and a serial line on pins E0 and E1, then enters an infinite loop. Inside
 *  the loop, the reciever has two modes: command and drive. In command mode, it
 *  listens for short (4-character) phrases for configuration or testing. In drive mode,
 *  it listens for long strings of characters representing the state of each module as
 *  hexadecimal values. Features a watchdog timer that will zero all controls if enough
 *  time has passed since last command. @b note: drive mode needs to be attached to actual
 *  controls.
 */

void task_receiver::run (void)
{
    // Make a variable which will hold times to use for precise task scheduling
    TickType_t previousTicks = xTaskGetTickCount ();

    // Get pair going
    *p_serial << "In Run" << endl;
    char pairkey[] = "_CON";
    for (; (!paired);)
    {
        if (p_ser_bt -> check_for_char())
        {
            char_in = p_ser_bt -> getchar();

            if (char_in == '_')
            {
                getCommand();
                *p_serial << "ACOMD: " << buffer << endl;
                if (strcmp(buffer, pairkey) == 0)
                {
                    *p_serial << "Paired !" << endl;
                    *p_ser_bt << "_ACK" << endl;
                    paired = true;
                }
            }
        }
        delay_from_for_ms(previousTicks, 100);
    }

    // get the drive mode going
    char drvkey[] = "_DRV";
    for (; !in_drive;)
    {
        if (p_ser_bt -> check_for_char())
        {
            char_in = p_ser_bt -> getchar();

            if (char_in == '_')
            {
                getCommand();
                *p_serial << "DCMD: " << buffer << endl;
                if (strcmp(buffer, drvkey) == 0)
                {
                    *p_serial << "Paired !" << endl;
                    *p_ser_bt << PMS("_ACK") << endl;
                    in_drive = true;
                }
            }
        }
        delay_from_for_ms(previousTicks, 100);
    }
    // DRIVE MODEE
    count = 0;
    for (;;)
    {
        *p_serial << "WE IN IT" << endl;
        if (p_ser_bt -> check_for_char())
        {
            char_in = p_ser_bt -> getchar();
            buffer[count] = char_in;

            count++;
            *p_serial << "C: " << char_in << endl;
        }
        if (count == 9)
        {
            *p_ser_bt << PMS("_ACK") << endl;
            count = 0;
            //printBuffer();
        }
        delay_from_for_ms(previousTicks, 200);
    }

}

// void task_receiver::getInput(uint8_t size)
// {
//     count = 0;

// }
//
bool task_receiver::getCommand(void)
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

void task_receiver::printBuffer()
{
    for (count = 0; count < 8; count++)
    {
        *p_serial << "Buffer[" << count << "]: " << buffer[count] << endl;
    }
    return;
}
