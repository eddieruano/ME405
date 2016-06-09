//*****************************************************************************
/** @file bno055_driver.cpp
 *  @brief     This is the header file for the BNO055 driver.
 *
 *  @details   This contains all prototypes of functions used in the BNO055
 *
 *  @author Eddie Ruano
 *
 *  Revisions: @ 5/28/2016 <<EDD>> fixed everything and made everything more
 *             robust and fixed a much of calls. Made it so that it's easier
 *             to use function.
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

// Include standard library header files
#include <stdlib.h>
#include <avr/io.h>
// Include header for serial port class
#include "rs232int.h"
// Include header for the driver class
#include "imu_driver.h"
#include <util/twi.h>
#include <math.h>
#include <limits.h>




#define STOP 0
#define CONTINUE 1
/**
 * @brief      This is the constuctor for the imu_driver class.
 * @details    This constructor takes in several addresses and delivers them
 *             to the protected variables declared in our header file.
 *
 * @param      serial_PORT_incoming  This holds the address to the incoming data
 *           register
 * @param      input_PORT_incoming   This holds the address to the incoming
 *                                   data register
 * @param      diag_PORT_incoming    This holds address port to incoming diag
 *                                   port reg
 * @param      pwm_PORT_incoming     This holds the address to the incoming
 *                                   data register
 * @param      ocr_PORT_incoming     This holds the address to the incoming
 *                                   ocr register
 * @param[in]  input_APIN_incoming   This holds defined value for INA pin
 * @param[in]  input_BPIN_incoming   This holds defined value for INB pin
 * @param[in]  diag_PIN_incoming     This holds defined value for appropriate
 *                                   diag ping
 * @param[in]  pwm_PIN_incoming      This holds defined value for pwm pin
 */

class bno055_driver
{
private:
    //no private variables needed.
protected:
    /// The imu_driver class uses this pointer to the serial port to say hello
    emstream* serial_PORT;
    /// pointer to input DDR register
    volatile uint8_t* input_DDR;
    /// pointer to the data input register
    volatile uint8_t* input_PORT;
    /// SCL
    uint8_t input_SCL;
    /// SDA
    uint8_t input_SDA;
    /// Counter for Comms
    uint8_t count;
    /// Data holder
    uint8_t *data;
    /// Slave address for BNO055 is 0x28
    uint8_t slave_address;
    uint8_t slave_address_write;
    uint8_t slave_address_read;

    ///set the public constructor and the public methods
public:
    bno055_driver::bno055_driver (
        emstream*,
        volatile uint8_t*,
        volatile uint8_t*,
        uint8_t,
        uint8_t
    );



}
