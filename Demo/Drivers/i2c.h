/*
 * i2c.h
 *
 *  Created on: 21 nov. 2018
 *      Author: William
 */

#ifndef DEMO_DRIVERS_I2C_H_
#define DEMO_DRIVERS_I2C_H_

#include <stdint.h>

typedef struct {
	int x;
} I2C_t;

int I2C_master_transmit(I2C_t* i2c, uint8_t addr, uint8_t *pData, uint16_t size);
int I2C_master_receive (I2C_t* i2c, uint8_t addr, uint8_t *pData, uint16_t size);

#endif /* DEMO_DRIVERS_I2C_H_ */
