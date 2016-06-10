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
#include "imu_driver.h"
#include "task_user_library.h"
#include "adc.h"
//Set these defines in case we want to change things later.

/// Value for BRAKE directive
#define BRAKE 0
/// Value for SETPOWER directive
#define SETPOWER 1
/// Value for FREEWHEEL directive
#define FREEWHEEL 2
/// Value for POTENTIOMETER directive
#define POTENTIOMETER 3

//#define CENTERZERO 1023

//#define X_CENTER 49
//#define Y_CENTER -29

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

    //Set the menu visible variable or FLAG to false so main menu may print
    is_menu_visible = false;
    in_encoder_module = false;
    // in_imu_module = false;
    in_drive_mode = false;
    in_joystick_mode = false;

    //Set the in main motor flag to false because we start in main menu
    in_main_motor_module = false;
    //Set both motor directives to freewheel because no command has been given
    local_motor1_directive = FREEWHEEL;
    //Intialize the power to ZERO
    local_motor1_power = 0;
    *p_serial << ATERM_BKG_WHITE;
    // p_imu = p_imu_incoming;

    p_adc_x = new adc (p_serial);
    p_adc_y = new adc (p_serial);



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
                // In this switch statement, we respond to different characters as commands typed in by the user
                switch (char_in)
                {
                // The 't' command asks what time it is right now
                case ('m'):
                    *p_serial << PMS("->Selected: ") << char_in;
                    *p_serial
                            << endl
                            << endl
                            << PMS ("\t->Switching to Motor Module..")
                            << endl
                            << PMS ("\t->Clearing Registers and Menus..")
                            << endl
                            << PMS ("\t->Intializing Motors..")
                            << endl;

                    // Reset the visible menu flag
                    resetMenus();
                    // Sets the variable to true so it doesn't go into single module yet
                    in_main_motor_module = true;
                    // go to case 1 over all
                    transition_to(1);
                    break;
                // Enter encoder test module
                case ('e'):
                    *p_serial << PMS("->Selected: ") << char_in;
                    *p_serial
                            << ATERM_CLEAR_SCREEN << ATERM_CURSOR_TO_YX(1, 1)
                            << PMS ("\t->Switching to Encoder Module..")
                            << endl
                            << PMS ("\t->Clearing Registers and Menus..")
                            << endl
                            << PMS ("\t->Intializing Encoder..")
                            << endl << endl;

                    // Reset the visible menu flag
                    resetMenus();
                    // Sets the variable to true so it doesn't go into single module yet
                    in_encoder_module = true;
                    // go to case 1 over all
                    transition_to(3);
                    break;
                // The 't' command asks what time it is right now
                // Enter encoder test module
                case ('i'):
                    *p_serial << PMS("->Selected: ") << char_in;
                    *p_serial
                            << ATERM_CLEAR_SCREEN << ATERM_CURSOR_TO_YX(1, 1)
                            << PMS ("\t->Switching to IMU Module..")
                            << endl
                            << PMS ("\t->Clearing Registers and Menus..")
                            << endl
                            << PMS ("\t->Intializing IMU..")
                            << endl << endl;

                    // Reset the visible menu flag
                    resetMenus();
                    // Sets the variable to true so it doesn't go into single module yet
                    // in_imu_module = true;
                    // go to case 1 over all
                    transition_to(4);
                    break;
                // The 't' command asks what time it is right now
                case ('t'):
                    *p_serial << (a_time.set_to_now ()) << endl;
                    break;

                // The 's' command asks for version and status information
                case ('s'):
                    show_status ();
                    break;
                // The 'd' means drive control
                case ('d'):
                    *p_serial << PMS("->Selected: ") << char_in;
                    *p_serial
                            << ATERM_CLEAR_SCREEN << ATERM_CURSOR_TO_YX(1, 1)
                            << PMS ("\t->Switching to Drive Mode..")
                            << endl << endl;
                    transition_to(5);
                    in_drive_mode = true;
                    resetMenus();
                    break;
                //case ('d'):
                // The 'h' command is a plea for help; '?' works also
                case ('h'):
                case ('?'):
                    print_help_message ();
                    break;

                // The 'n' command runs a test of entering a number
                case ('n'):
                    while (char_in != 'q')
                    {
                        *p_serial << PMS ("Enter decimal numeric digits, "
                                          "then RETURN or ESC") << endl;
                        getNumberInput();
                        *p_serial << endl << endl << PMS("\t->You Entered: ") << number_entered << endl;
                    }
                    break;

                // A control-C character causes the CPU to restart
                case (3):
                    *p_serial << PMS ("Resetting AVR") << endl;
                    wdt_enable (WDTO_120MS);
                    for (;;);
                    break;

                // If character isn't recognized, ask What's That Function?
                default:
                    *p_serial << '"' << char_in << PMS ("\": WTF?") << endl;
                    break;
                }; // End switch for characters
            } // End if a character was received

            break; // End of state 0

        // Motor Control BASE module
        case (1):
            if (in_main_motor_module) //first time here run menu and get input
            {
                // prints the menu for this module
                printMotorMenu();

                // user is going to input motor number or q
                getNumberInput();

                if (char_in == 'q')
                {
                    *p_serial << PMS("->Selected: ") << char_in << endl
                              << endl
                              << endl
                              << PMS("\t->Returning to Mission Control.. ")
                              << endl
                              << PMS("\t->Releasing Motors..") << endl
                              << PMS("\t->Resetting AVR..") << endl;
                    //wdt_enable (WDTO_120MS);
                    //for (;;);
                    //break;
                    //release motors
                    //motor_select->put(1);

                    //motor_directive -> put(FREEWHEEL);
                    //
                    // no longer in this module, leaving
                    in_main_motor_module = false;
                    transition_to(0);
                    resetMenus();
                    break;
                    //reset menu flag
                    //resetMenus();
                    // set the transition & wait for break
                    //transition_to(0);

                }
                // if the number entered is valid
                else if (isValidMotor(number_entered))
                {
                    // re print user input and give them confirmation message.
                    *p_serial
                            << endl
                            << endl
                            << PMS("\t->Switching to Control of Motor ") << number_entered << dec << PMS(".") << endl
                            << PMS ("\t->Return to the Main Motor Module to swap Motors.") << endl;
                    // set local motor select value
                    local_motor_select = number_entered;
                    //reset menus
                    resetMenus();
                    //set flag to off so we can go into the single motor module
                    in_main_motor_module = false;
                    // go back to the top of this menu
                    // since in_main_motor_module flag is set off, we will go into single motor control when this state reloops
                    transition_to(1);
                }
                else
                {
                    *p_serial << PMS("Try Again. ") << endl;
                    in_main_motor_module = true;

                }

                break;
            }
            if (!in_main_motor_module) //already were here now we go deeper
            {


                printSingleMotorOptions();
                // They now select the motor task
                if (hasUserInput())
                {
                    switch (char_in)
                    {
                    case ('s'):
                        *p_serial
                                << PMS("->Selected: ") << char_in << endl;
                        *p_serial
                                << endl
                                << PMS ("Enter the Power Value (-255 to 255);")
                                << endl
                                << PMS("  *Note: Negative Values = Reverse")
                                << endl;

                        //motor_directive -> put(1);
                        getNumberInput();
                        //setPower(motor_select->get(), number_entered);
                        setMotor(local_motor_select, (int16_t)number_entered, SETPOWER);
                        *p_serial
                                << endl << endl
                                << PMS ("\tPower set at ") << number_entered
                                << PMS(". ") << endl;

                        resetMenus();
                        printDashBoard();

                        //now that operation is complete, ask for more
                        *p_serial
                                << endl << PMS("->Choose Motor ")
                                << local_motor_select << PMS(" operation: ")
                                << PMS("\t(Press 'o' for options)")
                                << endl;
                        break;
                    case ('b'):
                        *p_serial
                                << PMS("->Selected: ") << char_in
                                << endl;
                        *p_serial
                                << PMS ("\t->Enter the Brake Force(0 - 255)")
                                << endl;
                        getNumberInput();
                        *p_serial
                                << endl << endl
                                << PMS ("\tBrake set at ") << number_entered
                                << PMS(". ") << endl;

                        setMotor(local_motor_select, number_entered, BRAKE);
                        resetMenus();

                        printDashBoard();
                        //now that operation is complete, ask for more
                        *p_serial
                                << endl << PMS("->Choose Motor ")
                                << local_motor_select << PMS(" operation: ")
                                << PMS("\t(Press 'o' for options)")
                                << endl;
                        break;
                    case ('f'):
                        *p_serial
                                << PMS("->Selected: ") << char_in << endl;
                        *p_serial
                                << PMS ("\t->Releasing Motor..") << endl;
                        setMotor(0, 0, FREEWHEEL);
                        resetMenus();
                        printDashBoard();
                        //now that operation is complete, ask for more
                        *p_serial
                                << endl << PMS("->Choose Motor ")
                                << local_motor_select << PMS(" operation: ")
                                << PMS("\t(Press 'o' for options)")
                                << endl;

                        break;
                    case ('p'):
                        *p_serial
                                << endl << PMS("->Selected: ")
                                << char_in << endl
                                << PMS("\t->Entering Potentiometer Control... ") << endl;
                        *p_serial << PMS ("\t->Potentiometer Activated.") << endl << endl;
                        *p_serial << endl << endl
                                  << PMS ("\t->Press 'q' to return to the Motor ") << local_motor_select << PMS(" Control")
                                  << endl << PMS ("\t->Press 'r' to refresh the DashBoard ") << endl;
                        transition_to(2);
                        resetMenus();
                        break;
                    case ('o'):
                        resetMenus();
                        printSingleMotorOptions();
                        break;
                    case ('q'):
                        *p_serial
                                << PMS("->Selected: ") << char_in << endl
                                << PMS(" Returning to Main Motor Module.. ")
                                << endl;
                        transition_to(1);
                        resetMenus();

                        in_main_motor_module = true;
                        break;
                    default:
                        *p_serial
                                << endl << PMS("'") << char_in
                                << PMS ("' is not a valid entry.") << endl;
                        *p_serial
                                << endl << PMS("->Choose Motor ")
                                << local_motor_select
                                << PMS(" operation: ") << endl;
                        break;
                    }
                }
            }

            break;
        //Potentiometer Mode
        case (2):
            if (hasUserInput())
            {
                if (char_in  == 'q')
                {
                    *p_serial << endl << PMS("->Selected: ") << char_in << endl;
                    transition_to(1);
                    resetMenus();
                    break;
                    //
                }
                if (char_in == 'r')
                {
                    *p_serial << endl << PMS("->Selected: ") << char_in << endl;
                    resetMenus();
                    printDashBoard();
                    *p_serial << endl << endl
                              << PMS ("\t->Press 'q' to return to the Motor ") << local_motor_select << PMS(" Control")
                              << endl << PMS ("\t->Press 'r' to refresh the DashBoard ") << endl;

                }
            }

            setMotor(local_motor_select, motor_power -> get(), POTENTIOMETER);
            printDashBoard();
            break;
        //for use later [3,4]
        case (3):
            // if (in_encoder_module)
            // {
            //     printEncoderModuleOptions();
            //     if (hasUserInput())
            //         switch (char_in)
            //         {
            //         case ('q'):
            //             *p_serial << ATERM_CLEAR_SCREEN << ATERM_BKG_WHITE
            //                       << PMS("->Selected: ") << char_in << endl
            //                       << endl
            //                       << endl
            //                       << PMS("\t->Returning to Mission Control.. ")
            //                       << endl
            //                       << PMS("\t->Releasing Encoder..") << endl;
            //             transition_to(0);
            //             in_main_motor_module = true;
            //             in_encoder_module = false;
            //             resetMenus();
            //             *p_serial
            //                     << ATERM_CLEAR_SCREEN
            //                     << ATERM_CURSOR_TO_YX(1, 1) << endl;

            //             break;
            //         case ('r'):
            //             //constant refreshing already
            //             transition_to(1)
            //             // in_imu_module = true;
            //             break;
            //         default:
                        
            //             break;
            //         }
            // }
            // break;
            break;
        case (4):
            // if (in_imu_module)
            // {
            //     printIMUModuleOptions();
            //     if (hasUserInput())
            //         switch (char_in)
            //         {
            //         case ('q'):
            //             *p_serial << ATERM_CLEAR_SCREEN
            //                       << PMS("->Selected: ") << char_in << endl
            //                       << endl
            //                       << endl
            //                       << PMS("\t->Returning to Mission Control.. ")
            //                       << endl
            //                       << PMS("\t->Releasing IMU..") << endl;
            //             transition_to(0);

            //             in_imu_module = false;
            //             resetMenus();   

            //             break;
            //             default:
            //             break;
            //             *p_serial << ATERM_CLEAR_SCREEN;
            //         }




            //         case ('r'):
                        
            //             break;
            //         default:
            //             break;
            //         }
            // }
            break;
        // Drive Mode
        case (5):
            if (in_drive_mode)
            {
                printDriveModeOptions();
                // printDashBoard();

                *p_serial
                        << ATERM_CURSOR_TO_YX(19, 1)
                        << ATERM_ERASE_IN_LINE(0)
                        << gear_state -> get()
                        << PMS ("\t\t")
                        << motor_setpoint -> get()
                        << PMS("\t")
                        << PMS("\t")
                        << motor_power -> get()
                        << PMS("\t")
                        << PMS("\t")
                        << encoder_count -> get()
                        << PMS("\t")
                        << ATERM_CURSOR_TO_YX(23, 1)
                        << ATERM_ERASE_IN_LINE(0)
                        << steering_power -> get()
                        << PMS("    \t")
                        << encoder_ticks_per_task -> get()
                        << PMS("\t")
                        << PMS("\t")
                        << x_joystick -> get()
                        << PMS("\t")
                        << PMS("\t")
                        << y_joystick -> get()
                        << PMS("\t");
                // *p_serial << encoder_ticks_per_task -> get() << endl;

            }
            break;
        //JoyStick test
        case (6):
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
        delay_ms (1000);
    }

}
