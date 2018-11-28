#include "uart.h"

#define TXFE 0x80
#define RXFF 0x40
#define TXFF 0x20
#define RXFE 0x10
#define BUSY 0x08

static inline void mmio_write(uint32_t reg, uint32_t data) {
	*(volatile uint32_t *)reg = data;
}
 
static inline uint32_t mmio_read(uint32_t reg) {
	return *(volatile uint32_t *)reg;
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

void uartEnableInterrupt(void) {

}

void uartPutC(unsigned char byte) {
	// Wait for UART to become ready to transmit.
	while (mmio_read(UART0_FR) & TXFF);
	mmio_write(UART0_DR, byte);
}

void uartPutS(const char *s) {
    while (*s) {
        uartPutC(*s++);
    }
}

void uartPutI(const uint32_t i){
    if(i/10 > 0)
        uartPutI(i/10);
    uartPutC('0' + (i%10));	  
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

char uartGetC(void) {
	while (mmio_read(UART0_FR) & RXFE);
	return mmio_read(UART0_DR);
}

void uartGetS(char *s) {
	while ((*s = uartGetC()) != '\r') {
		uartPutC(*s);
		s++;
	}

	*s = '\0';
}

int uartGetI(void) {
	char c;
	int val = 0;

	while ((c = uartGetC()) != '\r') {
		uartPutC(c);
		val *= 10;
		val += (c - '0');
	}

	return val;
}

void clearLine(int len) {
	int i;
	uartPutC('\r');

	for (i = 0; i < len; i++) {
		uartPutC(' ');
	}
}

