#ifndef __BSP_SENSOR_H
#define __BSP_SENSOR_H
#include "main.h"

//IO��������
#define DHT11_DATA_IO_IN()      {GPIOB->MODER&=0XFFFFFFFC;GPIOB->MODER|=0<<0;}  //0x00 input mode
#define DHT11_DATA_IO_OUT()     {GPIOB->MODER&=0XFFFFFFFC;GPIOB->MODER|=1<<0;}   //0x01 output  mode 

#define DHT11_DATA      TEMPERATURE_SIGNAL_Pin
#define DHT11_GPIO      TEMPERATURE_SIGNAL_GPIO_Port

#define DHT11_DATA_SetHigh()            HAL_GPIO_WritePin(DHT11_GPIO,DHT11_DATA,GPIO_PIN_SET)    // output high level
#define DHT11_DATA_SetLow()             HAL_GPIO_WritePin(DHT11_GPIO,DHT11_DATA,GPIO_PIN_RESET)    // output low level

#define DHT11_ReadData()	            HAL_GPIO_ReadPin(DHT11_GPIO,DHT11_DATA)

/* �궨�� -------------------------------------------------------------------*/
/***********************   DHT11 �������Ŷ���  **************************/
#define DHT11_Dout_GPIO_CLK_ENABLE()              __HAL_RCC_GPIOA_CLK_ENABLE()//__HAL_RCC_GPIOA_CLK_ENABLE()
#define DHT11_Dout_PORT                           TEMPERATURE_SIGNAL_GPIO_Port
#define DHT11_Dout_PIN                            TEMPERATURE_SIGNAL_Pin

/***********************   DHT11 �����궨��  ****************************/
#define DHT11_Dout_LOW()                          HAL_GPIO_WritePin(DHT11_Dout_PORT, DHT11_Dout_PIN, GPIO_PIN_RESET)
#define DHT11_Dout_HIGH()                         HAL_GPIO_WritePin(DHT11_Dout_PORT, DHT11_Dout_PIN, GPIO_PIN_SET)
#define DHT11_Data_IN()	                          HAL_GPIO_ReadPin(DHT11_Dout_PORT,DHT11_Dout_PIN)



/* ���Ͷ��� ------------------------------------------------------------------*/
/************************ DHT11 �������Ͷ���******************************/
typedef struct
{
	uint8_t  humi_high8bit;		//ԭʼ���ݣ�ʪ�ȸ�8λ
	uint8_t  humi_low8bit;	 	//ԭʼ���ݣ�ʪ�ȵ�8λ
	uint8_t  temp_high8bit;	 	//ԭʼ���ݣ��¶ȸ�8λ
	uint8_t  temp_low8bit;	 	//ԭʼ���ݣ��¶ȸ�8λ
	uint8_t  check_sum;	 	    //У���
  float    humidity;            //ʵ��ʪ��
  float    temperature;        //ʵ���¶�  
} DHT11_Data_TypeDef;
extern DHT11_Data_TypeDef DHT11;


/* ��չ���� ------------------------------------------------------------------*/
/* �������� ------------------------------------------------------------------*/
//void DHT11_Init( void );
//uint8_t DHT11_Read_TempAndHumidity(DHT11_Data_TypeDef * DHT11_Data);
void Update_DHT11_Value(void);
void Update_Dht11_Totencent_Value(void);

void  Dht11_Read_TempHumidity_Handler(DHT11_Data_TypeDef * pdth11);




#endif