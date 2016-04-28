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
//Set these defines in case we want to change things later.

/// Value for BRAKE directive
#define BRAKE 0
/// Value for SETPOWER directive
#define SETPOWER 1
/// Value for FREEWHEEL directive
#define FREEWHEEL 2
/// Value for POTENTIOMETER directive
#define POTENTIOMETER 3

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
    //Set the in main motor flag to false because we start in main menu
    in_main_motor_module = false;
    //Set both motor directives to freewheel because no command has been given
    local_motor1_directive = FREEWHEEL;
    local_motor2_directive = FREEWHEEL;
    //Intialize the power to ZERO
    local_motor1_power = 0;
    local_motor2_power = 0;



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
                            << endl
                            << endl
                            << PMS ("\t->Switching to Encoder Module..")
                            << endl
                            << PMS ("\t->Clearing Registers and Menus..")
                            << endl
                            << PMS ("\t->Intializing Encoder..")
                            << endl;

                    // Reset the visible menu flag
                    resetMenus();
                    // Sets the variable to true so it doesn't go into single module yet
                    in_encoder_module = true;
                    // go to case 1 over all
                    transition_to(3);
                    break;
                // The 't' command asks what time it is right now
                case ('t'):
                    *p_serial << (a_time.set_to_now ()) << endl;
                    break;

                // The 's' command asks for version and status information
                case ('s'):
                    show_status ();
                    break;

                // The 'd' command has all the tasks dump their stacks
                case ('d'):
                    print_task_stacks (p_serial);
                    break;

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
                        setMotor(motor_select->get(), 0, FREEWHEEL);
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
            if (in_encoder_module)
            {
                printEncoderModuleOptions();
                if (hasUserInput())
                    switch (char_in)
                    {
                    case ('q'):
                        *p_serial << PMS("->Selected: ") << char_in << endl
                                  << endl
                                  << endl
                                  << PMS("\t->Returning to Mission Control.. ")
                                  << endl
                                  << PMS("\t->Releasing Encoder..") << endl;
                        transition_to(0);
                        in_main_motor_module = true;
                        in_encoder_module = false;
                        resetMenus();

                        break;
                    case ('r'):

                        *p_serial << PMS("Encoder Count: ") << encoder_count -> get() << endl;
                        *p_serial << PMS("Encoder Count / sec: ") << count_per_sec -> get() << endl;
                        *p_serial << PMS("Error Count: ") << encoder_errors -> get() << endl;
                        *p_serial << PMS("State: ") << the_state -> get() << endl;
                        *p_serial << endl << PMS("\t\t-> press 'r' to refresh ")<<endl<<endl;
                        break;
                    default:
                        break;
                    }
            }
            break;
        case (4):
            break;
        case (5):
            *p_serial << PMS("This Demo has been disabled. Sorry") << endl << PMS("\t->Returning to main..") << endl;
            resetMenus();
            transition_to(0);
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

/**
 * @brief      Method prints out the main menu, which is the first thing the
 *             user sees.
 *
 * @details    Menu is designed so that Control and testing modules are one
 *             part and all other operations like the ones included in the
 *             given version of this class are sectioned off accordingly.
 */
void task_user::printMainMenu (void)
{
    if (!is_menu_visible)
    {
        *p_serial
                << endl
                //<< PMS ("Motor\t\tStatus\t\tPower\t\tDirection")
                << PROGRAM_VERSION << endl << endl << PMS("\t\t\t    ") << PMS (__DATE__) << endl
                << PMS ("|\t\t    Mission Control Program v1.0    \t\t|") << endl
                << PMS ("|---------------------------------------------------------------|") << endl
                << PMS ("|\t\t 'm'     Enter Motor Control Module   \t\t|") << endl
                << PMS ("|\t\t 'e'     Enter Encoder Control Module \t\t|") << endl
                << PMS ("|\t\t 'h'     Display all options          \t\t|") << endl
                << PMS ("|\t\t 'Ctl-C' Reset AVR                    \t\t|") << endl;

        is_menu_visible = true;
    }

}

/**
 * @brief      Method prints out the 'Main Motor Module' menu
 * @details    Only print the menu if the is_menu_visible flag is false. This
 *             is so that it doesn't inifinitely print as the user is trying
 *             to input a value or trying to read it.
 */
void task_user::printMotorMenu (void)
{
    if (!is_menu_visible)
    {
        *p_serial
                << endl << endl
                //<< PMS ("Motor\t\tStatus\t\tPower\t\tDirection")
                << PMS ("|\t\t    Main Motor Control Module    \t\t|") << endl
                << PMS ("|---------------------------------------------------------------|") << endl
                << PMS ("|\t\t '1'     to operate Motor 1      \t\t|") << endl
                << PMS ("|\t\t '2'     to operate Motor 2      \t\t|") << endl
                << PMS ("|\t\t 'q'     Return to Main Menu     \t\t|") << endl;
        is_menu_visible = true;
    }
}

