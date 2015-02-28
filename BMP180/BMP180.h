#ifndef BMP180_H
#define BMP180_H

/* BMP180 Digital Pressure Sensor Class for use with Mbed LPC1768 and other platforms
*  BMP180 from Bosch Sensortec
*  Copyright (c) 2013 Philip King Smith
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
* Bosch data sheet at: http://ae-bst.resource.bosch.com/media/products/dokumente/bmp180/BST-BMP180-DS000-09.pdf
* Some parts of the calculations used are infact from Rom Clement published Mbed code here: https://mbed.org/users/Rom/code/Barometer_bmp085/
*   I only used snippets of the Rom's code because i was making this into a class and so this is structured totaly different then his code example is.
* I also used the Bosch data sheet showing the calculations and adjusted everything accordingly!
*/

#include "mbed.h"

#define EEprom 22            // The EEPROM has 176bits of calibration data (176/8 = 22 Bytes)
#define BMP180ADDR 0xEF      // I2C address of BMP180 device
#define BMP180FREQ 1000000   // Data sheet says 3.4 MHz is max but not sure what mbed can do here!
#define CMD_READ_VALUE 0xF6
#define CMD_READ_CALIBRATION 0xAA
#define OVERSAMPLING_ULTRA_LOW_POWER 0  // these are the constants used in the oversample variable in the below code!
#define OVERSAMPLING_STANDARD 1
#define OVERSAMPLING_HIGH_RESOLUTION 2
#define OVERSAMPLING_ULTRA_HIGH_RESOLUTION 3


/** BMP180 Digital Pressure Sensor class using mbed's i2c class
 *
 * Example:
 * @code
 * // show how the BMP180 class works
 * #include "mbed.h"
 * #include "BMP180.h"
 *
 * int main()
 * {
 *
 *    long temp ;
 *    long pressure;
 *    int error ;
 *    BMP180 mybmp180(p9,p10);
 *    while(1) {
 *        error = mybmp180.readTP(&temp,&pressure,OVERSAMPLING_ULTRA_HIGH_RESOLUTION);
 *        printf("Temp is %ld\r\n",temp);
 *        printf("Pressure is %ld\r\n",pressure);
 *        printf("Error is %d\r\n\r\n",error);
 *        wait(2);
 *    }
 * }
 * @endcode
 */

class BMP180
{
public:
    /** Create object connected to BMP180 pins ( remember both pins need pull up resisters)
        *
        * Ensure the pull up resistors are used on these pins.  Also note there is no checking on
        *  if you use these pins p9, p10, p27, p28 so ensure you only use these ones on the LPC1768 device
        *
        * @param sda pin that BMP180 connected to (p9 or p28 as defined on LPC1768)
        * @param slc pin that BMP180 connected to (p10 or p27 ad defined on LPC1768)
        */
    BMP180(PinName sda, PinName slc);   // Constructor

    ~BMP180();                          // Destructor

    /** Read Temperature and Pressure at the same time
    *
    * This function will only return when it has readings.  This means that the time will vary depending on oversample setting!
    *   Note if your code can not wait for these readings to be taken and calculated you should use the functions below.
    *     These other functions are designed to allow your code to do other things then get the final readings.
    *     This function is only designed as a one shot give me the answer function.
    *
    * @param t the temperature fully compensated value is returned in this variable. Degrees celsius with one decimal so 253 is 25.3 C.
    * @param p the barometric pressure fully compensated value is returned in this variable. Pressure is in Pa so 88007 is 88.007 kPa.
    * @param oversample is the method method for reading sensor.  OVERSAMPLING_ULTRA_HIGH_RESOLUTION is used if an incorrect value is passed to this function.
    * @param returns 0 for no errors during i2c communication.  Any other number is just a i2c communication failure of some kind!
    */
    int readTP(long *t, long *p, int oversample);    // get both temperature and pressure fully compensated values! Note this only returns when measurements are complete

    /** Start the temperature reading process but return after the commands are issued to BMP180
    *
    * This function is ment to start the temperature reading process but will return to allow other code to run then a reading could be made at a later time.
    *   Note the maximum time needed for this measurment is 4.5 ms.
    *
    * @param returns 0 for no errors during i2c communication.  Any other number is just a i2c communication failure of some kind!
    */
    int startTemperature();             // Start temperature measurement

    /** Reads the last temperature reading that was started with startTemperature() function
    *
    * This function will return the fully compensated value of the temperature in Degrees celsius with one decimal so 253 is 25.3 C.
    *   Note this function should normaly follow the startTemperature() function and should also preceed the startPressure() and readPressure() commands!
    *   Note this function should follow startTemperature() after 4.5 ms minimum has elapsed or reading will be incorrect.
    *
    * @param t the temperature fully compensated value is returned in this variable.
    * @param returns 0 for no errors during i2c communication.  Any other number is just a i2c communication failure of some kind!
    */
    int readTemperature(long *t);       // Get the temperature reading that was taken in startTemperature() but ensure 4.5 ms time has elapsed

    /** Start the pressure reading process but return after the commands are issued to BMP180
    *
    * This function is ment to start the pressure reading process but will return to allow other code to run then a reading could be made at a later time.
    *   Note the time needed for this reading pressure process will depend on oversample setting.  The maximum time is 25.5 ms and minimum time is 4.5 ms.
    *
    * @param oversample is the method for reading sensor.  OVERSAMPLING_ULTRA_HIGH_RESOLUTION is used if an incorrect value is passed to this function.
    * @param returns 0 for no errors during i2c communication.  Any other number is just a i2c communication failure of some kind!
    */
    int startPressure(int oversample);  // Start pressure measurement!  Note oversample will vary the time to complete this measurement. See defines above for oversampling constants to use!

    /** Reads the last barometric pressure reading that was started with startPressure() function
    *
    * This function will return the fully compensated value of the barometric pressure in Pa.
    *   Note this function should follow startPressure() after the time needed to read the pressure.  This time will vary but maximum time is 25.5 ms and minimum time is 4.5 ms.
    *   Note that this reading is dependent on temperature so the startTemperature() and readTemperature() functions should proceed this function or the pressure value will be incorrect!
    *
    * @param p the barometric pressure fully compensated value is returned in this variable. Pressure is in Pa so 88007 is 88.007 kPa.
    * @param returns 0 for no errors during i2c communication.  Any other number is just a i2c communication failure of some kind!
    */
    int readPressure(long *p);          // Get the pressure reading that was taken in startPressure() but ensure time for the measurement to complete

protected:
    long x1;
    long x2;
    long x3;
    short ac1;
    short ac2;
    short ac3;
    unsigned short ac4;
    unsigned short ac5;
    unsigned short ac6;
    short b1;
    short b2;
    long b3;
    unsigned long b4;
    long b5;
    long b6;
    unsigned long b7;
    short mb;
    short mc;
    short md;
    int oversampling_setting;
    char rReg[3];
    char wReg[2];
    char cmd;
    char data[EEprom];
    char w[2];

    I2C bmp180i2c;              // the I2C class for this bmp180 communication.
    int oversampleCheck(int oversample);
};

#endif