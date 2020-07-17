/*
 * PIR.c
 *
 *  Created on: Jul 17, 2020
 *      Author: who_are_you
 */
#include "Source/Hard/PIR/pir.h"



void PIRInit()
{
	GPIO_PinModeSet(PIR_PORT, PIR_PIN, gpioModeInput, 0);
}
bool readPIR()
{
	return GPIO_PinInGet(PIR_PORT, PIR_PIN);
}