/**
 * @brief      Method prints out the 'Single Motor Control Module' menu
 *
 * @details    Only print the menu if the is_menu_visible flag is false. This
 *             is so that it doesn't inifinitely print as the user is trying
 *             to input a value or trying to read it.
 */
void task_user::printSingleMotorOptions(void)
{
    if (is_menu_visible == false)
    {   *p_serial << endl;
        *p_serial << PMS ("|\t\t   Single Motor Control Module   \t\t|") << endl;
        *p_serial << PMS ("|---------------------------------------------------------------|") << endl;
        *p_serial << PMS ("|\t\t 's'     Set the motor power     \t\t|") << endl;
        *p_serial << PMS ("|\t\t 'b'     Apply the motor brake   \t\t|") << endl;
        *p_serial << PMS ("|\t\t 'f'     Freewheel motor         \t\t|") << endl;
        *p_serial << PMS ("|\t\t 'p'     Enter Potentiometer Mode\t\t|") << endl;
        *p_serial << PMS ("|\t\t 'q'     Return to previous menu \t\t|") << endl;
        is_menu_visible = true;
    }
}
/**
 * @brief      Method prints out useful information about both motors.
 *
 * @details    Only print the menu if the is_menu_visible flag is false. This
 *             is so that it doesn't inifinitely print as the user is trying
 *             to input a value or trying to read it. Furthermore, it prints
 *             out 'status', 'power', & 'direction'
 */
void task_user::printDashBoard(void)
{
    if (!is_menu_visible)
    {

        const char* status;
        const char* direction;
        //int16_t temp_power;
        //if(motor_select)
        if (local_motor1_directive == 0)
        {
            status = "Stopped\t";
            direction = "N/A";
        }
        else if (local_motor1_directive == 1)
        {
            status = "Running\t";
        }
        else if (local_motor1_directive == 2)
        {
            status = "Frwheel\t";
        }
        else if (local_motor1_directive == 3)
        {
            status = "PotMeter";

        }
        else
        {
            status = "Error\t";
        }

        if (!(local_motor1_directive == BRAKE))
        {

            if (local_motor1_power < 0)
            {
                direction = "Reverse";
            }
            else if (local_motor1_power > 0)
            {
                direction = "Forwards";
            }

        }
        else
        {
            direction = "N/A";
        }


        *p_serial << endl;
        *p_serial
        //<< PMS ("")
                << PMS ("-------\t\t--------\t-------\t\t-----------")
                << endl
                << PMS ("Motor\t\tStatus\t\tPower\t\tDirection")
                << endl
                << PMS ("-------\t\t--------\t-------\t\t-----------")
                << endl
                << PMS ("Motor 1\t\t") << status << PMS("\t") << local_motor1_power << PMS("\t\t") << direction << endl;
        if (local_motor2_directive == BRAKE)
        {status = "Stopped\t";}
        else if (local_motor2_directive == SETPOWER)
        {
            status = "Running\t";
        }
        else if (local_motor2_directive == FREEWHEEL)
        {
            status = "Frwheel\t";
        }
        else if (local_motor2_directive == POTENTIOMETER)
        {
            status = "PotMeter";
            //*local_motor2_power;
        }
        else
        {
            status = "Error\t";
        }
        if (!(local_motor2_directive == BRAKE))
        {

            if (local_motor2_power < 0)
            {
                direction = "Reverse";
            }
            else if (local_motor2_power > 0)
            {
                direction = "Forwards";
            }

        }
        else
        {
            direction = "N/A";
        }
        *p_serial
                << endl
                << PMS ("Motor 2\t\t") << status << PMS("\t") << local_motor2_power << PMS("\t\t") << direction << endl;

        is_menu_visible = true;
    }
}

/**
 * @brief      This method contains the old main menu that has extra
 *             information and avaliable options.
 */
void task_user::print_help_message (void)
{
    *p_serial << endl << PROGRAM_VERSION << PMS (" help") << endl;
    *p_serial << PMS ("  t:     Show the time right now") << endl;
    *p_serial << PMS ("  s:     Version and setup information") << endl;
    *p_serial << PMS ("  d:     Stack dump for tasks") << endl;
    *p_serial << PMS ("  n:     Enter a number (demo)") << endl;
    *p_serial << PMS ("  Ctl-C: Reset the AVR") << endl;
    *p_serial << PMS ("  h:     HALP!") << endl;
}

/**
 * @brief      Method checks if the user has made an input
 *
 * @return     returns TRUE if the user has made an input and places that
 *             charater into the class variable 'char_in'. Returns FALSE
 *             otherwise.
 */
bool task_user::hasUserInput(void)
{
    if (p_serial->check_for_char ())            // If the user typed a
    {   // character, read
        char_in = p_serial->getchar ();
        return true;
    }
    return false;
}

/**
 * @brief      Method gets the number being entered by the user.
 *
 * @details    This method uses a loop to constantly look for input until a
 *             return or escape symbol occurs. It then  places the extracted
 *             negative or positive value in local class varible
 *             'number_entered'.
 */
