/*
 * i2c.c
 *
 *  Created on: 21 nov. 2018
 *      Author: William
 */
#include "bcm2835_intc.h"
#include "../utils.h"
#include "i2c.h"
#include "gpio.h"
#include "uart.h"

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>

static volatile xSemaphoreHandle I2C_Mutex = NULL;

static inline void mmio_write(uint32_t reg, uint32_t data) {
	*(volatile uint32_t *)reg = data;
}

static inline void mmio_write_or(uint32_t reg, uint32_t data) {
	*(volatile uint32_t *)reg |= data;
}

static inline void mmio_write_not(uint32_t reg, uint32_t data) {
	*(volatile uint32_t *)reg &= ~data;
}

static inline uint32_t mmio_read(uint32_t reg) {
	return *(volatile uint32_t *)reg;
}

static inline uint32_t mmio_read_bit(uint32_t reg, uint32_t bit) {
	return mmio_read(reg) & bit;
}

void i2c_lock() {
	if (I2C_Mutex != NULL) {
		xSemaphoreTake(I2C_Mutex, portMAX_DELAY);
	}
}

void i2c_unlock() {
	if (I2C_Mutex != NULL) {
		xSemaphoreGive(I2C_Mutex);
	}
}

enum {
    // The base address for I2C. 0x20205000 or 0x20804000?
	I2C_BASE  = 0x20205000,

    // The offsets for reach register for I2C.
	I2C_CONTROL             = (I2C_BASE + 0x00),
	I2C_STATUS              = (I2C_BASE + 0x01),
	I2C_DLEN                = (I2C_BASE + 0x02),
	I2C_SLAVE_ADDR          = (I2C_BASE + 0x03),
	I2C_FIFO                = (I2C_BASE + 0x04),
	I2C_CLK_DIV             = (I2C_BASE + 0x05),
	I2C_DATA_DELAY          = (I2C_BASE + 0x06),
	I2C_CLK_STRETCH_TIMEOUT = (I2C_BASE + 0x07),

	// I2C Setting bits
	I2C_CTRL_I2CEN  = (1 << 15),
	I2C_CTRL_INTR   = (1 << 10),
	I2C_CTRL_INTT   = (1 << 9) ,
	I2C_CTRL_INTD   = (1 << 8) ,
	I2C_CTRL_ST     = (1 << 7) ,
	I2C_CTRL_CLEAR  = (1 << 4) ,
	I2C_CTRL_READ   = 1,

	START_READ      = I2C_CTRL_I2CEN|I2C_CTRL_ST|I2C_CTRL_CLEAR|I2C_CTRL_READ,
	START_WRITE     = I2C_CTRL_I2CEN|I2C_CTRL_ST,

	I2C_STATUS_CLKT = (1 << 9),
	I2C_STATUS_ERR  = (1 << 8),
	I2C_STATUS_RXF  = (1 << 7),
	I2C_STATUS_TXE  = (1 << 6),
	I2C_STATUS_RXD  = (1 << 5),
	I2C_STATUS_TXD  = (1 << 4),
	I2C_STATUS_RXR  = (1 << 3),
	I2C_STATUS_TXW  = (1 << 2),
	I2C_STATUS_DONE = (1 << 1),
	I2C_STATUS_TA   = 1,

	CLEAR_STATUS    =  I2C_STATUS_CLKT|I2C_STATUS_ERR|I2C_STATUS_DONE,
};

int i2cInit() {
    SetGpioFunction(I2C_SDA_PIN, 4);
    SetGpioFunction(I2C_SCL_PIN, 4);

    PutGpio(I2C_SDA_PIN, PULL_UP);
    PutGpio(I2C_SCL_PIN, PULL_UP);

    I2C_Mutex = xSemaphoreCreateMutex();

	return I2C_Mutex != NULL;
}

void I2C_Status() {
	uart_lock();
	uartPutS("I2C: ERR=");
	uartPutI(mmio_read_bit(I2C_STATUS, I2C_STATUS_ERR)  != 0);

	uartPutS("  RXF=");
	uartPutI(mmio_read_bit(I2C_STATUS, I2C_STATUS_RXF)  != 0);

	uartPutS("  TXE=");
	uartPutI(mmio_read_bit(I2C_STATUS, I2C_STATUS_TXE)  != 0);

	uartPutS("  RXD=");
	uartPutI(mmio_read_bit(I2C_STATUS, I2C_STATUS_RXD)  != 0);

	uartPutS("  RXR=");
	uartPutI(mmio_read_bit(I2C_STATUS, I2C_STATUS_RXR)  != 0);

	uartPutS("  TXW=");
	uartPutI(mmio_read_bit(I2C_STATUS, I2C_STATUS_TXW)  != 0);

	uartPutS("  DONE=");
	uartPutI(mmio_read_bit(I2C_STATUS, I2C_STATUS_DONE) != 0);

	uartPutS("  TA=");
	uartPutI(mmio_read_bit(I2C_STATUS, I2C_STATUS_TA)   != 0);

	uartPutS(NEWLINE);
	uart_unlock();
}

