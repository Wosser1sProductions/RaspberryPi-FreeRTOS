/*
 * i2c.c
 *
 *  Created on: 21 nov. 2018
 *      Author: William
 */
#include "bcm2835_intc.h"
#include "i2c.h"

int I2C_interrupt_handler(unsigned int irq, void *pParam) {
	I2C_t* i2c = (I2C_t*) pParam;

	if (irq == BCM2835_IRQ_ID_I2C) {

		return 0;
	}

	return 1;
}

int I2C_master_transmit(I2C_t* i2c, uint8_t addr, uint8_t *pData, uint16_t size) {
	// Free?
	// ...

	// Send addr

	for (uint16_t i = 0; i < size; i++) {
		//send_reg = pData[i]
	}

	// Close

	return 1;
}

int I2C_master_receive (I2C_t* i2c, uint8_t addr, uint8_t *pData, uint16_t size) {
	// Data available?
	// ...

	// Send addr

	for (uint16_t i = 0; i < size; i++) {
		//pData[i] = recv_reg
	}

	// Close

	return 1;
}