void task_user::getNumberInput(void)
{
    number_entered = 0;
    bool negative = false;
    while (1)
    {
        //*p_serial << PMS ("in case 5")<<endl;
        if (p_serial->check_for_char ())        // If the user typed a
        {   // character, read
            char_in = p_serial->getchar ();     // the character

            // Respond to numeric characters, Enter or Esc only. Numbers are
            // put into the numeric value we're building up
            if (char_in >= '0' && char_in <= '9')
            {
                *p_serial << char_in;
                number_entered *= 10;
                number_entered += char_in - '0';
            }
            else if (char_in == '-')
            {
                //as soon as the user enters a negative sign we multiply by -1.
                *p_serial << char_in;
                negative = true;

            }
            else if (char_in == 'q')
            {
                //return so caller can handle the quit request.
                return;
            }
            // Carriage return is ignored; the newline character ends the entry
            else if (char_in == 10)
            {
                *p_serial << "\r";
            }
            // Carriage return or Escape ends numeric entry
            else if (char_in == 13 || char_in == 27)
            {

                if (negative)
                {
                    number_entered *= -1;
                }
                return;
            }
            else
            {
                *p_serial << PMS ("<invalid char \"") << char_in
                          << PMS ("\">");

            }
        }

        // Check the print queue to see if another task has sent this task
        // something to be printed
        else if (p_print_ser_queue->check_for_char ())
        {
            p_serial->putchar (p_print_ser_queue->getchar ());
        }
    }

}
//-----------------------------------------------------------------------------
/** This method displays information about the status of the system, including the
 *  following:
 *    \li The name and version of the program
 *    \li The name, status, priority, and free stack space of each task
 *    \li Processor cycles used by each task
 *    \li Amount of heap space free and setting of RTOS tick timer
 */
void task_user::show_status (void)
{
    time_stamp the_time;                    // Holds current time for printing

    // First print the program version, compile date, etc.
    *p_serial << endl << PROGRAM_VERSION << PMS (__DATE__) << endl
              << PMS ("System time: ") << the_time.set_to_now ()
              << PMS (", Heap: ") << heap_left() << "/" << configTOTAL_HEAP_SIZE
#ifdef OCR5A
              << PMS (", OCR5A: ") << OCR5A << endl << endl;
#elif (defined OCR3A)
              << PMS (", OCR3A: ") << OCR3A << endl << endl;
#else
              << PMS (", OCR1A: ") << OCR1A << endl << endl;
#endif

    // Have the tasks print their status; then the same for the shared data items
    print_task_list (p_serial);
    *p_serial << endl;
    print_all_shares (p_serial);

}

/**
 * @brief      Method sets motor to the correct state as specified by user.
 *
 * @details    This method sets the global registers to the correct values so
 *             that 'task_motor' can affect the correct motor. Also sets
 *             appropriate local values to store the state of this motor so
 *             that when 'printDashBoard()' is called it can print info on
 *             each motor because the global registers can only hold info for
 *             one motor at a time.
 *
 * @param[in]  motor_id  The motor select identifier
 * @param[in]  power     The power setting specified by the user in the run()
 * @param[in]  direct    The directive, or what we want the motor to do
 */
void task_user::setMotor(uint8_t motor_id, int16_t power, uint8_t direct)
{
    //place in global taskshares
    motor_select -> put(motor_id);
    motor_directive ->put(direct);
    motor_power->put(power);
    //work with local vars
    if (motor_id == 1)
    {
        local_motor1_directive = direct;
        local_motor1_power = power;
        return;
    }
    else if (motor_id == 2)
    {
        local_motor2_directive = direct;
        local_motor2_power = power;
        return;
    }
}
/**
 * @brief      Method checks if the motor selected is a valid input.
 *
 * @param[in]  motor_number  The number number entered by the user.
 *
 * @return     returns TRUE if the motor value is valid, FALSE otherwise
 */
bool task_user::isValidMotor(int16_t motor_number)
{
    if (motor_number == 1 || motor_number == 2)
    {
        return true;
    }
    return false;
}

/**
 * @brief      Method resets the is_menu_visible flag so that another menu may
 *             be printed.
 */
void task_user::resetMenus(void)
{
    is_menu_visible = false;
}

/**
 * @brief      Method prints out the 'Encoder Control Module' menu
 *
 * @details    Only print the menu if the is_menu_visible flag is false. This
 *             is so that it doesn't inifinitely print as the user is trying
 *             to input a value or trying to read it.
 */
void task_user::printEncoderModuleOptions(void)
{
    if (is_menu_visible == false)
    {   *p_serial << endl;
        *p_serial << PMS ("|\t\t        Encoder Control Module          \t\t|") << endl;
        *p_serial << PMS ("|---------------------------------------------------------------|") << endl;
        *p_serial << PMS ("|\t\t 'r'    Refresh the data                \t\t|") << endl;
        *p_serial << PMS ("|\t\t 'q'    quit to main menu               \t\t|") << endl;
        is_menu_visible = true;
    }
}