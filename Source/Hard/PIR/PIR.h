/*
 * PIR.h
 *
 *  Created on: Jul 17, 2020
 *      Author: who_are_you
 */
#include "stdbool.h"
#include "stddef.h"
#include "em_cmu.h"
#include "em_gpio.h"

#ifndef SOURCE_HARD_PIR_PIR_H_
#define SOURCE_HARD_PIR_PIR_H_

#define PIR_PORT			gpioPortC
#define PIR_PIN				(4U)


void PIRInit();
bool readPIR();

#endif /* SOURCE_HARD_PIR_PIR_H_ */
