/*
 * si7210.c
 *
 *  Created on: Jul 11, 2020
 *      Author: who_are_you
 */


#include "app/framework/include/af.h"
#include "stdbool.h"
#include "em_i2c.h"
//#include "Source/Utility/Debug.h"
#include "Source/Hard/I2C/i2c.h"
#include "si7210.h"

static bool Si7210_Read(uint8_t registerAddress ,uint8_t* data);
static bool Si7210_StartConvert();

bool Si7210_CheckInit (){
	I2C_TransferSeq_TypeDef    seq;
	I2C_TransferReturn_TypeDef ret;
	uint8_t                    i2c_read_data[1];
	uint8_t                    i2c_write_data[1];

	seq.addr  = SI7210_ADDR;
	seq.flags = I2C_FLAG_WRITE_READ;
	/* Select command to issue */
	i2c_write_data[0] = SI7210_READ_ID;
	seq.buf[0].data   = i2c_write_data;
	seq.buf[0].len    = 1;
	/* Select location/length of data to be read */
	seq.buf[1].data = i2c_read_data;
	seq.buf[1].len  = 1;

	ret = I2C_TransferInit(I2C0, &seq);
	// Sending data
	while (ret == i2cTransferInProgress)
	{
		ret = I2C_Transfer(I2C0);
	}
	if ((ret != i2cTransferDone) | (i2c_read_data[0] != SI7210_DEVICE_ID)) {

		emberAfCorePrintln ("Detect Si7210 failure!--Device Id:%d -- return:%d ",i2c_read_data[0],ret);
		return false;
	}
	emberAfCorePrintln ("Detect Si7210 success! ");
	return ret;
}


bool Si7210_ReadMagnetic(uint16_t* data)
{
	if(Si7210_StartConvert() != 0)
		return -1;
	for(int i =0; i<65535; i++);  // wait to coplete convert
	uint8_t dataLsb, dataMsb;
	uint8_t ret = -1;
	ret  = Si7210_Read(SI7210_READ_DATA_MSB, &dataMsb);
	ret += Si7210_Read(SI7210_READ_DATA_LSB, &dataLsb);
	dataMsb = (dataMsb&0x7F);
	*data = ( dataMsb << 8) + dataLsb - 16384;   // B = (256*Dspsigm[6:0]+Dspsigl[7:0] -16384)* (0.00125 or 0.0125)
	//emberAfCorePrintln("dataHigh:%d---dataRead:%d",dataMsb,*data);
	return ret;
}


static bool Si7210_Read(uint8_t registerAddress ,uint8_t* data)
{
	I2C_TransferSeq_TypeDef    seq;
	I2C_TransferReturn_TypeDef ret;
	uint8_t                    i2c_read_data[1];
	uint8_t                    i2c_write_data[1];


	seq.addr  = SI7210_ADDR;
	seq.flags = I2C_FLAG_WRITE_READ;
	/* Select command to issue */
	i2c_write_data[0] = registerAddress;
	seq.buf[0].data   = i2c_write_data;
	seq.buf[0].len    = 1;
	/* Select location/length of data to be read */
	seq.buf[1].data = i2c_read_data;
	seq.buf[1].len  = 1;

	ret = I2C_TransferInit(I2C0, &seq);
	// Sending data
	while (ret == i2cTransferInProgress)
	{
		ret = I2C_Transfer(I2C0);
	}
	*data = i2c_read_data[0];
	//emberAfCorePrintln("read_data:%d",i2c_read_data[0]);

	return ret;
}


static bool Si7210_StartConvert()
{
	uint8_t buyteConfigStart;
	//need to  read register first then just change the bit have to change to convert
	Si7210_Read(SI7210_START_CONVERT_REGISTER_ADDRESS,&buyteConfigStart);

	buyteConfigStart |= ONEBURT;
	buyteConfigStart &= ~(STOP);

	I2C_TransferSeq_TypeDef    seq;
	I2C_TransferReturn_TypeDef ret;
	uint8_t                    i2c_write_data[2];
	i2c_write_data[0] = SI7210_START_CONVERT_REGISTER_ADDRESS;
	i2c_write_data[1] = buyteConfigStart;


	seq.addr  = SI7210_ADDR;
	seq.flags = I2C_FLAG_WRITE;
	/* Select command to issue */
	seq.buf[0].data   = i2c_write_data;
	seq.buf[0].len    = 2;

	ret = I2C_TransferInit(I2C0, &seq);
	// Sending data
	while (ret == i2cTransferInProgress)
	{
		ret = I2C_Transfer(I2C0);
	}
	return ret;
}
