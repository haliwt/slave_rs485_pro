#ifndef __BSP_CONTROL_H_
#define __BSP_CONTROL_H_
#include "main.h"


#define PTC_IO_SetHigh()       HAL_GPIO_WritePin(RELAY_SIGNAL_GPIO_Port,RELAY_SIGNAL_Pin,GPIO_PIN_SET)
#define PTC_IO_SetLow()        HAL_GPIO_WritePin(RELAY_SIGNAL_GPIO_Port,RELAY_SIGNAL_Pin,GPIO_PIN_RESET)

#define PLASMA_IO_SetHigh()		HAL_GPIO_WritePin(PLASMA_SIGNAL_GPIO_Port,PLASMA_SIGNAL_Pin ,GPIO_PIN_SET)
#define PLASMA_IO_SetLow()		HAL_GPIO_WritePin(PLASMA_SIGNAL_GPIO_Port,PLASMA_SIGNAL_Pin ,GPIO_PIN_RESET)

#define FAN_IO_RUN_SetHigh()	HAL_GPIO_WritePin(MOTOR_CONTROL_RUN_GPIO_Port,MOTOR_CONTROL_RUN_Pin ,GPIO_PIN_SET)
#define FAN_IO_RUN_SetLow()		HAL_GPIO_WritePin(MOTOR_CONTROL_RUN_GPIO_Port,MOTOR_CONTROL_RUN_Pin ,GPIO_PIN_RESET)

#define FAN_IO_SetHigh()       HAL_GPIO_WritePin(MOTOR_CONTROL_1_GPIO_Port,MOTOR_CONTROL_1_Pin ,GPIO_PIN_SET)
#define FAN_IO_SetLow()		   HAL_GPIO_WritePin(MOTOR_CONTROL_1_GPIO_Port,MOTOR_CONTROL_1_Pin ,GPIO_PIN_RESET)


typedef enum{
  
  power_off = 0x01,
  power_on,
  run_update_data


}power_state;



typedef struct {

    uint8_t rs485_Command_label;
	uint8_t gPower_On;

   //adc 
	uint8_t ADC_channel_No;
	uint16_t ptc_temp_voltage;
	uint16_t fan_detect_voltage;

	uint8_t  gPlasma;
    uint8_t  gPtc;
    uint8_t  gUltrasonic;

	uint8_t  gTemperature;
    uint8_t  gHumidity;

	//fault 
	uint8_t fan_warning ;
	uint8_t ptc_warning;

	

	//void (*fan_continue_run)(void); 	/* fan ren ten minutes and stop */
	
    uint8_t gTimer_iwdg_feed_times;
	uint8_t gTimer_sensor_detect_times;
	uint8_t gTimer_fan_counter;
	uint8_t gTimer_ptc_adc_times ;
	uint8_t gTimer_fan_adc_times ;
	uint8_t gTimer_fan_works_times ;
	uint16_t gTimer_continuce_works_time ;
	


}MAINBOARD_T;

extern MAINBOARD_T g_tMain;


extern void (* fan_continue_run)(void);

void bsp_Init(void);


void Mainboard_Run_Process_Handler(void);


void Fan_Continue_RunTenMinutes(void (*fan_run)(void));






#endif 
