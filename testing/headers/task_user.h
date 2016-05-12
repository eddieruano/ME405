//*****************************************************************************
/** @file task_user.h
 *  @brief     This is the header file for the 'task_user' class. 

 *  @details   Built on top of JR Ridgely's architecture, this header lays out
 *             all the protoypes and local variables used in the 'task_user'
 *             class. 
 *
 *  @author Eddie Ruano
 *  @author JR Ridgely
 *
 *  Revisions: @li 4/20/2016 added main structure
 *             @li 4/22/2016 added pointers and correct logic
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
//****************************************************************************

// This define prevents this .h file from being included multiple times in a .cpp file
#ifndef _TASK_USER_H_
#define _TASK_USER_H_

#include <stdlib.h>                         // Prototype declarations for I/O functions

#include "FreeRTOS.h"                       // Primary header for FreeRTOS
#include "task.h"                           // Header for FreeRTOS task functions
#include "queue.h"                          // FreeRTOS inter-task communication queues

#include "rs232int.h"                       // ME405/507 library for serial comm.
#include "adc.h"                            // Header for A/D converter class driver
#include "motor_driver.h"
#include "time_stamp.h"                     // Class to implement a microsecond timer
#include "taskbase.h"                       // Header for ME405/507 base task class
#include "taskqueue.h"                      // Header of wrapper for FreeRTOS queues
#include "textqueue.h"                      // Header for a "<<" queue class
#include "taskshare.h"                      // Header for thread-safe shared data

#include "shares.h"                         // Global ('extern') queue declarations
#include "imu_driver.h"

/// This macro defines a string that identifies the name and version of this
/// program.
#define PROGRAM_VERSION PMS ("ME405 Testing Platfrom v6.0")


/**
 * @brief      This task will interact with the user and make the motors
 *             operate based on their input.
 */

class task_user : public TaskBase
{
private:
    /// This holds the character entered by the user in a class variable in order to allow its availability to all methods in the class.
    char char_in;        
    /// This holds the number entered by the user and makes it avaliable to all methods in the class.                 
    int16_t number_entered;
    /// This holds a boolean variable that holds a whether case1 will go into the main motor module if TRUE or the single motor module if FALSE
    bool in_main_motor_module;
    bool in_imu_module;
    bool in_encoder_module;
    bool in_drive_mode;
    bool in_joystick_mode;
    /// 
    int16_t local_motor1_power;
    /// This holds the local power variable for motor 2 in order to be able to display all info in printDashBoard()
    int16_t local_motor2_power;
    /// This holds what the first motor is currently doing because the global directive can be overwritten when switching to motor 2
    int8_t local_motor1_directive;
    /// This holds what the second motor is currently doing because the global directive can be overwritten when switching to motor 1
    int8_t local_motor2_directive;
    /// This holds what motor we are currently affecting.
    uint8_t local_motor_select;

    adc* p_adc_x;
    adc* p_adc_y;
protected:
    // protected so that only methods of this class or possibly descendents can use it

    /// This method displays a simple help message telling the user what to do
    void print_help_message (void);
    /// This method displays information about the status of the system
    void show_status (void);
    /// Method returns true if the user has typed somthing into the terminal window
    bool hasUserInput (void);
    /// Method extracts the number entered by the user and places it into private variable 'number_entered' 
    void getNumberInput(void);
    /// Prints the opening menu when the user enters the 'Main Motor Module' in case1
    void printMotorMenu (void);
    /// Prints the Main Menu at the beginning of the program.
    void printMainMenu (void);
    /// Prints the Single Motor Menu after the user has entered from the 'Main Motor Module'
    void printSingleMotorOptions(void);
    /// Prints useful information about each motor like directive, power, and direction
    void printDashBoard(void);
    /// Method that takes the motor number, motor power, and directive and sets the correct motor to that directive at the apporpriate power level.
    void setMotor(uint8_t, int16_t, uint8_t);
    /// This Method resets the 'run once' lock on each print menu method so that it can be run again.
    void resetMenus(void);
    /// Holds true if there is a menu currently printed on the screen, false otherwise.
    bool is_menu_visible;
    /// Method checks whether the motor number entered is valid and within range (<2) right now.
    bool isValidMotor(int16_t);
    /// Method prints the menu for the Encoder control module
    void printEncoderModuleOptions(void);

    void printIMUModuleOptions(void);

    void printDriveModeOptions(void);

    void printJoystickOptions(void);

public:
    /// This constructor creates a user interface task object
    task_user (const char*, unsigned portBASE_TYPE, size_t, emstream*, imu_driver*);

    /** This method is called by the RTOS once to run the task loop for ever and ever.
     */
    void run (void);
    imu_driver* p_imu;
};
#endif // _TASK_USER_H_
