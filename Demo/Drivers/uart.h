#ifndef UART_H
#define UART_H

#include <stdint.h>

void uartPutC(unsigned char byte);
void uartPutS(const char *s);
void uartPutI(const uint32_t i);

#endif // UART_H