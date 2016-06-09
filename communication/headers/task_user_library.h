//HOLDS functions task user

/// Value for BRAKE directive
#define BRAKE 0
/// Value for SETPOWER directive
#define SETPOWER 1
/// Value for FREEWHEEL directive
#define FREEWHEEL 2
/// Value for POTENTIOMETER directive
#define POTENTIOMETER 3

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
                << ATERM_BKG_WHITE
                << ATERM_TXT_RED
                << endl
                //<< PMS ("Motor\t\tStatus\t\tPower\t\tDirection")
                << PROGRAM_VERSION << endl << endl << PMS("\t\t\t    ") << PMS (__DATE__) << endl
                <<  ATERM_TXT_BLACK
                << PMS ("|\t\t    Mission Control Program v1.0    \t\t|") << endl
                << PMS ("|---------------------------------------------------------------|") << endl
                //<< ATERM_BKG_GREEN
                << PMS ("|\t\t         Enter Motor Control Module   \t\t|")
                << ATERM_BKG_WHITE
                << endl
                << PMS ("|\t\t         Enter Encoder Control Module \t\t|") << endl
                << PMS ("|\t\t         Enter IMU Control Module     \t\t|") << endl
                << PMS ("|\t\t         Enter Drive Mode             \t\t|") << endl
                << PMS ("|\t\t         Display all options          \t\t|") << endl
                << PMS ("|\t\t         Reset AVR                    \t\t|") << endl;


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
 * @brief      Method resets the is_menu_visible flag so that another menu may
 *             be printed.
 */
void task_user::resetMenus(void)
{
    is_menu_visible = false;
}



