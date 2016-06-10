//*****************************************************************************
/** @file main.cpp
 *  @brief     main.cpp declares all libraries, streams, tasks, and starts the 
 *             task scheduler.
 *
 *  @details   This main.cpp file is reponsible for the transmission of 
 *             information from the controller of the LegoCar Project running 
 *             on the Custom ME405 board with a ATmega1281 chipset. It S
 *
 *  @author(s) Anthony Lombardi Eddie Ruano
 *
 *  Revisions:
        @ 6/7/2016 <<EDD>> Optimized main and transmission protocol for speed

 *  License:
 *   Main transmitter declarations for LegoCar
 *   Copyright (C) 2016  Anthony Lombardi, Eddie Ruano
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
*/
//*****************************************************************************


#include <stdlib.h>             // Prototype declarations for I/O functions
#include <avr/io.h>             // Port I/O for SFR's
#include <avr/wdt.h>            // Watchdog timer header
#include <string.h>             // Functions for C string handling

#include "FreeRTOS.h"           // Primary header for FreeRTOS
#include "task.h"               // Header for FreeRTOS task functions
#include "queue.h"              // FreeRTOS inter-task communication queues
#include "croutine.h"           // Header for co-routines and such

#include "rs232int.h"           // ME405/507 library for serial comm.
#include "time_stamp.h"         // Class to implement a microsecond timer
#include "taskbase.h"           // Header of wrapper for FreeRTOS tasks
#include "textqueue.h"          // Wrapper for FreeRTOS character queues
#include "taskqueue.h"          // Header of wrapper for FreeRTOS queues
#include "taskshare.h"          // Header for thread-safe shared data
#include "shares.h"             // Global ('extern') queue declarations
#include "task_transmitter.h"
#include "controller_driver.h"

// Declare the queues which are used by tasks to communicate with each other here.
// Each queue must also be declared 'extern' in a header file which will be read
// by every task that needs to use that queue. The format for all queues except
// the serial text printing queue is 'frt_queue<type> name (size)', where 'type'
// is the type of data in the queue and 'size' is the number of items (not neces-
// sarily bytes) which the queue can hold

/** This is a print queue, descended from \c emstream so that things can be printed
 *  into the queue using the "<<" operator and they'll come out the other end as a
 *  stream of characters. It's used by tasks that send things to the user interface
 *  task to be printed.
 */
TextQueue* p_print_ser_queue;

/**
 * @brief      This main function sets up FreeRTOS, declares the only task 
 *             running on this transmitter board nicknamed "", and starts the 
 *             task scheduler which runs until board is powered off.
 * 
 * @details    The only task running is task_transmit and it within that task,
 *             the controller_driver is reading input from the joysicks. This 
 *             input is then packaged and transmitted via the bluetooth module 
 *             connected to the RX(0) and TX(0) lines on PortE.
 */

int main (void)
{
    // Disable the watchdog timer unless it's needed later. This is important because
    // sometimes the watchdog timer may have been left on...and it tends to stay on
    MCUSR = 0;
    wdt_disable ();

    // Set up a serial port for debugging at 9600 baud at the RXTX[1] ports
    rs232* p_ser_port = new rs232 (9600, 1);
    *p_ser_port << clrscr << PMS ("| ME405 LegoCar Transmitter |") << endl;

    //set up USART0 on E0 and E1 for external comms
    // rs232* p_ser_bt = new rs232(0, 0);
    // UCSR0A |= (1 << U2X0); // set the double-speed bit
    // UBRR0H = 0;
    // UBRR0L = 16; // set baud rate to 115200
    // *p_ser_bt << clrscr << PMS ("BT transmission check") << endl;

    // Create the queues and other shared data items here
    p_print_ser_queue = new TextQueue (32, "Print", p_ser_port, 10);

    // p_send_bt_queue = new TextQueue(32, "Send", p_ser_bt, 15);


    // Declare instance of driver to read input from controller joysticks etc.
    controller_driver* p_controller_driver = new controller_driver(p_ser_port);

    *p_ser_port << "Hello" << endl;


    // Start the transmission task and send it the driver for the controller.
    new task_transmitter ("trans", task_priority(5), 280, p_ser_port, p_controller_driver);

    // Start the task scheduler which runs forever
    vTaskStartScheduler ();
}

