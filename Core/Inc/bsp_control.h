#ifndef __BSP_CONTROL_H_
#define __BSP_CONTROL_H_
#include "main.h"


#

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
