//======================================================================================
/** @file encoder.h
 *    This file contains a header file for the encoder handler class.
 *
 *  Revisions:
 *    19 April 2016: Initial draft
 * 
 *  Author(s): Anthony Lombardi
*/


/* 
 * in case this file is included multiple times, we want the preprocessor 
 * to remove this code if it already read the define in the next line
 * of code below.
*/
#ifndef ENCODER 

#define ENCODER

#include "emstream.h"                       // Header for serial ports and devices
#include "FreeRTOS.h"                       // Header for the FreeRTOS RTOS
#include "task.h"                           // Header for FreeRTOS task functions
#include "queue.h"                          // Header for FreeRTOS queues
#include "semphr.h"                         // Header for FreeRTOS semaphores


//-------------------------------------------------------------------------------------
/** @brief   This class will run the encoder
 *  @details This class sets up the correct things
 *  
 */
class encoder
{
    private:
        //no private variables needed.

    protected:
        /// The encoder class uses this pointer to the serial port
        emstream* serial_PORT;
        /// Stores the number of ticks/transitions counted by the encoder
        volatile uint16_t count;
        /// Tracks the previous state of channels A and B as bits
        volatile uint8_t  prevState;

    //set the public constructor and the public methods
    public:
        encoder ();

        // prototype for the get_count method of encoder
        void get_count (void);
        // prototypre for the get_direction method of encoder
        uint8_t get_direction (void);
        // prototype for the set_index method of encoder
        /// If passed no argument, the index is set to 0 by default
        void set_index (void);
        void set_index (uint16_t);

}; // end of class


// This operator prints useful info about particular given encoder
emstream& operator << (emstream&, encoder&);

//closes the removal of the code
#endif // ifdef ENCODER