void I2C_interrupt_handler(unsigned int irq, void * pParam) {
	UNUSED(pParam);
	// I2C_t* i2c = (I2C_t*) pParam;

	if (irq == BCM2835_IRQ_ID_I2C) {
		i2c_lock();
		i2c_unlock();
	}
}

static int I2C_WaitUntilDone() {
	// Wait till done, use a timeout just in case
	int timeout = 50;

	while (!mmio_read_bit(I2C_STATUS, I2C_STATUS_DONE) && --timeout > 0) {
		//uartPutS_safe("I2C_WaitUntilDone: Not done yet..." NEWLINE);
		vTaskDelay(2);
	}

	if (timeout == 0) {
		uart_lock();
		uartCmd(CONSOLE_FG_RED);
		uartPutS("ERR I2C timeout?" NEWLINE);
		uartCmd(CONSOLE_RESET);
		uart_unlock();
		return 1;
	}

	//uartPutS_safe("I2C_WaitUntilDone: OK" NEWLINE);

	return 0;
}

int I2C_master_transmit(I2C_t* i2c, uint8_t addr, uint8_t *pData, uint16_t size) {
	UNUSED(i2c);

	i2c_lock();
	mmio_write(I2C_STATUS, CLEAR_STATUS);
//	mmio_write(I2C_CONTROL, I2C_CTRL_CLEAR);
	mmio_write(I2C_SLAVE_ADDR, addr);
	mmio_write(I2C_DLEN, size);

	#if 0 // print
		uart_lock();
		uartPutS("Write: ");
		for (uint16_t i = 0; i < size; i++) {
			mmio_write(I2C_FIFO, pData[i]);
			uartPutI(pData[i]);
			uartPutC(',');
		}
		uartPutS(NEWLINE);
		uart_unlock();
	#else
		for (uint16_t i = 0; i < size; i++) {
			mmio_write(I2C_FIFO, pData[i]);
		}
	#endif

//	uartPutS_safe("I2C start write" NEWLINE);

	mmio_write(I2C_STATUS, CLEAR_STATUS);
//	mmio_write_not(I2C_CONTROL, I2C_CTRL_READ);  //?
	mmio_write(I2C_CONTROL, START_WRITE);

	const int timeout = I2C_WaitUntilDone();

	i2c_unlock();
	return timeout;
}

int I2C_master_receive(I2C_t* i2c, uint8_t addr, uint8_t *pData, uint16_t size) {
	UNUSED(i2c);

	i2c_lock();

	#if 1  // Sequential or smaller blocks?
		mmio_write(I2C_SLAVE_ADDR, addr);
		mmio_write(I2C_DLEN, size);

//		uartPutS_safe("I2C start read" NEWLINE);

		mmio_write(I2C_STATUS, CLEAR_STATUS);
	//	mmio_write(I2C_CONTROL, I2C_CTRL_CLEAR);  // ?
		mmio_write(I2C_CONTROL, START_READ);

		if (I2C_WaitUntilDone()) return 1;  // timeout

		#if 0 // print
			uart_lock();
			uartPutS("Read: ");
			for (uint16_t i = 0; i < size; i++) {
				pData[i] = mmio_read_bit(I2C_FIFO, 0xFF);
				uartPutI(pData[i]);
				uartPutC(',');
			}
			uartPutS(NEWLINE);
			uart_unlock();
		#else
			for (uint16_t i = 0; i < size; i++) {
				pData[i] = mmio_read_bit(I2C_FIFO, 0xFF);
			}
		#endif
	#else
		#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))

		mmio_write(I2C_SLAVE_ADDR, addr);

		while (size > 0) {
			// Read data per block of 8 bytes
			const int block_len = MIN(size, 8);

			mmio_write(I2C_STATUS, CLEAR_STATUS);
			mmio_write(I2C_CONTROL, I2C_CTRL_CLEAR);
			mmio_write(I2C_DLEN, block_len);
			mmio_write(I2C_CONTROL, START_READ);

			if (I2C_WaitUntilDone()) return 1;  // timeout

			for (uint16_t i = 0; i < block_len; i++, pData++) {
				*(pData + i) = mmio_read_bit(I2C_FIFO, 0xFF);
			}

			size -= block_len;
			vTaskDelay(1);
		}
	#endif

//    uartPutS_safe("I2C read done" NEWLINE);

	i2c_unlock();
	return 0;
}
