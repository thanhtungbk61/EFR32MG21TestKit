/*
 * TestRF.c
 *
 *  Created on: December 10, 2018
 *      Author: LongHD
 */

/******************************************************************************/

/******************************************************************************/
/*                              INCLUDE FILES                                 */
/******************************************************************************/

#include "app/framework/include/af.h"

//#define DEBUG_PRINT
//#include "Source/Utility/Debug/Debug.h"

#include "plugin/network-find/network-find.h"
#include "app/framework/plugin/network-steering/network-steering.h"
#include "Source/Hard/Led/led.h"
#include "TestRF.h"

/******************************************************************************/
/*                     EXPORTED TYPES and DEFINITIONS                         */
/******************************************************************************/



/******************************************************************************/
/*                              PRIVATE DATA                                  */
/******************************************************************************/

EmberEventControl testRFEventControl;
EmberEventControl delayRebootEventControl;
TestRFFinishCb TestRFFinishFunc;

static En_TestRFModeState en_TestRFState = findAndJoin;
int8_t i8_RSSIRx = -40, i8_RSSITx = -40;
static int16_t i16_RSSISend = -40;
static uint8_t u8_MaxTimeTestRF = 0;
static boolean b_TestRFModeEnable = false;
uint8_t rssiCnt = 0;

int8_t arr_RSSITx[RSSI_BUFFER_SIZE], arr_RSSIRx[RSSI_BUFFER_SIZE];

/******************************************************************************/
/*                              EXPORTED DATA                                 */
/******************************************************************************/



/******************************************************************************/
/*                            FUNCTIONS                                       */
/******************************************************************************/

void testRFEventFunction(void);
void delayRebootEventFunction(void);
void TESTRF_SendMessage (int8_t* rssi);
void TESTRF_UpdateLedForRSSI (void);
static int8_t TESTRF_GetAverageValue(int8_t arr[], uint8_t n);
static void TESTRF_AverageRSSI (int8_t rssiTx, int8_t rssiRx, int8_t* rssiTxResult, int8_t* rssiRxResult);

/******************************************************************************/

void TESTRF_InitCallbackFunc (TestRFFinishCb callbackInit){
	if (callbackInit != NULL){
		TestRFFinishFunc = callbackInit;
	}
}

boolean TESTRF_GetEnableTestRFMode (void){
	return b_TestRFModeEnable;
}

static int8_t TESTRF_GetAverageValue(int8_t arr[], uint8_t n){
	int16_t retVal = 0;
	for (uint8_t i = 0; i < n; i++){
		retVal += arr[i];
	}
	return (retVal / n);
}

static void TESTRF_AverageRSSI (int8_t rssiTx, int8_t rssiRx, int8_t* rssiTxResult, int8_t* rssiRxResult){
	static uint8_t u8_RSSIArrTxIndex = 0, u8_RSSIArrRxIndex = 0;
	static boolean b_AverageInit = false;

	if (b_AverageInit){
		arr_RSSITx [u8_RSSIArrTxIndex ++] = rssiTx;
		arr_RSSIRx [u8_RSSIArrRxIndex ++] = rssiRx;

		if (u8_RSSIArrRxIndex >= RSSI_BUFFER_SIZE){
			u8_RSSIArrTxIndex = 0; u8_RSSIArrRxIndex = 0;
		}
	}
	else {
		b_AverageInit = true;
		for (uint8_t i = 0; i < RSSI_BUFFER_SIZE; i ++){
			arr_RSSITx [i] = rssiTx;
			arr_RSSITx [i] = rssiRx;
		}
	}
	*rssiTxResult = TESTRF_GetAverageValue (arr_RSSITx, RSSI_BUFFER_SIZE);
	*rssiRxResult = TESTRF_GetAverageValue (arr_RSSITx, RSSI_BUFFER_SIZE);
}

void TESTRF_UpdateLedForRSSI (void){
	TESTRF_AverageRSSI(i8_RSSIRx, i8_RSSITx, &i8_RSSIRx, &i8_RSSITx);
//	emberAfCorePrintln("Outter-TESTRF_UpdateLedForRSSI: %d", i8_RSSIRx);
	if (rssiCnt == 5) {
		emberEventControlSetInactive (testRFEventControl);
//		emberAfCorePrintln("Inner-TESTRF_UpdateLedForRSSI: %d", i8_RSSIRx);
		if ((i8_RSSIRx > RSSI_STRONG_SIGNAL) && (i8_RSSITx > RSSI_STRONG_SIGNAL)){
	//		LED_BlinkTwoInterval(ledPink, TIME_LED_BLINK_FAST, TIME_LED_BLINK_FAST, 1, ledOff);
			emberAfCorePrintln("ledGreen");
			toggleLed(LED_2, ledGreen, 3);
		}
		else if ((i8_RSSIRx < RSSI_WEAK_SIGNAL) || (i8_RSSITx < RSSI_WEAK_SIGNAL)){
//			LED_BlinkTwoInterval(ledBlue, TIME_LED_BLINK_FAST, TIME_LED_BLINK_FAST, 1, ledOff);
			emberAfCorePrintln("ledBlue");
			toggleLed(LED_2, ledBlue, 3);
		}
		else {
//			LED_BlinkTwoInterval(ledRed, TIME_LED_BLINK_FAST, TIME_LED_BLINK_FAST, 1, ledOff);
			emberAfCorePrintln("ledRed");
			toggleLed(LED_2, ledRed, 3);
		}
		//Finish
		emberEventControlSetInactive (delayRebootEventControl);
		emberEventControlSetDelayMS(delayRebootEventControl, 5000);
	} else {
		rssiCnt++;
	}
}

