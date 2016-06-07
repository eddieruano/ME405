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
#include "task_reciever.h"                  // Header for this task
#include "shares.h"                         // Shared inter-task communications
#include "task_transmitter.h"
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

task_reciever::task_reciever (
    const char* a_name, 
    unsigned portBASE_TYPE a_priority, 
    size_t a_stack_size,
    emstream* p_ser_dev,
    TaskShare<uint16_t>* p_motor_share,
    TaskShare<uint16_t>* p_steer_share,
    TaskShare<uint8_t>*  p_gears_share
): TaskBase (a_name, a_priority, a_stack_size, p_ser_dev)
{
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

void task_reciever::run (void)
{
    // Make a variable which will hold times to use for precise task scheduling
    TickType_t previousTicks = xTaskGetTickCount ();
    
    // set up vars to handle incoming transmissions
    char cmd_buf[CMD_BUF_LEN];
    bool command = true;
    char buffer[DRIVE_BUF_LEN];
    uint8_t count;
    uint16_t watchdog; 
    
    // init buffers to 0
    uint8_t i;
    for (i=0; i<CMD_BUF_LEN; i++)
    {
        cmd_buf[i] = 0;
    }
    for (i=0; i<DRIVE_BUF_LEN; i++)
    {
        buffer[i] = 0;
    }
    count = 0;

    // set up USART0 on E0 and E1 for external comms
    p_ser_bt = new rs232(0, 0);
    UCSR0A |= (1 << U2X0); // set the double-speed bit
    UBRR0 = 16; // set baud rate to 115200
    
    *p_serial << PMS ("Waiting for connection...") << endl;
    
    while (!compare_recv(p_ser_bt, "CONN", p_serial))
    {
        // wait for connection request
        delay_from_for_ms(previousTicks, 10);
    }
    *p_ser_bt << PMS ("ACK_") << endl;
    *p_serial << PMS ("Serial reciever ready.") << endl;
    
    // This is the task loop for the reciever control task. This loop runs until the
    // power is turned off or something equally dramatic occurs
    for (;;)
    {
        // Increment the run counter. This counter belongs to the parent class and can
        // be printed out for debugging purposes
        runs++;
        
        if (command) // mode
        {
            while (p_ser_bt->check_for_char()) // we have characters waiting on the line
            {
                char recv = p_ser_bt->getchar();
                if (recv >= 'A') // ignore non-alphanumeric chars
                {
                    for (i=0; i<CMD_BUF_LEN-1; i++)
                    {
                        cmd_buf[i] = cmd_buf[i+1]; // push the buffer down one slot
                    }
                    cmd_buf[CMD_BUF_LEN-2] = recv; // add the newly-read character
                    cmd_buf[CMD_BUF_LEN-1] = '\0'; // add a null terminator
                    
                    // debug, print the recieved character and the buffer contents
                    *p_serial << PMS ("recv: ") << recv << PMS (" buf: ") << cmd_buf << endl;
                }
            }
            // strcmp returns 0 if they match
            if (strcmp(cmd_buf, "DRVM") == 0) // Drive Mode command (system values to follow)
            {
                command = false; // switch to drive mode
                for (i=0; i<CMD_BUF_LEN-1; i++)
                {
                    cmd_buf[i] = 0; // reset buffer
                }
            }
            else if (strcmp(cmd_buf, "ECHO") == 0) // Echo command, for testing communication
            {
                *p_ser_bt << PMS ("ACK_") << endl; // send an acknowledge to the controller
                
                for (i=0; i<CMD_BUF_LEN-1; i++)
                {
                    cmd_buf[i] = 0; // reset buffer
                }
            }
            else if (strcmp(cmd_buf, "")) // unrecognized command that isn't an empty string
            {
                *p_ser_bt << PMS ("ERRC") << endl; // send command error warning to controller
                
                // send an error and the buffer contents that caused it, to the debug
                *p_serial << PMS ("Unknown command: \"") << cmd_buf << PMS ("\"") << endl;
                
                for (i=0; i<CMD_BUF_LEN-1; i++)
                {
                    cmd_buf[i] = 0; // reset buffer
                }
            }
        }
        else // drive mode
        {
            /*
            // time-out watchdog
            watchdog++;
            if (watchdog >= WDT_TIMEOUT)
            {
                *p_serial << PMS ("Connection timed out!") << endl;
                for (count=0; count<DRIVE_BUF_LEN; count++)
                {
                    buffer[count] = 0; // clear buffer out, so the system sets everything to zero
                }
                command = true;
            }
            */
            // read in characters
            while (p_ser_bt->check_for_char() && (count < DRIVE_BUF_LEN))
            {
                buffer[count++] = p_ser_bt->getchar();
            }
            if (count == DRIVE_BUF_LEN) // complete control code, ready to parse
            {
                uint16_t checksum = 0;
                // parse drive control codes into hexadecimal
                for (count=0; count<DRIVE_BUF_LEN; count++)
                {
                    if ( (buffer[count]>='0') && (buffer[count]<='9') )
                    {
                        buffer[count] -= '0';
                    }
                    else if ( (buffer[count]>='A') && (buffer[count]<='F') )
                    {
                        buffer[count] = buffer[count] - 'A' + 10;
                    }
                    else
                    {
                        buffer[count] = 0;
                    }
                    checksum += buffer[count];
                    // modular sum: checksum is 2's comp of sum of rest, so the sum of everything
                    // should be equal to zero (checked below)
                }
                uint16_t motor_speed = 0;
                uint16_t steer_angle = 0;
                bool set_gear_high = false;
                
                if (checksum == 0) // make sure message is correct (buffer[0:1] is checksum)
                {
                    motor_speed = 16*((uint16_t)buffer[2]) + buffer[3];
                    steer_angle = 16*((uint16_t)buffer[4]) + buffer[5];
                    set_gear_high = (buffer[6]!=0) ? (true) : (false);
                    if (buffer[7] == 0xC) command = true;
                    watchdog = 0;
                }
                else
                {
                    *p_ser_bt << PMS ("ERRD") << endl; // send drive code error to controller
                }
                count = 0;
                
                // do any calculations to convert setpoints?
                p_motor->put(motor_speed);
                p_steer->put(steer_angle);
                p_gears->put( (set_gear_high) ? (1) : (0) );
            }
        }
        
        // This is a method we use to cause a task to make one run through its task
        // loop every N milliseconds and let other tasks run at other times
        delay_from_for_ms (previousTicks, THREAD_DELAY);
    }
}

