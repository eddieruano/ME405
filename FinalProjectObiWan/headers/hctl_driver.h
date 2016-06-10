//*****************************************************************************
/** @file htcl_driver.h
 *  @brief     This file contains a class to handle reading data from an 
 *             HCTL-2000 encoder counter IC
 *
 *  @details   This is a task class that will control the operation of
 *             its own motor driver given to it as a pointer and initiated in
 *             the main() function. it lays out the logic necessary to make
 *             commands from task_user to the motor driver happen.
 *
 *  @author(s) Anthony
 *
 *  Revisions: @ 5/3/2016 <<EDD>> changed name and fixed comments to
 *             traditional style
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
//*****************************************************************************

// This define prevents this .H file from being included multiple times in a .CPP file
#ifndef _AVR_HCTL_H_
#define _AVR_HCTL_H_

#include "emstream.h"               // Header for serial ports and devices
#include "FreeRTOS.h"               // Header for the FreeRTOS RTOS
#include "task.h"                   // Header for FreeRTOS task functions
#include "queue.h"                  // Header for FreeRTOS queues
#include "semphr.h"                 // Header for FreeRTOS semaphores


/**
 * @brief      This class will run the encoder counter IC on an AVR processor.
 * 
 * @details    This class also has the prototypes form the correct public
 *             constructors and methods.
 */
class hctl_driver
{
protected:
    /// The HCTL class uses this pointer to the serial port to print information
    emstream* ptr_to_serial;

    /// the data bus port register (where we'll look for encoder data)
    volatile uint8_t* data_PORT;
    /// the port register for the !OE pin
    volatile uint8_t* oe_PORT;
    /// the pin number for the !OE pin
    uint8_t oe_pin;
    /// the port register for the SEL oe_pin
    volatile uint8_t* sel_PORT;
    /// the pin number for the SEL oe_pin
    uint8_t sel_pin;

public:
    // The constructor sets up the HCTL for use. The "= NULL" part is a
    // default parameter, meaning that if that parameter isn't given on the line
    // where this constructor is called, the compiler will just fill in "NULL".
    // In this case that has the effect of turning off diagnostic printouts
    hctl_driver (
        emstream*,
        volatile uint8_t*,
        volatile uint8_t*,
        uint8_t,
        volatile uint8_t*,
        uint8_t
    );

    /// This function reads the HCTL once, returning the result as an unsigned integer; it should be called from within a normal task, not an ISR
    uint16_t read ();
    /// returns the data port used by the HCTL.
    volatile uint8_t* get_data_port () { return data_PORT; }
    /// returns the port for the !OE pin.
    volatile uint8_t* get_oe_port () { return oe_PORT; }
    /// returns the port for the SEL pin.
    volatile uint8_t* get_sel_port () { return sel_PORT; }
    /// returns the pin number on oe_PORT being used by the !OE pin.
    uint8_t get_oe_pin () { return oe_pin; }
    /// returns the pin number on sel_PORT being used by the SEL pin.
    uint8_t get_sel_pin () { return sel_pin; }

}; // end of class hctl


// This operator prints the HCTL (see file hctl.cpp for details). It's not
// a part of class hctl, but it operates on objects of class hctl
emstream& operator << (emstream&, hctl_driver&);

#endif // _AVR_HCTL_H_
