/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    adc.c
  * @brief   This file provides code for the configuration
  *          of the ADC instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "adc.h"

/* USER CODE BEGIN 0 */
#include "bsp.h"
/* USER CODE END 0 */

ADC_HandleTypeDef hadc1;

/* ADC1 init function */
void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.LowPowerAutoPowerOff = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc1.Init.SamplingTimeCommon1 = ADC_SAMPLETIME_1CYCLE_5;
  hadc1.Init.SamplingTimeCommon2 = ADC_SAMPLETIME_1CYCLE_5;
  hadc1.Init.OversamplingMode = DISABLE;
  hadc1.Init.TriggerFrequencyMode = ADC_TRIGGER_FREQ_HIGH;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLINGTIME_COMMON_1;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

void HAL_ADC_MspInit(ADC_HandleTypeDef* adcHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(adcHandle->Instance==ADC1)
  {
  /* USER CODE BEGIN ADC1_MspInit 0 */

  /* USER CODE END ADC1_MspInit 0 */
    /* ADC1 clock enable */
    __HAL_RCC_ADC_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**ADC1 GPIO Configuration
    PA0     ------> ADC1_IN0
    PA1     ------> ADC1_IN1
    */
    GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN ADC1_MspInit 1 */

  /* USER CODE END ADC1_MspInit 1 */
  }
}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef* adcHandle)
{

  if(adcHandle->Instance==ADC1)
  {
  /* USER CODE BEGIN ADC1_MspDeInit 0 */

  /* USER CODE END ADC1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_ADC_CLK_DISABLE();

    /**ADC1 GPIO Configuration
    PA0     ------> ADC1_IN0
    PA1     ------> ADC1_IN1
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_0|GPIO_PIN_1);

  /* USER CODE BEGIN ADC1_MspDeInit 1 */

  /* USER CODE END ADC1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
/*****************************************************************
*
	*Function Name: static uint16_t Get_Adc(uint32_t ch)  
	*Function ADC input channel be selected "which one channe"
	*Input Ref: which one ? AC_Channel_?
	*Return Ref: No
	*
	*
*****************************************************************/
static uint16_t Get_Adc_Channel(uint32_t ch)   
{
    ADC_ChannelConfTypeDef ADC1_ChanConf;

	ADC1_ChanConf.Channel=ch;                                   //Í¨µÀ
    ADC1_ChanConf.Rank= ADC_REGULAR_RANK_1;                                    //第一个序刄1�7
    ADC1_ChanConf.SamplingTime=ADC_SAMPLETIME_1CYCLE_5;//ADC_SAMPLETIME_239CYCLES_5;      //²ÉÑùÊ±¼ä               


	HAL_ADC_ConfigChannel(&hadc1,&ADC1_ChanConf);        //Í¨µÀÅäÖÃ
	
    HAL_ADC_Start(&hadc1);                               //start ADC transmit
	
    HAL_ADC_PollForConversion(&hadc1,10);                //轮询转换
 
	return (uint16_t)HAL_ADC_GetValue(&hadc1);	        	//·µ»Ø×î½üÒ»´ÎADC1¹æÔò×éµÄ×ª»»½á¹û
}
/*****************************************************************
*
	*Function Name: static uint16_t Get_Adc(uint32_t ch)  
	*Function ADC input channel be selected "which one channe"
	*Input Ref: which one ? AC_Channel_?
	*Return Ref: No
	*
	*
*****************************************************************/
//static uint16_t Get_Adc_Average(uint8_t times)
//{
//	
//	HAL_ADC_Start(&hadc1);								
//    
//    temp_value= ADC1->DR;
//    return temp_value;
//}

static uint16_t Get_Adc_Average(uint32_t ch,uint8_t times)
{
	uint32_t temp_val=0;
	uint8_t t;
	for(t=0;t<times;t++)
	{
		temp_val+=Get_Adc_Channel(ch);
		delay_ms(1);
	}
	return temp_val/times;
} 



void Get_PTC_Temperature_Voltage(uint32_t channel,uint8_t times)
{
    uint16_t adcx;
	static uint8_t open_ptc_detected_flag ;
	adcx = Get_Adc_Average(channel,times);

    g_tMain.ptc_temp_voltage  =(uint16_t)((adcx * 3300)/4096); //amplification 100 ,3.11V -> 311

	if(open_ptc_detected_flag == 0){ //power on the voltage is small 
         open_ptc_detected_flag++ ;
         g_tMain.ptc_temp_voltage = 500;
    }
	#ifdef DEBUG
      printf("ptc= %d",  g_tModS.ptc_temp_voltage);
	#endif 
}


/*****************************************************************
	*
	*Function Name: void Judge_PTC_Temperature_Value(void)
	*Function: PTC adc read voltage
	*Input Ref: NO
	*Return Ref: No
	*
	*
*****************************************************************/
void Judge_PTC_Temperature_Value(void)
{
  
   
  if(g_tMain.ptc_temp_voltage < 373 || g_tMain.ptc_temp_voltage ==373){ //90 degree
	    g_tMain.gPtc =0 ;
	    PTC_IO_SetLow(); //turn off
		g_tMain.ptc_warning =1;
    
        Buzzer_Ptc_Error_Sound();
	   
   }
   
}

/*****************************************************************
	*
	*Function Name: void Get_Fan_Adc_Fun(uint8_t channel,uint8_t times)
	*Function ADC input channel be selected "which one channe"
	*Input Ref: which one ? AC_Channel_?, hexadecimal of average
	*Return Ref: No
	*
	*
*****************************************************************/
void Get_Fan_Adc_Fun(uint32_t channel,uint8_t times)
{

    static uint8_t detect_error_times;
	uint16_t adc_fan_hex;
	
	
	adc_fan_hex = Get_Adc_Average(channel,times);

    g_tMain.fan_detect_voltage  =(uint16_t)((adc_fan_hex * 3300)/4096); //amplification 1000 ,3.111V -> 3111
	HAL_Delay(5);

	if(g_tMain.fan_detect_voltage >300 &&  g_tMain.fan_detect_voltage < 2000){
           detect_error_times=0;
		   #ifdef DEBUG
             printf("adc= %d",run_t.fan_detect_voltage);
		   #endif 
           g_tMain.fan_warning = 0;
    }
	else{

	          
			   if(detect_error_times >0){
			   		detect_error_times=0;
		           g_tMain.fan_warning = 1;
				
			  
			   }
	           detect_error_times++;

     }
}

/* USER CODE END 1 */
