#include "bsp_beep.h"
#include "bsp.h"


void Buzzer_KeySound(void)
{
  HAL_TIM_PWM_Start(&htim14,TIM_CHANNEL_1);
  
}

void Buzzer_KeySound_Off(void)
{
  HAL_TIM_PWM_Stop(&htim14,TIM_CHANNEL_1);
	
}



