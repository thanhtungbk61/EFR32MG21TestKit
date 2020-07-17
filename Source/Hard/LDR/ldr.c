/*
 * adc.c
 *
 *  Created on: Jul 10, 2020
 *      Author: who_are_you
 */
#include "Source/Hard/LDR/ldr.h"
#include "app/framework/include/af.h"
#include "math.h"

void LDRInit(void)
{
  // Declare init structs
  IADC_Init_t init = IADC_INIT_DEFAULT;
  IADC_AllConfigs_t initAllConfigs = IADC_ALLCONFIGS_DEFAULT;
  IADC_InitSingle_t initSingle = IADC_INITSINGLE_DEFAULT;
  IADC_SingleInput_t initSingleInput = IADC_SINGLEINPUT_DEFAULT;

  // Enable IADC clock
  CMU_ClockEnable(cmuClock_IADC0, true);
  // enable pin

  // Reset IADC to reset configuration in case it has been modified
  IADC_reset(IADC0);

  // Configure IADC clock source for use while in EM2
  CMU_ClockSelectSet(cmuClock_IADCCLK, cmuSelect_FSRCO);

  // Modify init structs and initialize
  init.warmup = iadcWarmupKeepWarm;

  // Set the HFSCLK prescale value here
  init.srcClkPrescale = IADC_calcSrcClkPrescale(IADC0, CLK_SRC_ADC_FREQ, 0);

  // Configuration 0 is used by both scan and single conversions by default
  // Use unbuffered AVDD as reference
  initAllConfigs.configs[0].reference = iadcCfgReferenceVddx;

  // Divides CLK_SRC_ADC to set the CLK_ADC frequency for desired sample rate
  initAllConfigs.configs[0].adcClkPrescale = IADC_calcAdcClkPrescale(IADC0,
                                                                    CLK_ADC_FREQ,
                                                                    0,
                                                                    iadcCfgModeNormal,
                                                                    init.srcClkPrescale);

  // Set oversampling rate to 32x
  // resolution formula res = 11 + log2(oversampling * digital averaging)
  // in this case res = 11 + log2(32*1) = 16
  initAllConfigs.configs[0].osrHighSpeed = iadcCfgOsrHighSpeed32x;


  // Single initialization
  initSingle.dataValidLevel = _IADC_SINGLEFIFOCFG_DVL_VALID1;

  // Set conversions to run continuously
//  initSingle.triggerAction = iadcTriggerActionContinuous;

  // Set alignment to right justified with 16 bits for data field
//  initSingle.alignment = iadcAlignRight16;

  // Configure Input sources for single ended conversion
  initSingleInput.posInput = iadcPosInputPortCPin5;
  initSingleInput.negInput = iadcNegInputGnd;

  // Initialize IADC
  // Note oversampling and digital averaging will affect the offset correction
  // This is taken care of in the IADC_init() function in the emlib
  IADC_init(IADC0, &init, &initAllConfigs);

  // Initialize Scan
  IADC_initSingle(IADC0, &initSingle, &initSingleInput);

  // Allocate the analog bus for ADC0 inputs
  GPIO->IADC_INPUT_BUS |= GPIO_CDBUSALLOC_CDODD0_ADC0;  //IADC_INPUT_BUSALLOC

  // Enable interrupts on data valid level
//  IADC_enableInt(IADC0, IADC_IEN_SINGLEFIFODVL);

  // Enable ADC interrupts
//  NVIC_ClearPendingIRQ(IADC_IRQn);
//  NVIC_EnableIRQ(IADC_IRQn);
}

void readLDR(uint32_t* luxValue)
{
	uint32_t registor;
	IADC_Result_t iadcResult;
	IADC_command(IADC0, iadcCmdStartSingle);
	// Read data from the FIFO
	while((IADC0->STATUS & (_IADC_STATUS_CONVERTING_MASK | _IADC_STATUS_SINGLEFIFODV_MASK)) != _IADC_STATUS_SINGLEFIFODV_MASK);

    iadcResult = IADC_pullSingleFifoResult(IADC0);
//    emberAfCorePrintln("ADC:%d",iadcResult.data);
    registor= 10000*(4095 - iadcResult.data)/iadcResult.data;    // registor  = 10K*ADC / (4095 -ADC)
//    emberAfCorePrintln("registor:%d",registor);
    *luxValue = 316*pow(10,5)*pow(registor,-1.4);
}


void IADC_IRQHandler(void)
{
	uint32_t singleResult;
	IADC_Result_t sample;
  // Read data from the FIFO, 16-bit result
  sample = IADC_pullSingleFifoResult(IADC0);

  // For single-ended the result range is 0 to +Vref, i.e., 12 bits for the
  // conversion value.
  singleResult = sample.data * 3.3 / 0xFFF;

  IADC_clearInt(IADC0, IADC_IF_SINGLEFIFODVL);
  emberAfCorePrintln("IADC_IRQHandler----singleResult:%d----sample.data:%d",singleResult,sample.data);
}
