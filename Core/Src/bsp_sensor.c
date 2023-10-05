#include "bsp_sensor.h"
#include "bsp.h"





#define Bit_RESET 0
#define Bit_SET   1


static void DHT11_Mode_IPU(void);
static void DHT11_Mode_Out_PP(void);
static uint8_t DHT11_ReadByte(void);
static void  Dht11_Read_TempHumidity_Value(DHT11_Data_TypeDef * pdth11);

DHT11_Data_TypeDef DHT11;



//�ȴ�us����
//void delay_us(unsigned long i)
//{
//	unsigned long j;
//	for(;i>0;i--)
//	{
//			for(j=6;j>0;j--);
//	}
//}


/**
  * ��������: DHT11 ��ʼ������
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ������
  */
void DHT11_Init ( void )
{

	__HAL_RCC_GPIOA_CLK_ENABLE();
	DHT11_Mode_Out_PP();
	
	DHT11_Dout_HIGH();  // ����GPIO
}

/**
  * ��������: ʹDHT11-DATA���ű�Ϊ��������ģʽ
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ������
  */
static void DHT11_Mode_IPU(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = DHT11_Dout_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(DHT11_Dout_PORT, &GPIO_InitStruct);
}

/**
  * ��������: ʹDHT11-DATA���ű�Ϊ�������ģʽ
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ������
  */
static void DHT11_Mode_Out_PP(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = DHT11_Dout_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(DHT11_Dout_PORT, &GPIO_InitStruct);
}

/**
  * ��������: ��DHT11��ȡһ���ֽڣ�MSB����
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ������
  */
static uint8_t DHT11_ReadByte ( void )
{
	uint8_t i, temp=0;
	
	for(i=0;i<8;i++)    
	{	 
		/*ÿbit��50us�͵�ƽ���ÿ�ʼ����ѯֱ���ӻ����� ��50us �͵�ƽ ����*/  
		while(DHT11_Data_IN()==Bit_RESET);

		/*DHT11 ��26~28us�ĸߵ�ƽ��ʾ��0������70us�ߵ�ƽ��ʾ��1����
		 *ͨ����� x us��ĵ�ƽ��������������״ ��x ���������ʱ 
		 */
		delay_us(40); //��ʱx us �����ʱ��Ҫ��������0������ʱ�伴��	   	  

		if(DHT11_Data_IN()==Bit_SET)/* x us����Ϊ�ߵ�ƽ��ʾ���ݡ�1�� */
		{
			/* �ȴ�����1�ĸߵ�ƽ���� */
			while(DHT11_Data_IN()==Bit_SET);

			temp|=(uint8_t)(0x01<<(7-i));  //�ѵ�7-iλ��1��MSB���� 
		}
		else	 // x us��Ϊ�͵�ƽ��ʾ���ݡ�0��
		{			   
			temp&=(uint8_t)~(0x01<<(7-i)); //�ѵ�7-iλ��0��MSB����
		}
	}
	return temp;
}

/**
  * ��������: һ�����������ݴ���Ϊ40bit����λ�ȳ�
  * �������: DHT11_Data:DHT11��������
  * �� �� ֵ: ERROR��  ��ȡ����
  *           SUCCESS����ȡ�ɹ�
  * ˵    ����8bit ʪ������ + 8bit ʪ��С�� + 8bit �¶����� + 8bit �¶�С�� + 8bit У��� 
  */
uint8_t DHT11_Read_TempAndHumidity(DHT11_Data_TypeDef *DHT11_Data)
{  
  uint8_t temp;
  uint16_t humi_temp;
  
	/*���ģʽ*/
	DHT11_Mode_Out_PP();
	/*��������*/
	DHT11_Dout_LOW();
	/*��ʱ18ms*/
	HAL_Delay(20);

	/*�������� ������ʱ30us*/
	DHT11_Dout_HIGH(); 

	delay_us(30);   //��ʱ30us

	/*������Ϊ���� �жϴӻ���Ӧ�ź�*/ 
	DHT11_Mode_IPU();
  delay_us(40);   //��ʱ30us
	/*�жϴӻ��Ƿ��е͵�ƽ��Ӧ�ź� �粻��Ӧ����������Ӧ����������*/   
	if(DHT11_Data_IN()==Bit_RESET)     
	{
    /*��ѯֱ���ӻ����� ��80us �͵�ƽ ��Ӧ�źŽ���*/  
    while(DHT11_Data_IN()==Bit_RESET);

    /*��ѯֱ���ӻ������� 80us �ߵ�ƽ �����źŽ���*/
    while(DHT11_Data_IN()==Bit_SET);

    /*��ʼ��������*/   
    DHT11_Data->humi_high8bit= DHT11_ReadByte();
    DHT11_Data->humi_low8bit = DHT11_ReadByte();
    DHT11_Data->temp_high8bit= DHT11_ReadByte();
    DHT11_Data->temp_low8bit = DHT11_ReadByte();
    DHT11_Data->check_sum    = DHT11_ReadByte();

    /*��ȡ���������Ÿ�Ϊ���ģʽ*/
    DHT11_Mode_Out_PP();
    /*��������*/
    DHT11_Dout_HIGH();
    
    /* �����ݽ��д��� */
    humi_temp=DHT11_Data->humi_high8bit*100+DHT11_Data->humi_low8bit;
    DHT11_Data->humidity =(float)humi_temp/100;
    
    humi_temp=DHT11_Data->temp_high8bit*100+DHT11_Data->temp_low8bit;
    DHT11_Data->temperature=(float)humi_temp/100;    
    
    /*����ȡ�������Ƿ���ȷ*/
    temp = DHT11_Data->humi_high8bit + DHT11_Data->humi_low8bit +
           DHT11_Data->temp_high8bit+ DHT11_Data->temp_low8bit;
    if(DHT11_Data->check_sum==temp)
    { 
      return SUCCESS;
    }
    else 
      return ERROR;
	}	
	else
		return ERROR;
}


static void  Dht11_Read_TempHumidity_Value(DHT11_Data_TypeDef * pdth11)
{
	if(DHT11_Read_TempAndHumidity(pdth11) == 0){
		   
		 g_tMain.gHumidity   = (pdth11->humi_high8bit);
		   
		 g_tMain.gTemperature = (pdth11->temp_high8bit);
	   
	 }

}

void Update_DHT11_Value_Handler(void)
{
    
  
   Dht11_Read_TempHumidity_Value(&DHT11);
	

//	sendData_Real_TimeHum(run_t.gDht11_humidity ,run_t.gDht11_temperature);
	
    
}


















