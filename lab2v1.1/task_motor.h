//**************************************************************************************
/** @file task_motor.h
 *    This is the header file for our task_motor class
 *
 *  Revisions:
 *
 *  License:
 *    This file is copyright 2012 by JR Ridgely and released under the Lesser GNU
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

/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** task_motor.h
** Description: This is the header file for our task_motor class
**
** Author: Eddie Ruano
** Date:   April 15, 2015
** -------------------------------------------------------------------------*/

// This define prevents this .h file from being included multiple times in a .cpp file

#ifndef TASK_MOTOR_H
#define TASK_MOTOR_H

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
#include "adc.h"                            // Header for A/D converter driver class
#include "motor_driver.h"                   // Header for Generic Motor driver


//-------------------------------------------------------------------------------------
/** @brief
 *  @details
 */

class task_motor : public TaskBase
{
private:
    // No private variables or methods for this class

protected:
    // No protected variables or methods for this class

public:
    /// This constructor creates a generic task of which many copies can be made
    task_motor (const char*, unsigned portBASE_TYPE, size_t, emstream*, motor_driver*, uint8_t);

    /// This method is called by the RTOS once to run the task loop for ever and ever.
    void run (void);

    motor_driver* motor;

    /// Variable to confirm this particular task
    uint8_t identifier;


};

#endif // _TASK_MOTOR_H