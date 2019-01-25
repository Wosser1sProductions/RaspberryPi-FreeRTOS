/*
 * Sensorproject.h
 *
 *  Created on: 21 nov. 2018
 *      Author: William
 */

#ifndef DEMO_SENSORPROJECT_H_
#define DEMO_SENSORPROJECT_H_

#define BUTTON_1_PIN	20
#define BUTTON_2_PIN	26
#define LED_PIN			21
#define UARTS_TX_PIN	14
#define UARTS_RX_PIN	15
#define I2C_SDA_PIN		2
#define I2C_SCL_PIN		3

#define PWM_PERIOD_MS			25
#define PWM_PERIOD_MS_CHANGE 	2
#define PWM_PERIOD_MS_MIN	 	4

void SP_initHardware(void);
void SP_startTasks(void);

#endif /* DEMO_SENSORPROJECT_H_ */
