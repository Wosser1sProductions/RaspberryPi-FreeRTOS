/*
 * BMP180.c
 *
 *  Created on: 21 nov. 2018
 *      Author: William
 */

#include "BMP180.h"
#include <stdlib.h>
#include <math.h>
#include "uart.h"

#include <FreeRTOS.h>
#include <task.h>
#include <portable.h>
extern void vTaskDelay(unsigned long);
extern void* pvPortMalloc(size_t);

//BMP180 Addresses
#define BMP180_ADDRESS        0xEE
#define BMP_WRITE_ADDRESS     0xEE
#define BMP_READ_ADDRESS      0xEF

#define CONTROL 0x002E    // Temperature RAW takes 4.5ms
//BMP Commands
#define TEMP    0xF42E    // Temperature RAW takes 4.5ms
#define PRESS   0x0034    // Pressure RAW takes 4.5ms
#define PRESS1  0x0074    // Pressure RAW takes 7.5ms oversampling1
#define PRESS2  0x00B4    // Pressure RAW takes 13.5ms oversampling2
#define PRESS3  0x00F4    // Pressure RAW takes 25.5ms oversampling3

//EEPROM of BMP, MSB first
#define SRESET    0xE0   // softreset
#define BMP_ID    0xD0   // returns 0x55
#define CTRL_MEAS 0xF4   // 4:0 measurement, bit5 start of conversion 1=converting,
//7:5 Control oversampling for pressure measurement

//short
#define AC1  0xAA    // MSB
#define AC12 0xAB    // LSB
#define AC2  0xAC    //
#define AC22 0xAD    //
#define AC3  0xAE    //
#define AC32 0xAF    //

//unsigned short
#define AC4  0xB0 //
#define AC42 0xB1 //
#define AC5  0xB2 //
#define AC52 0xB3 //
#define AC6  0xB4 //
#define AC62 0xB5 //

//short
#define B1  0xB6 //
#define B12 0xB7 //
#define B2  0xB8 //
#define B22 0xB9 //

//short
#define MB  0xBA //
#define MB2 0xBB //
#define MC  0xBC //
#define MC2 0xBD //
#define MD  0xBE //
#define MD2 0xBF //


/******************************************************/
static bool BMP180_ReadRawTemperature(BMP180_t *self, int16_t* pUt) {
    int error = 0;

    // request temperature measurement
    self->m_data[0] = 0xF4;
    self->m_data[1] = 0x2E;

    // write 0XF2 into reg 0XF4
    error += I2C_master_transmit(self->i2c, BMP180_ADDRESS, self->m_data, 2u);
    vTaskDelay(5);

    // read raw temperature data
    self->m_data[0] = 0xF6;

    // set eeprom pointer position to 0XF6
    error += I2C_master_transmit(self->i2c, BMP180_ADDRESS, self->m_data, 2u);

    // get 16 bits at this position
    error += I2C_master_receive(self->i2c, BMP180_ADDRESS, self->m_data, 2u);

    if (error) {
        return 1;
    }

    *pUt = self->m_data[0] << 8 | self->m_data[1];

    return 0;
}

static bool BMP180_ReadRawPressure(BMP180_t *self, int32_t* pUp) {
    int error = 0;

    // request pressure measurement
    self->m_data[0] = 0xF4;
    self->m_data[1] = 0x34 + (self->m_oss << 6);

    // write 0x34 + (m_oss << 6) into reg 0XF4
    error += I2C_master_transmit(self->i2c, BMP180_ADDRESS, self->m_data, 2u);

    // Rounded up wait times to be safe
    vTaskDelay(self->m_oss);

    // read raw pressure data
    self->m_data[0] = 0xF6;

    // set eeprom pointer position to 0XF6
    error += I2C_master_transmit(self->i2c, BMP180_ADDRESS, self->m_data, 1u);

    // get 16 bits at this position
    error += I2C_master_receive(self->i2c, BMP180_ADDRESS, self->m_data, 2u);

    if (error) {
        return 1;
    }

    *pUp = ((int32_t)self->m_data[0] << 16 | self->m_data[1] << 8) >> (8 - self->m_oss);

    return 0;
}

