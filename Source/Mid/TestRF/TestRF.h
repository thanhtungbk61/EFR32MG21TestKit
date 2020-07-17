/*
 * TestRF.h
 *
 *  Created on: December 10, 2018
 *      Author: LongHD
 */

#ifndef SOURCE_UTILITY_TESTRF_TESTRF_H_
#define SOURCE_UTILITY_TESTRF_TESTRF_H_

/******************************************************************************/

/******************************************************************************/
/*                              INCLUDE FILES                                 */
/******************************************************************************/



/******************************************************************************/
/*                     EXPORTED TYPES and DEFINITIONS                         */
/******************************************************************************/

#define TEST_RF_MODE_ENABLE

#define CHANNEL26_MASK_TEST_RF                     0x04000000
#define IDENTIFY_CLUSTER_ID_TESTRF                 0x0003

#define TIME_RETRY_FIND_NETWORK_MS                 2000
#define TIME_SEND_MESSAGE_TEST_RSSI_MS             1500

#define TIMEOUT_TEST_RF_SECONDS                    60

#define RSSI_STRONG_SIGNAL                         (-60)
#define RSSI_WEAK_SIGNAL                           (-80)

#define RSSI_BUFFER_SIZE                           3

typedef void (*TestRFFinishCb) (uint32_t);

typedef enum _TESTRF_STATE_{
	findAndJoin,
	testRSSI,
} En_TestRFModeState;

typedef struct _TESTRF_MESSAGE_{
	uint16_t RFatrributeID;
	uint8_t  RFstatus;
	uint8_t  RFdataType;
	uint8_t  RFvalue[20];
} TestRFMessage;

/******************************************************************************/
/*                              PRIVATE DATA                                  */
/******************************************************************************/



/******************************************************************************/
/*                              EXPORTED DATA                                 */
/******************************************************************************/



/******************************************************************************/
/*                            FUNCTIONS                                       */
/******************************************************************************/

void TESTRF_InitCallbackFunc (TestRFFinishCb callbackInit);
boolean TESTRF_GetEnableTestRFMode (void);
boolean TESTRF_HandleIncomingMessage (EmberAfIncomingMessage* incomingMessage);
void TESTRF_EnableDisableRFMode (boolean enable);
void TESTRF_HandleFindNetworkResult (boolean JoinedNetwork);

/******************************************************************************/

#endif /* SOURCE_UTILITY_TESTRF_TESTRF_H_ */
