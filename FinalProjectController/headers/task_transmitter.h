//**************************************************************************************
/** @file task_transmitter.h
 *    This file contains the header for a task class that controls sending commmands
 *    over the serial port on E0 and E1.
 *
 *  Revisions:
 *    @li 09-30-2012 JRR Original file was a one-file demonstration with two tasks
 *    @li 10-05-2012 JRR Split into multiple files, one for each task
 *    @li 10-25-2012 JRR Changed to a more fully C++ version with class task_sender
 *    @li 10-27-2012 JRR Altered from data sending task into LED blinking class
 *    @li 11-04-2012 JRR Altered again into the multi-task monstrosity
 *    @li 12-13-2012 JRR Yet again transmogrified; now it controls LED brightness
 *    @li 05-31-2016 ATL task_brightness used as the template for task_transmitter
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
#ifndef _TASK_TRAN_H_
#define _TASK_TRAN_H_

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
#include "controller_driver.h"

#define CMD_BUF_LEN 5   // size for command buffer
#define DRIVE_BUF_LEN 8 // size for drive control buffer

#define THREAD_DELAY 10 // msec
#define WDT_TIMEOUT  (50000 / THREAD_DELAY) // 20 sec / delay = # of loops before timeout

//-------------------------------------------------------------------------------------
/** @brief   This task controls the reception and translation of commands over serial.
 *  @details Pins E0 and E1 are the serial port used for communication. This serial line
 *      is configured here to be at a baud rate of 115200.
 */

class task_transmitter : public TaskBase
{
private:
    /// pointer to the serial object that is used for comms.
    rs232* p_ser_bt;
    /// The driver for reading data from the pins
    controller_driver* p_local_controller_driver;
    /// General use token for loops and process flow
    bool token;
    /// General use token for single run operations
    bool entry_token;
    /// General use count variable for loop operations
    uint8_t count;
    /// Current mode of receiver
    uint8_t mode;
    /// General buffer
    char buffer[10];
    /// Buffer Outgoing 13 chars in total
    char outbuffer[13];
    /// Holder controller data
    uint16_t reader_data[3];
    /// Speed of the task
    uint16_t task_speed;
    /// Gen purpose
    char char_in;
    /// pair var
    bool paired;
    uint32_t timeout;
    bool in_drive;

    char superbuffer[3];

    uint8_t joy_read[2];



protected:
    // No protected variables or methods for this class


public:
    /// Constructor takes the usual task arguments, as well as three shared variables that store the motor setpoint, the steering angle, and the state of the gearbox.
    task_transmitter (
        const char*,
        unsigned portBASE_TYPE,
        size_t,
        emstream*,
        controller_driver*
    );

    // This method is called by the RTOS once to run the task loop forever and ever.
    void run (void);
    // method for comparing strings read off the serial port

    bool isAlpha(char);
    uint8_t checkResponse(void);
    void printBuffer(void);
    bool isValidPair(char);
    bool pairDevices(void);
    bool secureSend(void);
    void convertData(void);
    bool getCommand(void);
    bool send(void);
    void encodeData(void);
    char encodeToHexChar(uint16_t);
    
};



#endif // _TASK_TRAN_H_
