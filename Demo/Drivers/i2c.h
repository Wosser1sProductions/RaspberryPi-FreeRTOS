/*
 * i2c.h
 *
 *  Created on: 21 nov. 2018
 *      Author: William
 */

#ifndef DEMO_DRIVERS_I2C_H_
#define DEMO_DRIVERS_I2C_H_

/*
 *  Use this maybe?
 *      http://www.pieter-jan.com/node/15 at https://github.com/Pieter-Jan/PJ_RPI
 *
 *
 *
 */

#include <stdint.h>

typedef struct {
	int SDA;
	int SCL;
} I2C_t;

int I2C_interrupt_handler(unsigned int irq, void *pParam);
int I2C_master_transmit(I2C_t* i2c, uint8_t addr, uint8_t *pData, uint16_t size);
int I2C_master_receive (I2C_t* i2c, uint8_t addr, uint8_t *pData, uint16_t size);

#endif /* DEMO_DRIVERS_I2C_H_ */
