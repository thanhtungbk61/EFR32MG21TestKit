/*
 * si7210.h
 *
 *  Created on: Jul 11, 2020
 *      Author: who_are_you
 */

#ifndef SOURCE_HARD_SI7210_SI7210_H_
#define SOURCE_HARD_SI7210_SI7210_H_
// define bit

#define ONEBURT					BIT(2)
#define STOP					BIT(1)
#define SLEEP					BIT(0)
#define USESTORE				BIT(4)
/* I2C device address for Si7020 */
#define SI7210_ADDR            0x66

/* Device ID value for Si7020 */
#define SI7210_DEVICE_ID       0x14


#define SI7210_READ_ID       0xC0

#define SI7210_READ_DATA_MSB	0xC1
#define SI7210_READ_DATA_LSB	0xC2
#define SI7210_START_CONVERT_REGISTER_ADDRESS	0xC4
#define SI7210_START_CONVERT_REGISTER_DATA	0x04


bool Si7210_CheckInit ();
bool Si7210_ReadMagnetic(uint16_t* data);
#endif /* SOURCE_HARD_SI7210_SI7210_H_ */
