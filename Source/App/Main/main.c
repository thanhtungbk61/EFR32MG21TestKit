/*
 * main.c
 *
 *  Created on: Jul 8, 2020
 *      Author: who_are_you
 */
#include "Source/Hard/LDR/ldr.h"
#include "app/framework/include/af.h"
#include EMBER_AF_API_NETWORK_STEERING

#define TIMEREPORT			3000
// include user lib
#include "Source/Hard/Button/button.h"
#include "Source/Hard/Led/led.h"
#include "Source/Hard/Si7020/si7020.h"
#include "Source/Hard/Si7210/si7210.h"
#include "Source/Hard/I2C/i2c.h"
#include "Source/Hard/PIR/pir.h"
#include "Source/Mid/TestRF/TestRF.h"
// Event control struct declaration
EmberEventControl myDelayEventControl;
EmberEventControl reportSensorEventControl;
EmberEventControl testLed_1_EventControl;
EmberEventControl testLed_2_EventControl;


// Event function forward declaration
void myDelayEventHandle(void);
void reportSensorEventHandle(void);
void testLed_1_EventHandle(void);
void testLed_2_EventHandle(void);
//

// user callbackFunction
void buttonPressAndHoldCallbackHandle(buttonNumber button, uint8_t pressTime, uint8_t holdTime);
void buttonHoldCallbackIndicate(buttonNumber button, uint8_t holdTime);
void testRFCallbackHandle(uint32_t);


// main init
uint8_t ledBlinkTime = 1;
// test Led
bool button_1_state_test = false;
bool button_2_state_test = false;
uint8_t led_1_color_test = ledRed;
uint8_t led_2_color_test = ledRed;

uint32_t ldrLuxVaue;
bool enableSi7020 = false;
bool enableSi7210 = false;
void emberAfMainInitCallback(void)
{
	//
	emberAfCorePrintln("emberAfMainInitCallback");
	buttonInit();
	ledInit();
	i2cInit();
	LDRInit();
	PIRInit();

	if( Si7020_CheckInit() == 0)
		enableSi7020 = true;
	if( Si7210_CheckInit() == 0)
		enableSi7210 = true;
	// button callback register
	buttonPressTimeCallbackInit(buttonPressAndHoldCallbackHandle);
	buttonHoldCallbackInit(buttonHoldCallbackIndicate);
	TESTRF_InitCallbackFunc(testRFCallbackHandle);
	//
	emberEventControlSetDelayMS(myDelayEventControl,500);
	emberEventControlSetDelayMS(reportSensorEventControl,TIMEREPORT);
}


// Event function stub
void myDelayEventHandle(void)
{
//	Si7210_ReadData(&si7210Data);
	emberEventControlSetInactive(myDelayEventControl);
	testLed();
	if(ledBlinkTime != 0)
	{
		ledBlinkTime --;
		emberEventControlSetDelayMS(myDelayEventControl,TEST_LED);
	}
	else
	{

		offLed(LED_1);
		offLed(LED_2);
	}
}

void reportSensorEventHandle(void)
{
	uint32_t humi, temp, lux;
	uint16_t magnetic;
	uint8_t status = -1;
	bool pirData =0;
	emberEventControlSetInactive(reportSensorEventControl);
	if( enableSi7020 == true)
	{
		status = Si7020_MeasureTempAndHumi(&humi, &temp);
		if(status == 0)
			emberAfCorePrintln("Humi:%d\nTemp:%d",humi, temp);
	}

	if( enableSi7210 == true)
	{
		status = Si7210_ReadMagnetic(&magnetic);
		if(status == 0)
			emberAfCorePrintln("Magnetic:%d",magnetic);
	}
	//
	readLDR(&lux);
	emberAfCorePrintln("Lux:%d",lux);
	// read PIR
	pirData = readPIR();
	emberAfCorePrintln("PIR value: %d",pirData);

	emberEventControlSetDelayMS(reportSensorEventControl,TIMEREPORT);

}


