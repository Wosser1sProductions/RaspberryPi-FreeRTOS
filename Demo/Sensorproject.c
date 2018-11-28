/*
 * Sensorproject.c
 *
 *  Created on: 21 nov. 2018
 *      Author: William
 */

#include "Sensorproject.h"
#include <stdbool.h>
#include <FreeRTOS.h>
#include <task.h>

#include "Drivers/irq.h"
#include "Drivers/gpio.h"
#include "Drivers/uart.h"
#include "Drivers/i2c.h"
#include "Drivers/BMP180.h"

#define MAX(X,Y)	((X) > (Y) ? (X) : (Y))

static struct {
	I2C_t    *i2c;
	BMP180_t *bmp;

	float measured_temp;
	float measured_pres;

	uint32_t pwm_period;
} Locals;

static void putTemperatureColour(float t) {
	uartCmd(CONSOLE_BOLD);

	if (t < -10.0f) {
		uartCmd(CONSOLE_FG_BRIGHT_WHITE);
	} else if (t < 0.0f) {
		uartCmd(CONSOLE_FG_BRIGHT_CYAN);
	} else if (t < 10.0f) {
		uartCmd(CONSOLE_FG_CYAN);
	} else if (t < 20.0f) {
		uartCmd(CONSOLE_FG_BRIGHT_BLUE);
	} else if (t < 25.0f) {
		uartCmd(CONSOLE_FG_GREEN);
	} else if (t < 29.0f) {
		uartCmd(CONSOLE_FG_BRIGHT_YELLOW);
	} else if (t < 33.0f) {
		uartCmd(CONSOLE_FG_YELLOW);
	} else if (t < 38.0f) {
		uartCmd(CONSOLE_FG_BRIGHT_RED);
	} else {
		uartCmd(CONSOLE_FG_RED);
	}
}

static void taskButtons(void) {
	int p1 = 0, p2 = 0;

	uartPutS("Start Button task." NEWLINE);

	GpioDetectEdge(BUTTON_1_PIN);
	GpioDetectEdge(BUTTON_2_PIN);
	vTaskDelay(1);

	while (1) {
		p1 = GpioDetectEdge(BUTTON_1_PIN);
		p2 = GpioDetectEdge(BUTTON_2_PIN);

		if (p1) {
			Locals.pwm_period += PWM_PERIOD_MS_CHANGE;
		} else if (p2) {
			if (Locals.pwm_period >= PWM_PERIOD_MS_MIN + PWM_PERIOD_MS_CHANGE) {
				Locals.pwm_period -= PWM_PERIOD_MS_CHANGE;
			} else {
				Locals.pwm_period = PWM_PERIOD_MS_MIN;
			}
		}

		if (p1 | p2) {
			uartCmd(CONSOLE_RESET);
			uartCmd(CONSOLE_FG_MAGENTA);
			uartPutS("Period changed to ");
			uartPutI(Locals.pwm_period);
			uartPutS(NEWLINE);
			uartCmd(CONSOLE_RESET);
		}

		vTaskDelay(250);
	}
}

static void taskLED(void) {
	int i, delay = 0;

	uartPutS("Start LED task." NEWLINE);

	while (true) {
		if (Locals.pwm_period <= PWM_PERIOD_MS_MIN) {
			SetGpio(LED_PIN, 1);
		} else {
			for (i = 1; i < Locals.pwm_period; i++) {
				const uint32_t max = MAX((int)Locals.pwm_period - i, 1);

				SetGpio(LED_PIN, 1);
				vTaskDelay(i);
				SetGpio(LED_PIN, 0);
				vTaskDelay(max);
			}

			for (i = 1; i < Locals.pwm_period; i++) {
				const uint32_t max = MAX((int)Locals.pwm_period - i, 1);

				SetGpio(LED_PIN, 1);
				vTaskDelay(max);
				SetGpio(LED_PIN, 0);
				vTaskDelay(i);
			}
		}
	}
}

static void taskSensor(void) {
	uartPutS("Start Sensor task." NEWLINE);

//	if (Locals.bmp == NULL) {
//		uartCmd(CONSOLE_FG_RED);
//		uartPutS("ERROR Sensor not initialised!." NEWLINE);
//		uartCmd(CONSOLE_RESET);
//		return;
//	}

	while (true) {
		//BMP180_readData(Locals.bmp, &Locals.measured_temp, &Locals.measured_pres);
		Locals.measured_temp += 1.0f; Locals.measured_pres += 1.0f;
		vTaskDelay(1000);
	}
}

static void taskMain(void) {
	uartPutS("Start main task." NEWLINE);

	while (true) {
		uartCmd(CONSOLE_FG_WHITE);
		uartPutS("Sensor temperature: ");
		putTemperatureColour(Locals.measured_temp);
		uartPutF(Locals.measured_temp);
		uartPutS(" " DEGREES_CELCIUS NEWLINE);
		uartCmd(CONSOLE_RESET);

		uartCmd(CONSOLE_FG_WHITE);
		uartPutS("   Sensor pressure: ");
		uartCmd(CONSOLE_BOLD);
		uartCmd(CONSOLE_FG_BRIGHT_WHITE);
		uartPutF(Locals.measured_pres);
		uartPutS(" hPa" NEWLINE);
		uartCmd(CONSOLE_RESET);

		vTaskDelay(5000);
	}
}

void SP_initHardware(void) {
	uartCmd(CONSOLE_FG_MAGENTA);
    uartPutS("Setting pins..." NEWLINE);

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

    uartCmd(CONSOLE_FG_WHITE);

    Locals.bmp = BMP180_create();
    if (!BMP180_initialise(Locals.bmp, Locals.i2c, 35.75f, OSS_Setting)) {
    	BMP180_destroy(&Locals.bmp);
    }

    Locals.pwm_period = PWM_PERIOD_MS;

    uartCmd(CONSOLE_RESET);
}

void SP_startTasks(void) {
	xTaskCreate(taskSensor , "taskSensor" , 128, NULL, 0, NULL);
	xTaskCreate(taskMain   , "taskMain"   , 128, NULL, 1, NULL);
	xTaskCreate(taskLED    , "taskLED"    , 128, NULL, 2, NULL);
	xTaskCreate(taskButtons, "taskButtons", 128, NULL, 0, NULL);
}
