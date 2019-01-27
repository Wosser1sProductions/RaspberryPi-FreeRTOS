#include "uart.h"
#include "bcm2835_intc.h"
#include "irq.h"

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>

#define UART_BUFFER_SIZE	128

#define TXFE 0x80
#define RXFF 0x40
#define TXFF 0x20
#define RXFE 0x10
#define BUSY 0x08

static volatile xSemaphoreHandle UART_Mutex = NULL;

static volatile xQueueHandle UART_RecvQueue = NULL;
static volatile xQueueHandle UART_SendQueue = NULL;

static inline void mmio_write(uint32_t reg, uint32_t data) {
	*(volatile uint32_t *)reg = data;
}
 
static inline uint32_t mmio_read(uint32_t reg) {
	return *(volatile uint32_t *)reg;
}

void uart_lock() {
	if (UART_Mutex != NULL) {
		xSemaphoreTake(UART_Mutex, portMAX_DELAY);
	}
}

void uart_unlock() {
	if (UART_Mutex != NULL) {
		xSemaphoreGive(UART_Mutex);
	}
}

enum {
    // The GPIO registers base address.
    GPIO_BASE = 0x20200000,
 
    // The offsets for reach register.
 
    // Controls actuation of pull up/down to ALL GPIO pins.
    GPPUD = (GPIO_BASE + 0x94),
 
    // Controls actuation of pull up/down for specific GPIO pin.
    GPPUDCLK0 = (GPIO_BASE + 0x98),
 
    // The base address for UART.
    UART0_BASE = 0x20201000,
 
    // The offsets for reach register for the UART.
    UART0_DR     = (UART0_BASE + 0x00),
    UART0_RSRECR = (UART0_BASE + 0x04),
    UART0_FR     = (UART0_BASE + 0x18),
    UART0_ILPR   = (UART0_BASE + 0x20),
    UART0_IBRD   = (UART0_BASE + 0x24),
    UART0_FBRD   = (UART0_BASE + 0x28),
    UART0_LCRH   = (UART0_BASE + 0x2C),
    UART0_CR     = (UART0_BASE + 0x30),
    UART0_IFLS   = (UART0_BASE + 0x34),
    UART0_IMSC   = (UART0_BASE + 0x38),
    UART0_RIS    = (UART0_BASE + 0x3C),
    UART0_MIS    = (UART0_BASE + 0x40),
    UART0_ICR    = (UART0_BASE + 0x44),
    UART0_DMACR  = (UART0_BASE + 0x48),
    UART0_ITCR   = (UART0_BASE + 0x80),
    UART0_ITIP   = (UART0_BASE + 0x84),
    UART0_ITOP   = (UART0_BASE + 0x88),
    UART0_TDR    = (UART0_BASE + 0x8C),
};

int uartInit() {
	// Seems not needed?
//    SetGpioFunction(UARTS_TX_PIN, 4);
//    SetGpioFunction(UARTS_RX_PIN, 4);

//	UART_Mutex = xSemaphoreCreateMutex();

	return UART_Mutex != NULL;
}

int uartEnableInterrupt() {
	UART_RecvQueue = xQueueCreate(UART_BUFFER_SIZE, sizeof(char));
	UART_SendQueue = xQueueCreate(UART_BUFFER_SIZE, sizeof(char));

	irqRegister(BCM2835_IRQ_ID_UART, &uart_interrupt_handler, NULL);

	return UART_RecvQueue != NULL && UART_SendQueue != NULL;
}

void uart_interrupt_handler(unsigned int irq, void *pParam) {
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

	if (irq == BCM2835_IRQ_ID_UART) {
		const uint32_t irq_read = mmio_read(UART0_MIS);
		if (irq_read & RXFE) {
			const char irq_in = mmio_read(UART0_DR);

			while(xQueueSendToBackFromISR(UART_RecvQueue, &irq_in, &xHigherPriorityTaskWoken) == 0)
				taskYIELD();
		} else if (irq_read & TXFF) {
			char irq_out;

			if (xQueueReceiveFromISR(UART_SendQueue, &irq_out, &xHigherPriorityTaskWoken) == 0) {
				// Disable interrupt?
				mmio_write(UART0_IMSC, mmio_read(UART0_IMSC) & ~RXFE);
			} else {
				mmio_write(UART0_DR, irq_out);
			}
		}

		if(xHigherPriorityTaskWoken) {
			taskYIELD();
		}
	}
}

void uartPutC(char byte) {
	if (UART_SendQueue != NULL) {
		while (xQueueSendToBack(UART_SendQueue, &byte, (portTickType) 10) == 0);

		// Set interrupt?
		mmio_write(UART0_IMSC, mmio_read(UART0_IMSC) | TXFF | RXFE);
	} else {
		// Wait for UART to become ready to transmit.
		while (mmio_read(UART0_FR) & TXFF);
		mmio_write(UART0_DR, byte);
	}
}

void uartPutS(const char *s) {
    while (*s) {
        uartPutC(*s++);
    }
}

void uartPutS_safe(const char *s) {
	uart_lock();
    while (*s) {
        uartPutC(*s++);
    }
    uart_unlock();
}

void uartPutI(const uint32_t i) {
    if(i/10 > 0)
        uartPutI(i/10);
    uartPutC('0' + (i%10));	  
}

void uartPutI_safe(const uint32_t i) {
	uart_lock();
	uartPutI(i);
	uart_unlock();
}

void uartPutF(float f) {
	if (f < 0.0f) {
		f = -f;
		uartPutC('-');
	}

	const uint32_t integer_part = f;

	uartPutI(integer_part);
	uartPutC('.');
	uartPutI((uint32_t)((f - integer_part) * 100.0));
}

void uartPutF_safe(float f) {
	uart_lock();
	uartPutF(f);
	uart_unlock();
}

char uartGetC(void) {
	char c;

	if (UART_RecvQueue != NULL) {
		// Get chars from interrupt queue
		while (xQueueReceive(UART_RecvQueue, &c, (portTickType) 10) == 0);
	} else {
		while (mmio_read(UART0_FR) & RXFE);
		c = mmio_read(UART0_DR);
	}

	return c;
}

void uartGetS_safe(char *s) {
	uart_lock();
	while ((*s = uartGetC()) != '\r') {
		uartPutC(*s);
		s++;
	}
	uart_unlock();

	*s = '\0';
}

int uartGetI_safe(void) {
	char c;
	int val = 0;

	uart_lock();
	while ((c = uartGetC()) != '\r') {
		uartPutC(c);
		val *= 10;
		val += (c - '0');
	}
	uart_unlock();

	return val;
}

void clearLine_safe(int len) {
	int i;

	uart_lock();
	uartPutC('\r');

	for (i = 0; i < len; i++) {
		uartPutC(' ');
	}
	uart_unlock();
}