boolean TESTRF_HandleIncomingMessage (EmberAfIncomingMessage* incomingMessage){
	uint16_t lengthCMD;
	int16_t  tempData;
	if (TESTRF_GetEnableTestRFMode() == false){
		return false;
	}

	if (incomingMessage -> apsFrame -> clusterId == 0x0003){
		i8_RSSIRx = incomingMessage -> lastHopRssi;
		i16_RSSISend = (uint16_t) (i8_RSSIRx);

		lengthCMD = incomingMessage -> msgLen;

		tempData = (int16_t) ( (((incomingMessage -> message[lengthCMD - 1]) << 8) & 0xFF00) | ((incomingMessage -> message[lengthCMD - 2]) & 0x00FF));
		i8_RSSITx = (int8_t) (tempData);

		TESTRF_UpdateLedForRSSI ();
		return true;
	}
	return false;
}

void TESTRF_EnableDisableRFMode (boolean enable){
	if (enable){
//		LED_ControlLed(ledOff);
		offLed(LED_1);
		offLed(LED_2);
		u8_MaxTimeTestRF = 0;
		b_TestRFModeEnable = true;
		emberEventControlSetDelayMS (testRFEventControl, 100);
	}
	else {
		b_TestRFModeEnable = false;
		if (TestRFFinishFunc != NULL){
			TestRFFinishFunc (1000);
		}
	}
}

void TESTRF_HandleFindNetworkResult (boolean JoinedNetwork){
	if (JoinedNetwork){
		en_TestRFState = testRSSI;
		u8_MaxTimeTestRF = 0;

		emberEventControlSetInactive (testRFEventControl);
		emberEventControlSetActive (testRFEventControl);
	}
	else {
		if (TestRFFinishFunc != NULL){
			TestRFFinishFunc (1000);
		}
	}
}

void testRFEventFunction(void){
	emberEventControlSetInactive (testRFEventControl);

	if (u8_MaxTimeTestRF > TIMEOUT_TEST_RF_SECONDS){
		if (TestRFFinishFunc != NULL){
			TestRFFinishFunc (100);
		}
	}

	switch (en_TestRFState){
	case findAndJoin:
		emberAfCorePrintln("Find network channel 26\n");
		u8_MaxTimeTestRF += 2;
		emAfPluginNetworkSteeringSetChannelMask (CHANNEL26_MASK_TEST_RF, 1);
		emAfPluginNetworkSteeringSetChannelMask (CHANNEL26_MASK_TEST_RF, 0);
		emAfPluginNetworkSteeringOptionsMask = EMBER_AF_PLUGIN_NETWORK_STEERING_OPTIONS_NO_TCLK_UPDATE;
		emberAfPluginNetworkSteeringStart();
		emberEventControlSetDelayMS (testRFEventControl, TIME_RETRY_FIND_NETWORK_MS);
		break;
	case testRSSI:
		emberAfCorePrintln("Send Test RSSI\n");
		u8_MaxTimeTestRF ++;
		TESTRF_SendMessage ((int8_t*) &i16_RSSISend);
		emberEventControlSetDelayMS (testRFEventControl, TIME_SEND_MESSAGE_TEST_RSSI_MS);
		break;
	default:
		break;
	}
}

void TESTRF_SendMessage (int8_t* rssi){
	TestRFMessage messageSend;
	uint16_t length;

	messageSend.RFatrributeID = 0x0000;
	messageSend.RFstatus = EMBER_SUCCESS;
	messageSend.RFdataType = ZCL_INT16S_ATTRIBUTE_TYPE;

    memcpy(messageSend.RFvalue, rssi, emberAfGetDataSize(ZCL_INT16S_ATTRIBUTE_TYPE));
    length = sizeof(messageSend.RFatrributeID)
            + sizeof(messageSend.RFstatus)
            + sizeof(messageSend.RFdataType) + emberAfGetDataSize(ZCL_INT16S_ATTRIBUTE_TYPE);

    emberAfFillCommandGlobalServerToClientReadAttributesResponse(IDENTIFY_CLUSTER_ID_TESTRF,
                                                                 &messageSend, length);
    emberAfSetCommandEndpoints(0x0001, 0x0001);
    emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, 0x0000);
}

void delayRebootEventFunction(void) {
	emberEventControlSetInactive (delayRebootEventControl);
	emberAfCorePrintln("delayRebootEventFunction");
	emberClearBindingTable();
	emberLeaveNetwork();
	TESTRF_EnableDisableRFMode(false);
}
