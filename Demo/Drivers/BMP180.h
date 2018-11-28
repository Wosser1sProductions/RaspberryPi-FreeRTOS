/*
 * BMP180.h
 *
 *  Created on: 21 nov. 2018
 *      Author: William
 */

#ifndef DEMO_DRIVERS_BMP180_H_
#define DEMO_DRIVERS_BMP180_H_

#include <stdbool.h>
#include "i2c.h"

// Oversampling settings
typedef enum {
    ULTRA_LOW_POWER       =  5, ///< 1 pressure sample  :  4.5 ms delay
    STANDARD              =  8, ///< 2 pressure samples :  7.5 ms delay
    HIGH_RESOLUTION       = 14, ///< 4 pressure samples : 13.5 ms delay
    ULTRA_HIGH_RESOLUTION = 26  ///< 8 pressure samples : 25.5 ms delay
} OverSamplingSetting_t;

#define OSS_Setting STANDARD

/**
 *  Struct to define a BMP180 pressure sensor.
 */
typedef struct {
	I2C_t *i2c;

	OverSamplingSetting_t m_oss;
    float m_altitude;

    uint8_t m_data[2];

    uint16_t ac4, ac5, ac6;
    int16_t  ac1, ac2, ac3, b1, b2, b5, md;
    int64_t  x1, x2, x3, b3, b6, mc;
    uint64_t b4, b7;
} BMP180_t;

BMP180_t* BMP180_create(void);
void BMP180_destroy(BMP180_t **self);

bool BMP180_initialise(BMP180_t *self, I2C_t *i2c, float altitude, OverSamplingSetting_t oss);

bool BMP180_readData(BMP180_t *self, float *pTemperature, float *pPressure);


#endif /* DEMO_DRIVERS_BMP180_H_ */
