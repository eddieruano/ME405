//*****************************************************************************
/** @file shares.h
 *
 *  @brief     This file contains extern declarations for queues and other
 *             inter-task data communication objects used in a 
 *             ME405/507/FreeRTOS project
 *
 *  @details   This is a task class that will control the operation of
 *             its own motor driver given to it as a pointer and initiated in
 *             the main() function. it lays out the logic necessary to make
 *             commands from task_user to the motor driver happen.
 *
 *  @author Eddie Ruano
 *
 *  Revisions: @li 4/20/2016  ER added more shares for the task_motor 
 *             operations
 *             @li 01-04-2014 JRR Re-reorganized, allocating shares with new
 *             now
 *             @li 09-30-2012 JRR Original file was a one-file demonstration
 *             with two tasks
 *             @li 10-05-2012 JRR Split into multiple files, one for each task 
 *             plus a main one
 *             @li 10-29-2012 JRR Reorganized with global queue and shared
 *              data references
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
#ifndef _SHARES_H_
#define _SHARES_H_

//----------------------------------------------------------------------------
// Externs:  In this section, we declare variables and functions that are used in all
// (or at least two) of the files in the data acquisition project. Each of these items
// will also be declared exactly once, without the keyword 'extern', in one .cpp file
// as well as being declared extern here. 

// This queue allows tasks to send characters to the user interface task for display.
extern TextQueue* p_print_ser_queue;

/// This declares a taskshare which indicates that this variable is a shared
/// variable. This variable stores the setpoint for the PID motor controller.
extern TaskShare<int16_t>* motor_setpoint;

/// This declares a taskshare which indicates that this variable is a shared 
/// variable. This variable will hold the duty cycle of a motor. 
extern TaskShare<int16_t>* motor_power;
/// This declares a taskshare which indicates that this variable is a shared 
/// variable. This variable will hold the command that the user wishes the motor
/// to do.
///  0 Indicates SET POWER
///  1 Indicates BRAKE W/ brake_power
///  2 Indicates FREEWHEEL
extern TaskShare<uint8_t>* motor_directive;

/// This variable holds the encoder count and is positive or negative relative to the rotation of the counts accumulated either ClockWise(+), or CounterClockWise (-)
extern TaskShare<int32_t>* encoder_count;
/// This variable holds the ticks per seconds so that other tasks like task_user may access it.
extern TaskShare<int16_t>* encoder_ticks_per_task;
/// This holds the total number of errors detected by the ISR when setting the counts        
extern TaskShare<uint32_t>* data_read;

extern TaskShare<uint8_t>* activate_encoder;

extern TaskShare<int16_t>* steering_power;

extern TaskShare<uint16_t>* steering_angle;

extern TaskShare<int16_t>* x_joystick;
extern TaskShare<int16_t>* y_joystick;

#endif // _SHARES_H_