void testLed_1_EventHandle(void)
{
	emberEventControlSetInactive(testLed_1_EventControl);
	if(button_1_state_test == true)
	{
		switch(led_1_color_test)
		{
			case ledRed:
				onLed(LED_1,ledRed);
				led_1_color_test = ledGreen;
				break;
			case ledGreen:
				onLed(LED_1,ledGreen);
				led_1_color_test = ledBlue;
				break;
			case ledBlue:
				onLed(LED_1,ledBlue);
				led_1_color_test = ledRed;
				break;
			default:
				led_1_color_test = ledRed;
				break;
		}
		emberEventControlSetDelayMS(testLed_1_EventControl, 1000);
	}
	else
	{
		offLed(LED_1);
		led_1_color_test = ledRed;
	}
}

void testLed_2_EventHandle(void)
{
	emberEventControlSetInactive(testLed_2_EventControl);
	if(button_2_state_test == true)
	{
		switch(led_2_color_test)
		{
			case ledRed:
				onLed(LED_2,ledRed);
				led_2_color_test = ledGreen;
				break;
			case ledGreen:
				onLed(LED_2,ledGreen);
				led_2_color_test = ledBlue;
				break;
			case ledBlue:
				onLed(LED_2,ledBlue);
				led_2_color_test = ledRed;
				break;
			default:
				led_2_color_test = ledRed;
				break;
		}
		emberEventControlSetDelayMS(testLed_2_EventControl, 1000);
	}
	else
	{
		offLed(LED_2);
		led_2_color_test = ledRed;
	}
}
void buttonHoldCallbackIndicate(buttonNumber button, uint8_t holdTime)
{
//	offLed();
	emberAfCorePrintln("buttonHoldCallbackHandle--buttonNumber:%d----holdTime:%d",button,holdTime);
}


void buttonPressAndHoldCallbackHandle(buttonNumber button, uint8_t pressTime, uint8_t holdTime)
{
	if(holdTime > HOLD_TIME_MIN)
	{
		emberAfCorePrintln("buttonHoldTimeCallbackHandle--buttonNumber:%d----holdTime:%d",button,holdTime);
	}
	else
	{
		emberAfCorePrintln("buttonPressTimeCallbackHandle--buttonNumber:%d----pressTime:%d",button,pressTime);
		switch (pressTime)
		{
			case press_1:
				if(button == BUTTON_1)
				{

					// test Si7210
			//		uint16_t si7210Data;
			//		Si7210_ReadData(&si7210Data);
			//		emberAfCorePrintln("Si7210Data:%d",si7210Data);

					// test LDR
			//		readLDR(&ldrLuxVaue);
			//		emberAfCorePrintln("ldrLuxVaue:%d---",ldrLuxVaue);
					if(button_1_state_test == false)
					{
						button_1_state_test = true;
						emberEventControlSetActive(testLed_1_EventControl);
					}
					else
					{
						button_1_state_test = false;
						emberEventControlSetActive(testLed_1_EventControl);
					}

				}
				else
				{
					if(button_2_state_test == false)
					{
						button_2_state_test = true;
						emberEventControlSetActive(testLed_2_EventControl);
					}
					else
					{
						button_2_state_test = false;
						emberEventControlSetActive(testLed_2_EventControl);
					}
				}
				break;
			case press_2:
				emberAfCorePrintln("\nTEST RF\n");
				emberEventControlSetInactive(reportSensorEventControl);
				TESTRF_EnableDisableRFMode(true);
				break;
			case press_3:
				break;
			case press_4:
				break;
			default:
				break;
		}
	}


}

void testRFCallbackHandle(uint32_t rssi) {
	emberAfCorePrintln("\nRESET FACTORY\n");
	halReboot();
}

//emberAfFillCommandGlobalServerToClientReadAttributesResponse();


/** @brief Stack Status
 *
 * This function is called by the application framework from the stack status
 * handler.  This callbacks provides applications an opportunity to be notified
 * of changes to the stack status and take appropriate action.  The return code
 * from this callback is ignored by the framework.  The framework will always
 * process the stack status after the callback returns.
 *
 * @param status   Ver.: always
 */
boolean emberAfStackStatusCallback(EmberStatus status) {
	if (status == EMBER_NETWORK_UP) {
		TESTRF_HandleFindNetworkResult (true);
	} else if (status == EMBER_NETWORK_DOWN) {
//		TESTRF_HandleFindNetworkResult (false);
	}
	return true;
}

boolean emberAfPreMessageReceivedCallback(EmberAfIncomingMessage* incomingMessage){
	return TESTRF_HandleIncomingMessage(incomingMessage);
}
