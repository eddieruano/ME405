//**************************************************************************************
/** @file task_user.cpp
 *    This file contains source code for a user interface task for a ME405/FreeRTOS
 *    test suite.
 *
 *  Revisions:
 *    @li 09-30-2012 JRR Original file was a one-file demonstration with two tasks
 *    @li 10-05-2012 JRR Split into multiple files, one for each task
 *    @li 10-25-2012 JRR Changed to a more fully C++ version with class task_user
 *    @li 11-04-2012 JRR Modified from the data acquisition example to the test suite
 *    @li 01-04-2014 JRR Changed base class names to TaskBase, TaskShare, etc.
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

#include <avr/io.h>                         // Port I/O for SFR's
#include <avr/wdt.h>


#include "task_user.h"                      // Header for this file
#define BRAKE 0
#define SETPOWER 1
#define FREEWHEEL 2
#define POTENTIOMETER 3
#define VAR 3


/** This constant sets how many RTOS ticks the task delays if the user's not talking.
 *  The duration is calculated to be about 5 ms.
 */
const TickType_t ticks_to_delay = ((configTICK_RATE_HZ / 1000) * 5);


//-------------------------------------------------------------------------------------
/** This constructor creates a new data acquisition task. Its main job is to call the
 *  parent class's constructor which does most of the work.
 *  @param a_name A character string which will be the name of this task
 *  @param a_priority The priority at which this task will initially run (default: 0)
 *  @param a_stack_size The size of this task's stack in bytes
 *                      (default: configMINIMAL_STACK_SIZE)
 *  @param p_ser_dev Pointer to a serial device (port, radio, SD card, etc.) which can
 *                   be used by this task to communicate (default: NULL)
 */

task_user::task_user (const char* a_name,
                      unsigned portBASE_TYPE a_priority,
                      size_t a_stack_size,
                      emstream* p_ser_dev
                     )
    : TaskBase (a_name, a_priority, a_stack_size, p_ser_dev)
{
    // Nothing is done in the body of this constructor. All the work is done in the
    // call to the frt_task constructor on the line just above this one
    is_menu_visible = false;
    //number_entered = 0;
    in_main_motor_module = false;
    in_single_motor_module = false;
    local_motor1_directive = FREEWHEEL;
    local_motor2_directive = FREEWHEEL;
    local_motor1_power = 0;
    local_motor2_power = 0;


}


