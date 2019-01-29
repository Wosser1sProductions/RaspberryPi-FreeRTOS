/*
 * utils.h
 *
 *  Created on: 24 okt. 2018
 *      Author: William
 */

#ifndef DEMO_UTILS_H_
#define DEMO_UTILS_H_

#define ASMNOP()    __asm volatile ("nop")

#define wait_cycles(c) \
	do { \
		for (volatile int i = c; i--;) ASMNOP(); \
	} while (0)


#define UNUSED(x) (void)x

#endif /* DEMO_UTILS_H_ */
