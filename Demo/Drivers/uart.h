#ifndef UART_H
#define UART_H

#include <stdint.h>

#define UARTS_TX_PIN  14
#define UARTS_RX_PIN  15

#define DEGREES_CELCIUS		"\u00B0C"   ///< Degrees Celcius
#define NEWLINE				"\r\n"

/**
 *  Terminal commands with colours for printing.
 *  Reference: http://www.lihaoyi.com/post/BuildyourownCommandLinewithANSIescapecodes.html
 */
typedef enum console_commands {
	CONSOLE_CLS = 0,           ///< Clear entire screen
	CONSOLE_CURSOR,            ///< Set cursor to start
	CONSOLE_RESET,             ///< Reset formatting to black bg with white fg

	CONSOLE_BOLD,              ///< Make text bold
	CONSOLE_UNDERLINE,         ///< Make text underlined

	CONSOLE_FG_BLACK,          ///< Set text FG colour
	CONSOLE_FG_BRIGHT_BLACK,   ///< Set text FG colour
	CONSOLE_FG_RED,            ///< Set text FG colour
	CONSOLE_FG_BRIGHT_RED,     ///< Set text FG colour
	CONSOLE_FG_GREEN,          ///< Set text FG colour
	CONSOLE_FG_BRIGHT_GREEN,   ///< Set text FG colour
	CONSOLE_FG_YELLOW,         ///< Set text FG colour
	CONSOLE_FG_BRIGHT_YELLOW,  ///< Set text FG colour
	CONSOLE_FG_BLUE,           ///< Set text FG colour
	CONSOLE_FG_BRIGHT_BLUE,    ///< Set text FG colour
	CONSOLE_FG_MAGENTA,        ///< Set text FG colour
	CONSOLE_FG_BRIGHT_MAGENTA, ///< Set text FG colour
	CONSOLE_FG_CYAN,           ///< Set text FG colour
	CONSOLE_FG_BRIGHT_CYAN,    ///< Set text FG colour
	CONSOLE_FG_WHITE,          ///< Set text FG colour
	CONSOLE_FG_BRIGHT_WHITE,   ///< Set text FG colour

	CONSOLE_BG_BLACK,          ///< Set text BG colour
	CONSOLE_BG_BRIGHT_BLACK,   ///< Set text BG colour
	CONSOLE_BG_RED,            ///< Set text BG colour
	CONSOLE_BG_BRIGHT_RED,     ///< Set text BG colour
	CONSOLE_BG_GREEN,          ///< Set text BG colour
	CONSOLE_BG_BRIGHT_GREEN,   ///< Set text BG colour
	CONSOLE_BG_YELLOW,         ///< Set text BG colour
	CONSOLE_BG_BRIGHT_YELLOW,  ///< Set text BG colour
	CONSOLE_BG_BLUE,           ///< Set text BG colour
	CONSOLE_BG_BRIGHT_BLUE,    ///< Set text BG colour
	CONSOLE_BG_MAGENTA,        ///< Set text BG colour
	CONSOLE_BG_BRIGHT_MAGENTA, ///< Set text BG colour
	CONSOLE_BG_CYAN,           ///< Set text BG colour
	CONSOLE_BG_BRIGHT_CYAN,    ///< Set text BG colour
	CONSOLE_BG_WHITE,          ///< Set text BG colour
	CONSOLE_BG_BRIGHT_WHITE,   ///< Set text BG colour
} ConsoleCommand_t;

extern const char* CONSOLE_LUT[];

#define uartCmd(C) uartPutS(CONSOLE_LUT[(C)])

int uartInit();
int uartEnableInterrupt();

void uart_interrupt_handler(unsigned int irq, void *pParam);

void uart_lock();
void uart_unlock();

void uartPutC(char byte);
void uartPutS(const char *s);
void uartPutS_safe(const char *s);
void uartPutI(const uint32_t i);
void uartPutI_safe(const uint32_t i);
void uartPutF(float f);
void uartPutF_safe(float f);

char uartGetC(void);
void uartGetS_safe(char *s);
int  uartGetI_safe(void);

void clearLine_safe(int len);

#endif // UART_H
