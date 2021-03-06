/*
 * ExampleButtonGame.c
 *
 *  Created on: 24 okt. 2018
 *      Author: William
 */

#include "ExampleButtonGame.h"
#include <stdbool.h>
#include <FreeRTOS.h>
#include <task.h>

#include "Drivers/irq.h"
#include "Drivers/gpio.h"
#include "Drivers/uart.h"

struct {
	bool btn1_pressed;
	bool btn2_pressed;
	uint8_t round;
	uint8_t score_p1;
	uint8_t score_p2;
	bool won;
	bool quit;

	char win_msg_p1[MESSAGE_BUFFER];
	char win_msg_p2[MESSAGE_BUFFER];
} Game;

//void irq_button(unsigned int irq, void *pParam) {
//	irqBlock();
//	const int pin = (int)(*pParam);
//
//	if (!Game.won) {
//		switch (pin) {
//			case BUTTON_1_PIN:
//				Game.won = Game.btn1_pressed = true;
//				break;
//			case BUTTON_2_PIN:
//				Game.won = Game.btn2_pressed = true;
//				break;
//		}
//	}
//
//	ClearGpioInterrupt(pin);
//	irqUnblock();
//}

static void taskButtons(void) {
	int p1 = 0, p2 = 0;

	uartPutS_safe("Start button task." NEWLINE);
	GpioDetectEdge(BUTTON_1_PIN);
	GpioDetectEdge(BUTTON_2_PIN);
	vTaskDelay(1 / portTICK_RATE_MS);

	while (1) {
		if (!Game.won) {
			p1 = GpioDetectEdge(BUTTON_1_PIN);
			p2 = GpioDetectEdge(BUTTON_2_PIN);

			if (p1) {
				SetGpio(LED_PIN, 0);
				Game.won = Game.btn1_pressed = true;
			} else if (p2) {
				SetGpio(LED_PIN, 0);
				Game.won = Game.btn2_pressed = true;
			}
		} else if (Game.quit) {
			GpioDetectEdge(BUTTON_1_PIN);
			GpioDetectEdge(BUTTON_2_PIN);
		}
		vTaskDelay(1 / portTICK_RATE_MS);
	}
}