void task_user::run (void)
{
    //*p_serial << PMS("in the user")<<endl;
    time_stamp a_time;                      // Holds the time so it can be displayed
    number_entered = 0;            // Holds a number being entered by user
    //uint8_t redirect = 0;

    for (;;)
    {
        switch (state)
        {
        // Initialize first task where we wait for
        case (0):
            printMainMenu();

            if (p_serial->check_for_char ())            // If the user typed a
            {   // character, read
                char_in = p_serial->getchar ();         // the character

                // In this switch statement, we respond to different characters as
                // commands typed in by the user
                switch (char_in)
                {
                // The 't' command asks what time it is right now
                case ('m'):
                    //redirect = 1;
                    *p_serial << PMS("->Selected: ") << char_in;
                    *p_serial
                            << endl
                            << endl
                            << PMS ("\t->Switching to Motor Module..") << endl
                            << PMS ("\t->Clearing Registers and Menus..") << endl
                            << PMS ("\t->Intializing Motors..") << endl;

                    //This handles the
                    resetMenus();
                    in_main_motor_module = true;
                    transition_to(1);
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
                    *p_serial << PMS ("Enter decimal numeric digits, "
                                      "then RETURN or ESC") << endl;
                    number_entered = 0;
                    //redirect = 0;
                    transition_to (5);
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
        //
        // Motor Conrol BASE module
        case (1):
            if (in_main_motor_module) //first time here run menu and get input
            {
                printMotorMenu();
                resetMenus();
                in_main_motor_module = false;
                in_single_motor_module = true;

                getNumberInput();

                if (char_in == 'q')
                {
                    *p_serial << endl << PMS("->Selected: ") << number_entered;
                    *p_serial
                            << endl
                            << endl
                            << PMS("\t->Switching to Control of Motor ") << number_entered << dec << PMS(".") << endl
                            << PMS ("\t->Return to the Main Motor Module to swap Motors.") << endl;
                    local_motor_select = number_entered;
                    transition_to(1);
                }
                else if (isValidMotor(number_entered))
                {
                    *p_serial << PMS("->Selected: ") << char_in << endl
                              << endl << endl
                              << PMS("\t->Returning to Mission Control.. ")
                              << endl
                              << PMS("\t->Releasing Motors..") << endl;

                }
                else
                {
                    *p_serial << PMS("Try Again. ") << endl;
                    in_main_motor_module = true;
                    in_single_motor_module = false;
                }
                break;
            }
            if (in_single_motor_module) //already were here now we go deeper
            {


                printSingleMotorOptions();
                // They now select the motor task
                if (getUserInput())
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
                        setMotor(local_motor_select, number_entered, 1);
                        *p_serial 
                            << endl << endl 
                            << PMS ("\tPower set at ") << number_entered 
                            << PMS(". ") << endl;

                        resetMenus();
                        printDashBoard();
                        *p_serial 
                            << endl 
                            << PMS("->Choose Motor ") << local_motor_select 
                            << PMS(" operation: ") << endl;

                        //transition_to(2);
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

                        setMotor(local_motor_select, number_entered, 0);
                        *p_serial 
                            << endl << PMS("->Choose Motor ") 
                            << local_motor_select 
                            << PMS(" operation: ") << endl;
                        //set_brake(motor_select->get(), number_entered)
                        break;
                    case ('f'):
                        *p_serial 
                            << PMS("->Selected: ") << char_in << endl;
                        *p_serial 
                            << PMS ("\t->Releasing Motor..") << endl;
                        setMotor(motor_select->get(), 0, 2);
                        *p_serial 
                            << endl << PMS("->Choose Motor ") 
                            << local_motor_select << PMS(" operation: ") 
                            << endl;;
                        //getNumberInput();
                        break;
                    case ('p'):
                        *p_serial 
                            << endl << PMS("->Selected: ") 
                            << char_in << endl 
                            << PMS("\t->Entering Potentiometer Control... ") << endl;
                        *p_serial << PMS ("\t->Potentiometer Activated.") << endl << endl
                                  << PMS ("\t->Press 'q' to return to the Motor ") << local_motor_select << PMS(" Control")
                                  << endl << PMS ("\t->Press 'r' to refresh the DashBoard ") << endl;
                        transition_to(2);
                        resetMenus();
                        break;
                    case ('q'):
                        *p_serial 
                            << PMS("->Selected: ") << char_in << endl 
                            << PMS(" Returning to Main Motor Module.. ") 
                            << endl;
                        transition_to(1);
                        resetMenus();
                        in_single_motor_module = false;
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

        case (2):
            if (getUserInput())
            {
                if (char_in  == 'q')
                {
                    *p_serial << PMS("->Selected: ") << char_in << endl;
                    transition_to(1);
                    resetMenus();
                    break;
                    //in_single_motor_module = true;
                }
                if (char_in == 'r')
                {
                    *p_serial << PMS("->Selected: ") << char_in << endl;
                    resetMenus();
                }
            }
            setMotor(local_motor_select, 0, POTENTIOMETER);
            printDashBoard();
            break;

        case (3):
            if (getUserInput())
            {
                motor_select -> put(1);
                //print_help_message();
                motor_directive -> put(1);
                motor_power -> put(128);

                transition_to(1);
            }
            break;
        case (4):
            *p_serial << PMS ("M2") << endl;
            motor_select -> put(2);
            motor_directive -> put(1);
            motor_power -> put(128);
            transition_to(2);
            break;
        case (5):


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


//-------------------------------------------------------------------------------------
/** This method prints a simple help message.
 */

/**
 * @brief      { function_description }
 */
void task_user::printMainMenu (void)
{
    if (!is_menu_visible)
    {
        *p_serial
                << endl << endl
                //<< PMS ("Motor\t\tStatus\t\tPower\t\tDirection")
                << PMS ("|\t\t    Mission Control Program v1.0    \t\t|") << endl
                << PMS ("|---------------------------------------------------------------|") << endl
                << PMS ("|\t\t 'm'     Enter Motor Control Module \t\t|") << endl
                << PMS ("|\t\t 'Ctl-C' Reset AVR                  \t\t|") << endl;

        is_menu_visible = true;
    }

}

/**
 * @brief      { function_description }
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

void task_user::printDashBoard(void)
{
    if (!is_menu_visible)
    {

        const char* status;
        const char* direction;
        int16_t temp_power = motor_power -> get();
        local_motor1_power = motor_power ->get();
        if (local_motor1_directive == 0)
        {status = "Stopped";}
        else if (local_motor1_directive == 1)
        {
            status = "Running";
        }
        else if (local_motor1_directive == 2)
        {
            status = "Frwheel";
        }
        else if (local_motor1_directive == 3)
        {
            status = "PotMeter";

        }
        else
        {
            status = "Error";
        }

        if (temp_power < 0)
        {
            direction = "Reverse";
        }
        else if (temp_power > 0)
        {
            direction = "Forwards";
        }
        else
        {
            direction = "N/A";
        }

        *p_serial << endl;
        *p_serial
                << PMS ("")
                << PMS ("Motor\t\tStatus\t\tPower\t\tDirection")
                << endl
                << PMS ("-------\t\t--------\t-------\t\t-----------")
                << endl
                << PMS ("Motor 1\t\t") << status << PMS("\t\t") << temp_power << PMS("\t\t") << direction << endl;
        if (local_motor2_directive == 0)
        {status = "Stopped";}
        else if (local_motor2_directive == 1)
        {
            status = "Running";
        }
        else if (local_motor2_directive == 2)
        {
            status = "Frwheel";
        }
        else if (local_motor2_directive == VAR)
        {
            status = "PotMeter";
            local_motor2_power = motor_power -> get();
        }
        else
        {
            status = "Error";
        }
        temp_power = local_motor2_power;
        if (temp_power < 0)
        {
            direction = "Reverse";
        }
        else if (temp_power > 0)
        {
            direction = "Forwards";
        }
        else
        {
            direction = "N/A";
        }
        *p_serial
        // << PMS ("")
        // << PMS ("Motor\t\tStatus\t\tPower\t\tDirection")
        //<< endl
                << PMS ("-------\t\t--------\t-------\t\t-----------")
                << endl
                << PMS ("Motor 2\t\t") << status << PMS("\t\t") << temp_power << PMS("\t\t") << direction << endl;


        is_menu_visible = true;
    }
}

void task_user::print_help_message (void)
{
    *p_serial << PROGRAM_VERSION << PMS (" help") << endl;
    *p_serial << PMS ("  t:     Show the time right now") << endl;
    *p_serial << PMS ("  s:     Version and setup information") << endl;
    *p_serial << PMS ("  d:     Stack dump for tasks") << endl;
    *p_serial << PMS ("  n:     Enter a number (demo)") << endl;
    *p_serial << PMS ("  Ctl-C: Reset the AVR") << endl;
    *p_serial << PMS ("  h:     HALP!") << endl;
}


//-------------------------------------------------------------------------------------
/** This method displays information about the status of the system, including the
 *  following:
 *    \li The name and version of the program
 *    \li The name, status, priority, and free stack space of each task
 *    \li Processor cycles used by each task
 *    \li Amount of heap space free and setting of RTOS tick timer
 */


bool task_user::getUserInput(void)
{
    if (p_serial->check_for_char ())            // If the user typed a
    {   // character, read
        char_in = p_serial->getchar ();
        return true;
    }
    return false;
}

void task_user::getNumberInput()
{
    number_entered = 0;
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
                number_entered *= -1;
            }
            else if (char_in == 'q')
            {
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


void task_user::resetMenus(void)
{
    is_menu_visible = false;
}


void task_user::setMotor(uint8_t motor_id, uint32_t power, int8_t direct)
{
    motor_select -> put(motor_id);
    motor_directive ->put(direct);
    motor_power->put(power);
    // *p_serial << motor_select -> get()<<endl<<motor_directive->get()<<endl<<motor_power->get()<<endl;
    if (motor_id == 1)
    {
        local_motor1_directive = direct;
        local_motor1_power = power;
        if (direct == 3)
        {
            local_motor1_power = VAR;
        }
        return;
    }
    else if (motor_id == 2)
    {
        local_motor2_directive = direct;
        local_motor2_power = power;
        if (direct == 3)
        {
            local_motor2_power = VAR;
        }
        return;
    }



}


bool task_user::isValidMotor(uint32_t motor_number)
{
    if (motor_number == 1 || motor_number == 2)
    {
        return true;
    }
    return false;
}