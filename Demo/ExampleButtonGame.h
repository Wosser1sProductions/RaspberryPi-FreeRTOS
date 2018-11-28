/*
 * ExampleButtonGame.h
 *
 *  Created on: 24 okt. 2018
 *      Author: William
 */

#ifndef DEMO_DRIVERS_EXAMPLEBUTTONGAME_H_
#define DEMO_DRIVERS_EXAMPLEBUTTONGAME_H_

#define BUTTON_1_PIN	20
#define BUTTON_2_PIN	26
#define LED_PIN			21

#define GAME_ROUNDS		9

#define MESSAGE_BUFFER	80

void BG_initHardware(void);
void BG_startTasks(void);

#endif /* DEMO_DRIVERS_EXAMPLEBUTTONGAME_H_ */
