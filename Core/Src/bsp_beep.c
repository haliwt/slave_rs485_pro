#include "bsp_beep.h"
#include "bsp.h"


void Buzzer_KeySound(void)
{
  HAL_TIM_PWM_Start(&htim14,TIM_CHANNEL_1);
  HAL_Delay(100);
  Buzzer_KeySound_Off();
  
}

void Buzzer_KeySound_Off(void)
{
  HAL_TIM_PWM_Stop(&htim14,TIM_CHANNEL_1);
	
}

void Buzzer_Fan_Error_Sound(void)
{
   
   Buzzer_KeySound();
   HAL_Delay(50);
   Buzzer_KeySound_Off();//Buzzer_KeySound();
   HAL_Delay(50);
   Buzzer_KeySound();
   HAL_Delay(50);
   Buzzer_KeySound_Off();//Buzzer_KeySound();
   HAL_Delay(50);
   Buzzer_KeySound();
   HAL_Delay(50);
   Buzzer_KeySound_Off();//Buzzer_KeySound();



}

void Buzzer_Ptc_Error_Sound(void)
{

	 
	  Buzzer_KeySound();
	  HAL_Delay(50);
	  Buzzer_KeySound_Off();
	  HAL_Delay(50);
	  Buzzer_KeySound();
	  HAL_Delay(50);
	  Buzzer_KeySound_Off();


}

