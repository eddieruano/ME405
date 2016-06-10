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

task_transmitter::task_transmitter (
    const char* a_name, 
    unsigned portBASE_TYPE a_priority, 
    size_t a_stack_size,
    emstream* p_ser_dev,
    TaskShare<uint16_t>* p_motor_share,
    TaskShare<uint16_t>* p_steer_share,
    TaskShare<uint8_t>*  p_gears_share
): TaskBase (a_name, a_priority, a_stack_size, p_ser_dev)
{
    p_motor = p_motor_share;
    p_steer = p_steer_share;
    p_gears = p_gears_share;
}


//-------------------------------------------------------------------------------------
/** This method is called once by the RTOS scheduler, and runs forever. Sets up the
 *  buffers and a serial line on pins E0 and E1, then enters an infinite loop. Inside
 *  the loop, the transmitter calculates what values to send for power and steering,
 *  as well as whether to be in high gear or low gear, and command mode. It then
 *  calculates a checksum using the 'modular sum' method, and adds that to the buffer.
 */

void task_transmitter::run (void)
{
    // Make a variable which will hold times to use for precise task scheduling
    TickType_t previousTicks = xTaskGetTickCount ();
    bool in_cmd_mode = true;
    char debug_buf[20];

    // set up USART0 on E0 and E1 for external comms
    p_ser_bt = new rs232(0, 0);
    UCSR0A |= (1 << U2X0); // set the double-speed bit
    UBRR0 = 16; // set baud rate to 115200
    *p_serial << PMS ("Serial transmitter connecting...") << endl; // debug notification

    // startup connection test
    //*p_ser_bt << PMS ("CONN") << endl;
    while (!compare_recv(p_ser_bt, "ACK_", p_serial))
    {
        // wait for connection confirmation, retry every second
        *p_ser_bt << PMS ("CONN") << endl;
        delay_from_for_ms(previousTicks, 1000);
    }
    *p_serial << PMS ("Serial transmitter ready.") << endl;
    // end of connection test block
    
    // This is the task loop for the transmitter control task. This loop runs until the
    // power is turned off or something equally dramatic occurs
    for (;;)
    {
        // report recieved stuff if this is uncommented
        // /*
        int i=0;
        while ( (p_ser_bt->check_for_char()) && (i<CMD_BUF_LEN) )
        {
            char in = p_ser_bt->getchar();
            if (in >= 'A')
            {
                debug_buf[i++] = in;
            }
        }
        debug_buf[i] = '\0'; // add null terminator
        if (i>0)
        {
            *p_serial << PMS ("Recv: ") << debug_buf << endl;
        }
        // */ // end of reception report block
        
        if (in_cmd_mode)
        {
            if (p_serial->check_for_char())
            {
                char in = p_serial->getchar();
                if (in >= 'A')
                {
                    *p_ser_bt << in;
                }
            }
        }
        else // in drive mode
        {
            uint16_t motor_value = p_motor->get();
            uint16_t steer_value = p_steer->get();
            bool  set_gear_high = (p_gears->get() != 0) ? (true) : (false);
            bool enter_cmd_mode = true;
            
            // any calculations we need to do?
            
            // fill out buffer to send
            buffer[2] = (motor_value >> 8);
            buffer[3] = motor_value;
            buffer[4] = (steer_value >> 8);
            buffer[5] = steer_value;
            buffer[6] = (set_gear_high) ? (1) : (0);
            buffer[7] = (enter_cmd_mode) ? (0xC) : (0);
        
            // calculate checksum using modular sum method
            uint16_t chk = 0;
            for (i=2; i<DRIVE_BUF_LEN; i++)
            {
                chk += buffer[i]; // sum of other values
            }
            buffer[0] = (~chk + 1) >> 8; // two's complement
            buffer[1] = (~chk + 1);
            
            // write command bytes over serial
            for (i=0; i<(DRIVE_BUF_LEN/2); i++)
            {
                *p_ser_bt << hex << buffer[2*i] << buffer[2*i +1] << dec;
            }
            *p_ser_bt << endl;
        }    
        // Increment the run counter. This counter belongs to the parent class and can
        // be printed out for debugging purposes
        runs++;
        
        // This is a method we use to cause a task to make one run through its task
        // loop every N milliseconds and let other tasks run at other times
        delay_from_for_ms (previousTicks, THREAD_DELAY);
    }
}

bool compare_recv (emstream* port, char* comp, emstream* deb)
{

//bool compare_recv (emstream* port, char* comp)
//{
    char recv[20]; // probably overkill with the size
    int i = 0;
    while ( (port->check_for_char()) && (i<19) )
    {
        recv[i] = port->getchar();
        if (recv[i] >= 'A') i++;
    }
    recv[i] = '\0'; // add null terminator
    *deb << recv << endl;
    
    if (strcmp(recv, comp)==0)
    {
        return true;
    }
    else return false;
}