static void taskGameloop(void) {
	int i;

	uartPutS_safe("Start game task." NEWLINE);

	do {
		Game.won = Game.quit = true;
		vTaskDelay(10 / portTICK_RATE_MS);
		Game.btn1_pressed = Game.btn2_pressed = false;
		Game.round = Game.score_p1 = Game.score_p2 = 0;

		uart_lock();
		uartPutS("Enter win message for Player 1 (button pin ");
		uartPutI(BUTTON_1_PIN);
		uartPutS("): ");
		uart_unlock();
		uartGetS_safe(Game.win_msg_p1);
		clearLine_safe(50 + MESSAGE_BUFFER);

		uart_lock();
		uartPutS("\rEnter win message for Player 2 (button pin ");
		uartPutI(BUTTON_2_PIN);
		uartPutS("): ");
		uart_unlock();
		uartGetS_safe(Game.win_msg_p2);
		clearLine_safe(50 + MESSAGE_BUFFER);

		uart_lock();
		uartCmd(CONSOLE_CLS);
		uartCmd(CONSOLE_CURSOR);
		uart_unlock();

		while (Game.round < GAME_ROUNDS + 1) {
			if (Game.won) {
				uartCmd(CONSOLE_FG_GREEN);

				if (Game.btn1_pressed) {
					Game.score_p1++;
					uartPutS_safe("Player 1 wins this round!" NEWLINE);
				} else if (Game.btn2_pressed) {
					Game.score_p2++;
					uartPutS_safe("Player 2 wins this round!" NEWLINE);
				}

				Game.btn1_pressed = Game.btn2_pressed = false;

				if (Game.round == GAME_ROUNDS) break;

				uart_lock();
				uartCmd(CONSOLE_FG_BRIGHT_YELLOW);
				uartPutS(NEWLINE "Round ");
				uartPutI(++Game.round);
				uartPutS(NEWLINE);

				uartCmd(CONSOLE_RESET);
				uart_unlock();

				for (i = 3; i--;) {
					uart_lock();
					uartPutS("New game starts in ");
					uartPutI(i);
					uartPutS("...\r");
					uart_unlock();
					vTaskDelay(1000 / portTICK_RATE_MS);
				}

				uartPutS_safe(NEWLINE);
				SetGpio(LED_PIN, 1);
				Game.won = false;
			}

			vTaskDelay(10 / portTICK_RATE_MS);
		}

		Game.won  = true;
		Game.quit = false;
		vTaskDelay(10 / portTICK_RATE_MS);

		uart_lock();
		uartCmd(CONSOLE_FG_RED);
		uartPutS(NEWLINE "Game over." NEWLINE NEWLINE);
		uartCmd(CONSOLE_FG_CYAN);
		uartPutS("Winner: ");
		uartCmd(CONSOLE_FG_GREEN);

		i = Game.score_p1 > Game.score_p2;

		if (i) {
			uartPutS("Player 1 ");
		} else {
			uartPutS("Player 2 ");
		}

		uartCmd(CONSOLE_FG_WHITE);
		uartPutS("with a score of ");
		uartCmd(CONSOLE_FG_BRIGHT_RED);
		uartPutI(i ? Game.score_p1 : Game.score_p2);
		uartCmd(CONSOLE_RESET);
		uart_unlock();
		vTaskDelay(1 / portTICK_RATE_MS);

		uart_lock();
		if (i) {
			uartPutS(NEWLINE "Player 1 says: ");
			uartCmd(CONSOLE_FG_BRIGHT_MAGENTA);
			uartPutS(Game.win_msg_p1);
			uartPutS(NEWLINE);
		} else {
			uartPutS(NEWLINE "Player 2 says: ");
			uartCmd(CONSOLE_FG_BRIGHT_MAGENTA);
			uartPutS(Game.win_msg_p2);
			uartPutS(NEWLINE);
		}

		uartCmd(CONSOLE_RESET);
		uartPutS(NEWLINE NEWLINE);

		uartCmd(CONSOLE_FG_YELLOW);
		uartPutS("To restart, press both buttons at the same time..." NEWLINE);
		uart_unlock();

		while (true) {
			int b1 = GpioDetectEdge(BUTTON_1_PIN);
			vTaskDelay(50 / portTICK_RATE_MS);
			int b2 = GpioDetectEdge(BUTTON_2_PIN);
			vTaskDelay(50 / portTICK_RATE_MS);

			if (b1 && b2) {
				Game.quit = false;
				break;
			}
		}

		vTaskDelay(10 / portTICK_RATE_MS);
		uart_lock();
		uartCmd(CONSOLE_RESET);
		uartCmd(CONSOLE_CLS);
		uartCmd(CONSOLE_CURSOR);
		uart_unlock();
	} while (!Game.quit);

	vTaskDelete(NULL);
};

void BG_initHardware(void) {
	uart_lock();
	uartCmd(CONSOLE_FG_MAGENTA);
    uartPutS("Setting pins for game..." NEWLINE);
    uart_unlock();

    SetGpioFunction(BUTTON_1_PIN, 1);
    SetGpioDirection(BUTTON_1_PIN, GPIO_IN);
    PutGpio(BUTTON_1_PIN, PULL_UP);
    EnableGpioDetect(BUTTON_1_PIN, DETECT_FALLING);

    SetGpioFunction(BUTTON_2_PIN, 1);
    SetGpioDirection(BUTTON_2_PIN, GPIO_IN);
    PutGpio(BUTTON_2_PIN, PULL_UP);
    EnableGpioDetect(BUTTON_2_PIN, DETECT_FALLING);

    SetGpioFunction(LED_PIN, 1);
    SetGpioDirection(LED_PIN, GPIO_OUT);
    SetGpio(LED_PIN, 0);

//    irqRegister(BCM2835_IRQ_ID_GPIO_0, (INTERRUPT_VECTOR) { irq_button, BUTTON_1_PIN});
//    irqRegister(BCM2835_IRQ_ID_GPIO_1, (INTERRUPT_VECTOR) { irq_button, BUTTON_2_PIN});
    uartCmd(CONSOLE_RESET);
}

void BG_startTasks(void) {
	xTaskCreate(taskButtons , "tskButtons" , 128, NULL, 1, NULL);
	xTaskCreate(taskGameloop, "tskGameloop", 256, NULL, 0, NULL);
}
