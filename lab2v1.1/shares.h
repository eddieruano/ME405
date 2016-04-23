//*************************************************************************************
/** @file shares.h
 *    This file contains extern declarations for queues and other inter-task data
 *    communication objects used in a ME405/507/FreeRTOS project. 
 *
 *  Revisions:
 *    @li 09-30-2012 JRR Original file was a one-file demonstration with two tasks
 *    @li 10-05-2012 JRR Split into multiple files, one for each task plus a main one
 *    @li 10-29-2012 JRR Reorganized with global queue and shared data references
 *    @li 01-04-2014 JRR Re-reorganized, allocating shares with new now
 *
 *  License:
 *		This file is copyright 2015 by JR Ridgely and released under the Lesser GNU 
 *		Public License, version 2. It intended for educational use only, but its use
 *		is not limited thereto. */
/*		THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 *		AND	ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * 		IMPLIED 	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * 		ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
 * 		LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUEN-
 * 		TIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS 
 * 		OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
 * 		CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
 * 		OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * 		OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */
//*************************************************************************************

// This define prevents this .h file from being included multiple times in a .cpp file
#ifndef _SHARES_H
#define _SHARES_H

//-------------------------------------------------------------------------------------
// Externs:  In this section, we declare variables and functions that are used in all
// (or at least two) of the files in the data acquisition project. Each of these items
// will also be declared exactly once, without the keyword 'extern', in one .cpp file
// as well as being declared extern here. 

// This queue allows tasks to send characters to the user interface task for display.
extern TextQueue* p_print_ser_queue;

/// This declares a taskshare which indicates that this variable is a shared 
/// variable. This variable will hold the duty cycle of a motor. 
extern TaskShare<int16_t>* motor_power;

/// This declares a taskshare which indicates that this variable is a shared 
/// variable. This variable will hold the duty cycle of a motor.
extern TaskShare<int16_t>* brake_power;

/// This declares a taskshare which indicates that this variable is a shared 
/// variable. This variable will hold the direction of the motor. 
/// -1 Indicates REVERSE
///  0 Indicates STOPPED
///  1 Indicates FORWARDS
///  2 Indicates FREEWHEEL
extern TaskShare<int8_t>* motor_direction;

/// This declares a taskshare which indicates that this variable is a shared 
/// variable. This variable will hold the command that the user wishes the motor
/// to do.
/// -1 Indicates CHANGE_DIRECTION
///  0 Indicates STOP W/ brake_power
///  1 Indicates SET POWER
///  2 Indicates FREEWHEEL
extern TaskShare<int8_t>* motor_command;
/// This declares a taskshare which indicates that this variable is a shared 
/// variable. This variable will hold the select for each motor.
/// 0 Indicates Motor 0
/// 1 Indicates Motor 1
extern TaskShare<uint8_t>* motor_select;




#endif // _SHARES_H
