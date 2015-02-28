#include "BMP180.h"

BMP180::BMP180(PinName sda, PinName scl) : bmp180i2c(sda,scl)
{
    bmp180i2c.frequency(BMP180FREQ);
    oversampling_setting = OVERSAMPLING_HIGH_RESOLUTION;
    rReg[0] = 0;
    rReg[1] = 0;
    rReg[2] = 0;
    wReg[0] = 0;
    wReg[1] = 0;
    w[0] = 0xF4;
    w[1] = 0xF4;

    cmd = CMD_READ_CALIBRATION; // EEPROM calibration command
    for (int i = 0; i < EEprom; i++) { // read the 22 registers of the EEPROM
        bmp180i2c.write(BMP180ADDR, &cmd, 1);
        bmp180i2c.read(BMP180ADDR, rReg, 1);
        data[i] = rReg[0];
        cmd += 1;
        wait_ms(10);
    }

    // parameters AC1-AC6
    //The calibration is partioned in 11 words of 16 bits, each of them representing a coefficient
    ac1 =  (data[0] <<8) | data[1]; // AC1(0xAA, 0xAB)... and so on
    ac2 =  (data[2] <<8) | data[3];
    ac3 =  (data[4] <<8) | data[5];
    ac4 =  (data[6] <<8) | data[7];
    ac5 =  (data[8] <<8) | data[9];
    ac6 = (data[10] <<8) | data[11];
    // parameters B1,B2
    b1 =  (data[12] <<8) | data[13];
    b2 =  (data[14] <<8) | data[15];
    // parameters MB,MC,MD
    mb =  (data[16] <<8) | data[17];
    mc =  (data[18] <<8) | data[19];
    md =  (data[20] <<8) | data[21];
}

BMP180::~BMP180()
{
}

int BMP180::startTemperature()              // Start temperature measurement
{
    int errors = 0;
    errors += bmp180i2c.write(BMP180ADDR, w, 2);
    wReg[0] = 0xF4;
    wReg[1] = 0x2E;
    errors += bmp180i2c.write(BMP180ADDR, wReg, 2);   // write 0x2E in reg 0XF4
    return(errors);
}

int BMP180::readTemperature(long *t)        // Get the temperature reading that was taken in startTemperature() but ensure 4.5 ms time has elapsed
{
    int errors = 0;
    rReg[0] = 0;
    rReg[1] = 0;
    rReg[2] = 0;
    cmd = CMD_READ_VALUE;                           // 0xF6
    errors += bmp180i2c.write(BMP180ADDR, &cmd, 1); // set pointer on 0xF6 before reading it?
    errors += bmp180i2c.read(BMP180ADDR, rReg, 2);  // read 0xF6 (MSB) and 0xF7 (LSB)// rReg is 3 long though
    *t = (rReg[0] << 8) | rReg[1];                  // UT = MSB << 8 + LSB
    
    x1 = (((long) *t - (long) ac6) * (long) ac5) >> 15; // aka (ut-ac6) * ac5/pow(2,15)
    x2 = ((long) mc << 11) / (x1 + md);                 // aka mc * pow(2, 11) / (x1 + md)
    b5 = x1 + x2;
    *t = ((b5 + 8) >> 4);                               // (b5+8)/pow(2, 4)
    return(errors);
}

int BMP180::startPressure(int oversample)  // Start pressure measurement!  Note oversample will vary the time to complete this measurement. See defines above for oversampling constants to use!
{
    int errors = 0;
    oversampling_setting = BMP180::oversampleCheck(oversample);
    int uncomp_pressure_cmd = 0x34 + (oversampling_setting<<6);
    errors = bmp180i2c.write(BMP180ADDR, w, 2);
    wReg[0] = 0xF4;
    wReg[1] = uncomp_pressure_cmd;
    errors += bmp180i2c.write(BMP180ADDR, wReg, 2);
    return(errors);
}

int BMP180::readPressure(long *p)   // Get the pressure reading that was taken in startPressure() but ensure time for the measurement to complete
{
    int errors = 0;
    rReg[0] = 0;
    rReg[1] = 0;
    rReg[2] = 0;
    cmd = CMD_READ_VALUE;                           // 0xF6
    errors += bmp180i2c.write(BMP180ADDR, &cmd, 1);
    errors += bmp180i2c.read(BMP180ADDR, rReg, 3);  // read 0xF6 (MSB), 0xF7 (LSB), 0xF8 (XLSB)
    *p = ((rReg[0] << 16) | (rReg[1] << 8) | rReg[2]) >> (8 - oversampling_setting);
    
    b6 = b5 - 4000;             // realize b5 is set in readTemperature() function so that needs to be done first before this function! 
    x1 = (b6*b6) >> 12;         // full formula(b2*(b6*b6)/pow(2,12))/pow(2,11)
    x1 *= b2;
    x1 >>= 11;

    x2 = (ac2*b6);
    x2 >>= 11;

    x3 = x1 + x2;

    b3 = (((((long)ac1 )*4 + x3) <<oversampling_setting) + 2) >> 2; 

    x1 = (ac3* b6) >> 13;
    x2 = (b1 * ((b6*b6) >> 12) ) >> 16;
    x3 = ((x1 + x2) + 2) >> 2;
    b4 = (ac4 * (unsigned long) (x3 + 32768)) >> 15;

    b7 = ((unsigned long) *p - b3) * (50000>>oversampling_setting);
    if (b7 < 0x80000000) {
        *p = (b7 << 1) / b4;
    } else {
        *p = (b7 / b4) << 1;
    }

    x1 = *p >> 8;
    x1 *= x1;                               // pressure/pow(2,8) * pressure/pow(2, 8)
    x1 = (x1 * 3038) >> 16;
    x2 = ( *p * -7357) >> 16;
    *p += (x1 + x2 + 3791) >> 4;            // pressure in Pa
    return(errors);
}

int BMP180::readTP(long *t, long *p, int oversample)  // get both temperature and pressure calculations that are compensated
{
    int errors = 0;
    errors += BMP180::startTemperature();
    wait_ms(4.5);
    errors += BMP180::readTemperature(t);
    errors += BMP180::startPressure(oversample);
    switch (oversample) {
        case OVERSAMPLING_ULTRA_LOW_POWER:
            wait_ms(4.5);
            break;
        case OVERSAMPLING_STANDARD:
            wait_ms(7.5);
            break;
        case OVERSAMPLING_HIGH_RESOLUTION:
            wait_ms(13.5);
            break;
        case OVERSAMPLING_ULTRA_HIGH_RESOLUTION:
            wait_ms(25.5);
            break;
    }
    errors += BMP180::readPressure(p);
    return(errors);
}

int BMP180::oversampleCheck(int oversample)
{
    switch(oversample) {
        case OVERSAMPLING_ULTRA_LOW_POWER:
            break;
        case OVERSAMPLING_STANDARD:
            break;
        case OVERSAMPLING_HIGH_RESOLUTION:
            break;
        case OVERSAMPLING_ULTRA_HIGH_RESOLUTION:
            break;
        default:
            oversample = OVERSAMPLING_ULTRA_HIGH_RESOLUTION;
            break;
    }
    return(oversample);
}