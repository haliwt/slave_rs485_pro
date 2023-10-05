#ifndef __CONTROL_H_
#define __CONTROL_H_
#include "main.h"


#define PTC_IO_SetHigh()       HAL_GPIO_WritePin(RELAY_SIGNAL_GPIO_Port,RELAY_SIGNAL_Pin,GPIO_PIN_SET)
#define PTC_IO_SetLow()        HAL_GPIO_WritePin(RELAY_SIGNAL_GPIO_Port,RELAY_SIGNAL_Pin,GPIO_PIN_RESET)

#define PLASMA_IO_SetHigh()		HAL_GPIO_WritePin(PLASMA_SIGNAL_GPIO_Port,PLASMA_SIGNAL_Pin ,GPIO_PIN_SET)
#define PLASMA_IO_SetLow()		HAL_GPIO_WritePin(PLASMA_SIGNAL_GPIO_Port,PLASMA_SIGNAL_Pin ,GPIO_PIN_RESET)

//#define ULTRASONIC_IO_SetHigh()		HAL_GPIO_WritePin(PLASMA_SIGNAL_GPIO_Port,PLASMA_SIGNAL_Pin ,GPIO_PIN_SET)
//#define ULTRASONIC_IO_SetLow();

typedef struct {

    uint8_t gPower_On;
	uint8_t gFan_continueRun;
	uint8_t  gFan_counter;
	uint8_t gTimer_fan_adc_times;
	uint8_t gFan_level;

	//adc 
	uint8_t ADC_channel_No;

	uint8_t  gPlasma;
    uint8_t  gPtc;
    uint8_t  gUltrasonic;

	uint8_t  gTemperature;
    uint8_t  gHumidity;

	//fault 
	uint8_t fan_warning ;
	uint8_t ptc_warning;


}MAINBOARD_T;

extern MAINBOARD_T g_tMain;


void Mainboard_Run_Process_Handler(void);







#endif 
