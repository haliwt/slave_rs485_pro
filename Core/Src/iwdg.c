/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    iwdg.c
  * @brief   This file provides code for the configuration
  *          of the IWDG instances.
  ******************************************************************************
  * @attention
  *
  * Tout =(4*2^prer *rlr)/32 (ms) //IWDG ->LSI ->32KHz
  *      =((4*2^0)*4095)/32
  *		 = 511.8ms.
  *
  * Tout = (64 * 4095) /32 = 8s.
  *
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "iwdg.h"

/* USER CODE BEGIN 0 */
#include "bsp.h"
/* USER CODE END 0 */

IWDG_HandleTypeDef hiwdg;

/* IWDG init function */
void MX_IWDG_Init(void)
{

  /* USER CODE BEGIN IWDG_Init 0 */

  /* USER CODE END IWDG_Init 0 */

  /* USER CODE BEGIN IWDG_Init 1 */

  /* USER CODE END IWDG_Init 1 */
  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_64;
  hiwdg.Init.Window = 4095;
  hiwdg.Init.Reload = 4095;
  if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN IWDG_Init 2 */

  /* USER CODE END IWDG_Init 2 */

}

/* USER CODE BEGIN 1 */
/**
 * @brief       Î¹¶ÀÁ¢¿´ÃÅ¹·
 * @param       ÎÞ
 * @retval      ÎÞ
 */
void iwdg_feed(void)
{

	if(g_tMain.gTimer_iwdg_feed_times >5){
	      g_tMain.gTimer_iwdg_feed_times=0;
	      HAL_IWDG_Refresh(&hiwdg);
	}
}


/* USER CODE END 1 */
