#include "interrupt_manager.h"
#include "bsp.h"



/**
  * Function Name: void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
  * Function: Tim3 interrupt call back function
  * Tim3 timer :timing time 10ms
  * 
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{

    static uint8_t tm0;
    if(htim->Instance==TIM17){
		
	   tm0 ++ ;
	 if(tm0 > 99){//100 *10ms = 1000ms =1s
        tm0 =0;
		g_tMain.gTimer_sensor_detect_times++;
		g_tMain.gTimer_iwdg_feed_times++;
		g_tMain.gTimer_fan_counter++;
		g_tMain.gTimer_continuce_works_time++;
		g_tMain.gTimer_ptc_adc_times++ ;
	    g_tMain.gTimer_fan_adc_times++ ;
		g_tMain.gTimer_continuce_works_time++ ;
		g_tMain.gTimer_fan_works_times ++;
		g_tMain.gTimer_compare_temp++;
	
		
	    
 	}
 }
}
