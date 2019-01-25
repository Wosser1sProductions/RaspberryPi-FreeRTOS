#ifndef UART_H
#define UART_H

#include <stdint.h>

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

static const char* CONSOLE_LUT[] = {
	"\033[2J",    ///< CONSOLE_CLS
	"\033[H",     ///< CONSOLE_CURSOR
	"\033[0m",    ///< CONSOLE_RESET

	"\033[1m",    ///< CONSOLE_BOLD
	"\033[4m",    ///< CONSOLE_UNDERLINE

	"\033[30m",   ///< CONSOLE_FG_BLACK
	"\033[30;1m", ///< CONSOLE_FG_BRIGHT_BLACK
	"\033[31m",   ///< CONSOLE_FG_RED
	"\033[31;1m", ///< CONSOLE_FG_BRIGHT_RED
	"\033[32m",   ///< CONSOLE_FG_GREEN
	"\033[32;1m", ///< CONSOLE_FG_BRIGHT_GREEN
	"\033[33m",   ///< CONSOLE_FG_YELLOW
	"\033[33;1m", ///< CONSOLE_FG_BRIGHT_YELLOW
	"\033[34m",   ///< CONSOLE_FG_BLUE
	"\033[34;1m", ///< CONSOLE_FG_BRIGHT_BLUE
	"\033[35m",   ///< CONSOLE_FG_MAGENTA
	"\033[35;1m", ///< CONSOLE_FG_BRIGHT_MAGENTA
	"\033[36m",   ///< CONSOLE_FG_CYAN
	"\033[36;1m", ///< CONSOLE_FG_BRIGHT_CYAN
	"\033[37m",   ///< CONSOLE_FG_WHITE
	"\033[37;1m", ///< CONSOLE_FG_BRIGHT_WHITE

	"\033[40m",   ///< CONSOLE_BG_BLACK
	"\033[40;1m", ///< CONSOLE_BG_BRIGHT_BLACK
	"\033[41m",   ///< CONSOLE_BG_RED
	"\033[41;1m", ///< CONSOLE_BG_BRIGHT_RED
	"\033[42m",   ///< CONSOLE_BG_GREEN
	"\033[42;1m", ///< CONSOLE_BG_BRIGHT_GREEN
	"\033[43m",   ///< CONSOLE_BG_YELLOW
	"\033[43;1m", ///< CONSOLE_BG_BRIGHT_YELLOW
	"\033[44m",   ///< CONSOLE_BG_BLUE
	"\033[44;1m", ///< CONSOLE_BG_BRIGHT_BLUE
	"\033[45m",   ///< CONSOLE_BG_MAGENTA
	"\033[45;1m", ///< CONSOLE_BG_BRIGHT_MAGENTA
	"\033[46m",   ///< CONSOLE_BG_CYAN
	"\033[46;1m", ///< CONSOLE_BG_BRIGHT_CYAN
	"\033[47m",   ///< CONSOLE_BG_WHITE
	"\033[47;1m"  ///< CONSOLE_BG_BRIGHT_WHITE
};

#define uartCmd(C) uartPutS(CONSOLE_LUT[(C)])

int uartEnableInterrupt();

int uart_interrupt_handler(unsigned int irq, void *pParam);

void uartPutC(char byte);
void uartPutS(const char *s);
void uartPutI(const uint32_t i);
void uartPutF(float f);

char uartGetC(void);
void uartGetS(char *s);
int  uartGetI(void);

void clearLine(int len);

#endif // UART_H
