//**************************************************************************************
/** @file task_reciever.h
 *    This file contains the header for a task class that controls recieving commmands
 *    over the serial port on E0 and E1.
 *
 *  Revisions:
 *    @li 09-30-2012 JRR Original file was a one-file demonstration with two tasks
 *    @li 10-05-2012 JRR Split into multiple files, one for each task
 *    @li 10-25-2012 JRR Changed to a more fully C++ version with class task_sender
 *    @li 10-27-2012 JRR Altered from data sending task into LED blinking class
 *    @li 11-04-2012 JRR Altered again into the multi-task monstrosity
 *    @li 12-13-2012 JRR Yet again transmogrified; now it controls LED brightness
 *    @li 05-25-2016 ATL task_brightness used as the template for task_reciever
 *    @li 05-27-2016 ATL added thread delay and watchdog timeout constants
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

// This define prevents this .h file from being included multiple times in a .cpp file
#ifndef _TASK_RECV_H_
#define _TASK_RECV_H_

#include <stdlib.h>                         // Prototype declarations for I/O functions
#include <avr/io.h>                         // Header for special function registers

#include "FreeRTOS.h"                       // Primary header for FreeRTOS
#include "task.h"                           // Header for FreeRTOS task functions
#include "queue.h"                          // FreeRTOS inter-task communication queues

#include "taskbase.h"                       // ME405/507 base task class
#include "time_stamp.h"                     // Class to implement a microsecond timer
#include "taskqueue.h"                      // Header of wrapper for FreeRTOS queues
#include "taskshare.h"                      // Header for thread-safe shared data

#include "rs232int.h"                       // ME405/507 library for serial comm.

//-------------------------------------------------------------------------------------
/** @brief   This task controls the reception and translation of commands over serial.
 *  @details Pins E0 and E1 are the serial port used for communication. This serial line
 *      is configured here to be at a baud rate of 115200.
 */

class task_receiver : public TaskBase
{
private:
    /// pointer to the serial object that is used for comms.
    rs232* p_ser_bt;
    /// General use token for loops and process flow
    bool token;
    bool in_drive;
    /// General use token for single run operations
    bool entry_token;
    /// General use count variable for loop operations
    uint16_t count;
    /// Current mode of receiver
    uint8_t mode;
    /// General purpose char holder
    char char_in;
    bool paired;
    uint16_t task_speed;
    uint32_t timeout;

protected:
    // No protected variables or methods for this class

public:
    // This constructor creates the task.
    task_receiver (
        const char*,
        unsigned portBASE_TYPE,
        size_t,
        emstream*
    );

    // This method is called by the RTOS once to run the task loop forever and ever.
    void run (void);
    bool compare_recv (emstream*, char*, emstream*);
    bool compare_string(char*, char*, uint8_t);
    bool isAlpha(char);
    uint8_t checkResponse(void);   
    void printBuffer(void); 
    bool pairDevices(void);
    bool isValidPair(char);
    bool getCommand(void);
    char buffer[8];
};

#endif // _TASK_RECV_H_
