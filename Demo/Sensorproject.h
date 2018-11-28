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
#define I2C_TX_PIN		0
#define I2C_RX_PIN		0

#define PWM_PERIOD_MS			25
#define PWM_PERIOD_MS_CHANGE 	2
#define PWM_PERIOD_MS_MIN	 	4

void SP_initHardware(void);
void SP_startTasks(void);

#endif /* DEMO_SENSORPROJECT_H_ */