static float BMP180_TrueTemperature(BMP180_t *self, int16_t ut) {
	self->x1 = ((ut - self->ac6) * self->ac5) >> 15;
	self->b5 = self->x1 + (self->mc / (self->x1 + self->md));

    // convert to Celcius
    return (float)((self->b5 + 8) >> 4) / 10.F;
}

static float BMP180_TruePressure(BMP180_t *self, int32_t up) {
	int64_t p;

    // straight out from the documentation
    self->b6 = self->b5 - 4000;
    self->x1 = (self->b2 * (self->b6 * self->b6 >> 12)) >> 11;
    self->x2 = self->ac2 * self->b6 >> 11;
    self->b3 = (((self->ac1 + self->x1 + self->x2) << self->m_oss) + 2) >> 2;
    self->x1 = (self->ac3 * self->b6) >> 13;
    self->x2 = (self->b1 * ((self->b6 * self->b6) >> 12)) >> 16;
    self->x3 = (self->x1 + self->x2 + 2) >> 2;
    self->b4 = self->ac4 * (uint64_t)(self->x3 + 32768) >> 15;
    self->b7 = ((uint64_t)up - self->b3)* (50000 >> self->m_oss);

    if (self->b7 < 0x80000000) {
        p = (self->b7 << 1) / self->b4;
    } else {
        p = (self->b7 / self->b4) << 1;
    }

    self->x1 = (p >> 8) * (p >> 8);
    self->x1 = (self->x1 * 3038) >> 16;
    self->x2 = (-7357 * p) >> 16;

    p = p + ((self->x1 + self->x2 + 3791) >> 4);

    // convert to hPa and, if altitude has been initialized, to sea level pressure
    return (self->m_altitude == 0.F) ? (float)p / 100.F
                                     : (float)p / (100.F * pow((1.0 - self->m_altitude / 44330.0), 5.255));
}
/******************************************************/

inline BMP180_t* BMP180_create(void) {
    //return (BMP180_t*) calloc(1u, sizeof(BMP180_t));
	return (BMP180_t*)pvPortMalloc(sizeof(BMP180_t));
}

inline void BMP180_destroy(BMP180_t **self) {
    if (*self != NULL) {
    	//free(*self);
    	vPortFree(*self);
    	*self = NULL;
    }
}

bool BMP180_initialise(BMP180_t *self, I2C_t *i2c, float altitude, OverSamplingSetting_t oss) {
    uint8_t data[22];
    int error = 0;

    uartPutS("[CREATE] BMP180 : Initialising... ");

    self->i2c        = i2c;
    self->m_altitude = altitude;
    self->m_oss      = oss;

    // read calibration data
    data[0] = 0xAA;

    // set the eeprom pointer position to 0xAA
    error += I2C_master_transmit(self->i2c, BMP180_ADDRESS, data, 1u);
    // read 11 x 16 bits at this position
    error += I2C_master_receive(self->i2c, BMP180_ADDRESS, data, 22u);

    if (error == 0) {
        // store calibration data for further calculus
        self->ac1 = data[0]  << 8 | data[1];
        self->ac2 = data[2]  << 8 | data[3];
        self->ac3 = data[4]  << 8 | data[5];
        self->ac4 = data[6]  << 8 | data[7];
        self->ac5 = data[8]  << 8 | data[9];
        self->ac6 = data[10] << 8 | data[11];
        self->b1  = data[12] << 8 | data[13];
        self->b2  = data[14] << 8 | data[15];
        //self->mb  = data[16] << 8 | data[17];   // Not used?
        self->mc  = data[18] << 8 | data[19];
        self->md  = data[20] << 8 | data[21];

        // Pre-calc
        self->ac1 = self->ac1 << 2;
        self->mc  = (0xFFFF0000 | self->mc) << 11;

    	uartCmd(CONSOLE_FG_GREEN);
    	uartPutS("Ok" NEWLINE);
    } else {
    	uartCmd(CONSOLE_FG_RED);
    	uartPutS("Error" NEWLINE);
    }

    return error == 0;
}

bool BMP180_readData(BMP180_t *self, float *pTemperature, float *pPressure) {
	int16_t t;
	int32_t p;

    if (BMP180_ReadRawTemperature(self, &t) || BMP180_ReadRawPressure(self, &p)) {
        return 1;   // Error
    }

    *pTemperature = BMP180_TrueTemperature(self, t);
    *pPressure    = BMP180_TruePressure(self, p);

    return 0;
}
