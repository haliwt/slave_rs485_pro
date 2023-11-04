#include "bsp.h"
#include "modbus_slave.h"

MAINBOARD_T g_tMain;
Protocol_t  g_tPro;


static void MODS_Read_Host_Analysis_Info(void);


/*
*********************************************************************************************************
*	                                   ��������
*********************************************************************************************************
*/
static void MODS_SendWithCRC(uint8_t *_pBuf, uint8_t _ucLen);
//static void MODS_SendAckOk(void);
//static void MODS_SendAckErr(uint8_t _ucErrCode);

static void MODS_AnalyzeApp(void);

//static void MODS_RxTimeOut(void);

//static void MODS_01H(void);
//static void MODS_02H(void);
//static void MODS_03H(void);
//static void MODS_04H(void);
//static void MODS_05H(void);
//static void MODS_06H(void);
//static void MODS_10H(void);

//static uint8_t MODS_ReadRegValue(uint16_t reg_addr, uint8_t *reg_value);
//static uint8_t MODS_WriteRegValue(uint16_t reg_addr, uint16_t reg_value);

void MODS_ReciveNew(uint8_t _byte);
uint8_t crc16_check_flag;


/*
*********************************************************************************************************
*	                                   ����
*********************************************************************************************************
*/
/*
Baud rate	Bit rate	 Bit time	 Character time	  3.5 character times
  2400	    2400 bits/s	  417 us	      4.6 ms	      16 ms
  4800	    4800 bits/s	  208 us	      2.3 ms	      8.0 ms
  9600	    9600 bits/s	  104 us	      1.2 ms	      4.0 ms
 19200	   19200 bits/s    52 us	      573 us	      2.0 ms
 38400	   38400 bits/s	   26 us	      286 us	      1.75 ms(1.0 ms)
 115200	   115200 bit/s	  8.7 us	       95 us	      1.75 ms(0.33 ms) ����̶���Ϊ1750us
*/
typedef struct
{
	uint32_t Bps;
	uint32_t usTimeOut;
}MODBUSBPS_T;

const MODBUSBPS_T ModbusBaudRate[] =
{	
    {2400,	16000}, /* ������2400bps, 3.5�ַ��ӳ�ʱ��16000us */
	{4800,	 8000}, 
	{9600,	 4000},
	{19200,	 2000},
	{38400,	 1750},
	{115200, 1750},
	{128000, 1750},
	{230400, 1750},
};



//static uint8_t g_mods_timeout = 0;
MODS_T g_tModS ;


/*
*********************************************************************************************************
*	�� �� ��: MODS_Poll
*	����˵��: �������ݰ�. �����������������á�
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void MODS_Poll(void)
{

	uint16_t crc1;

  if(g_tModS.Rx_rs485_data_flag ==  rs485_receive_data_success){

       g_tPro.pro_addr = BEBufToUint16(g_tModS.RxBuf); //大端数据发送 ,1,2
	   g_tPro.pro_local_addr = BEBufToUint16(g_tModS.RxBuf); /* 大端数据，3，4 */
  	    
		/* 计算CRC校验和，这里是将接收到的数据包含CRC16值一起做CRC16，结果是0，表示正确接收 */
		crc1 = CRC16_Modbus(g_tModS.RxBuf,g_tModS.RxCount);
		if (crc1 != 0)
		{
			g_tModS.RxCount = 0;	/* 必须清零计数器，方便下次帧同步 */
			g_tModS.Rx_rs485_data_flag=0;
			//if(g_tModH.RxBuf[0]==0)run_t.broadcast_response_signal = FAIL_BROADCAST;
			Answerback_RS485_Signal(g_tModS.RxBuf[1],0xff,g_tModS.RxBuf[3],0x01);
		}
		else{
        	crc16_check_flag = 1;
			
		    }
    }
	if(crc16_check_flag==1){
		/* 分析应用层协议 */
		MODS_AnalyzeApp();
		crc16_check_flag=0;
	    g_tModS.Rx_rs485_data_flag=0;
		g_tModS.RxCount =0;
	 
	
   	}
	
}
static void MODS_AnalyzeApp(void)
{

  MODS_Read_Host_Analysis_Info();


}

/*
*********************************************************************************************************
*	�� �� ��: MODS_ReciveNew
*	����˵��: ���ڽ����жϷ���������ñ����������յ�һ���ֽ�ʱ��ִ��һ�α�������
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void MODS_ReciveNew(uint8_t _byte)
{
#if 0
	/*
		3.5���ַ���ʱ������ֻ������RTUģʽ���棬��ΪRTUģʽû�п�ʼ���ͽ�������
		�������ݰ�֮��ֻ�ܿ�ʱ���������֣�Modbus�����ڲ�ͬ�Ĳ������£����ʱ���ǲ�һ���ģ�
		���鿴��C�ļ���ͷ
	*/
	uint8_t i;
	
	/* ���ݲ����ʣ���ȡ��Ҫ�ӳٵ�ʱ�� */
	for(i = 0; i < (sizeof(ModbusBaudRate)/sizeof(ModbusBaudRate[0])); i++)
	{
		if(SBAUD485 == ModbusBaudRate[i].Bps)
		{
			break;
		}	
	}

	g_mods_timeout = 0;
	
	/* Ӳ����ʱ�жϣ���ʱ����us Ӳ����ʱ��1����MODBUS�ӻ�, ��ʱ��2����MODBUS����*/
	bsp_StartHardTimer(1, ModbusBaudRate[i].usTimeOut, (void *)MODS_RxTimeOut);
#endif 
	if (g_tModS.RxCount < S_RX_BUF_SIZE)
	{
		g_tModS.RxBuf[g_tModS.RxCount++] = _byte;
	}
}


/*******************************************************************************************************
	*
	*	�� �� ��: MODS_SendWithCRC
	*	����˵��: ����һ������, �Զ�׷��2�ֽ�CRC
	*	��    ��: _pBuf ���ݣ�
	*			  _ucLen ���ݳ��ȣ�����CRC��
	*	�� �� ֵ: ��
	*
*********************************************************************************************************/
static void MODS_SendWithCRC(uint8_t *_pBuf, uint8_t _ucLen)
{
	uint16_t crc;
	uint8_t buf[S_TX_BUF_SIZE];

	memcpy(buf, _pBuf, _ucLen);
	crc = CRC16_Modbus(_pBuf, _ucLen);
	buf[_ucLen++] = crc >> 8;
	buf[_ucLen++] = crc;

	RS485_SendBuf(buf, _ucLen);
}
#if 0
/********************************************************************************************************
*	�� �� ��: MODS_SendAckErr
*	����˵��: ���ʹ���Ӧ��
*	��    ��: _ucErrCode : �������
*	�� �� ֵ: ��
**********************************************************************************************************/
static void MODS_SendAckErr(uint8_t _ucErrCode)
{
	uint8_t txbuf[3];

	txbuf[0] = g_tModS.RxBuf[0];					/* 485��ַ */
	txbuf[1] = g_tModS.RxBuf[1] | 0x80;				/* �쳣�Ĺ����� */
	txbuf[2] = _ucErrCode;							/* �������(01,02,03,04) */

	MODS_SendWithCRC(txbuf, 3);
}

/*
*********************************************************************************************************
*	�� �� ��: MODS_SendAckOk
*	����˵��: ������ȷ��Ӧ��.
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void MODS_SendAckOk(void)
{
	uint8_t txbuf[6];
	uint8_t i;

	for (i = 0; i < 6; i++)
	{
		txbuf[i] = g_tModS.RxBuf[i];
	}
	MODS_SendWithCRC(txbuf, 6);
}
#endif 

/*******************************************************************************************************
*
*	Function Name: MODS_01H
*	Function: ��ȡ��Ȧ״̬����ӦԶ�̿���D01/D02/D03��
*	Input Ref: ��
*	Return Ref: ��
*
********************************************************************************************************/
static void MODS_Read_Host_Analysis_Info(void)
{
   
	   
	   g_tPro.pro_fun_code = g_tModS.RxBuf[5]; /* 主机  地址   0x01*/
	   g_tPro.pro_data_len = g_tModS.RxBuf[6];
	   g_tPro.pro_data = g_tModS.RxBuf[7];


   //RS485 ANSWERING SIGNAL Grama Analysis
  if(g_tPro.pro_addr ==0x5555){//broadcast mode
   
          g_tModS.answering_signal_flag = rs485_broadcast_mode;
		  g_tPro.pro_addr = 0;
   
		  // Answerback_RS485_Signal(byte_load_addr,byte_fun_code,byte_len,byte_data);
  }
  else if(g_tPro.pro_addr == cpuId.slave_address){

       g_tModS.rs485_send_answering_signal_flag = rs485_answering_signal_data;
	   Answerback_RS485_Signal(g_tPro.pro_local_addr,g_tPro.pro_fun_code,g_tPro.pro_data_len,g_tPro.pro_data);

  }

  if(g_tModS.answering_signal_flag == rs485_broadcast_mode){
   
 
   switch(g_tPro.pro_fun_code){


			case mod_power: //0x01
				
				switch(g_tPro.pro_data){

                   case 0:
                       
				      g_tMain.rs485_Command_label = power_off;
				       
				      
				   break;

				   case 1:
				      g_tMain.rs485_Command_label = power_on;
					 

				   break;

				}	
				g_tModS.answering_signal_flag = 0xff;
	        break;

			case mod_ptc:

			   if(g_tMain.gPower_On == power_on){
			  
			   switch(g_tPro.pro_data){

                   case 0:
                      g_tMain.gPtc = 0;
			         
				   break;

				   case 1:
				      
					 g_tMain.gPtc = 1;
					 
				   break;

				}	
               
			   }
			   g_tModS.answering_signal_flag = 0xff;
			break;

			case mod_plasma:

				 if(g_tMain.gPower_On == power_on){
			   
			     switch(g_tPro.pro_data){

                   case 0:
                  g_tMain.gPlasma=0; 
				    
				   break;

				   case 1:
				      
				    g_tMain.gPlasma=1;
					

				   break;

				}	
              
				}
				g_tModS.answering_signal_flag = 0xff;
			break;

			case mod_ulrasonic:

			    if(g_tMain.gPower_On == power_on){
				
				 switch(g_tPro.pro_data){

                   case 0:
                       g_tMain.gUltrasonic = 0;
				     
				   break;

				   case 1:
				     g_tMain.gUltrasonic = 1;
					 
				   

				   break;

				}	
               

			   }
				g_tModS.answering_signal_flag = 0xff;
			break;

			case mod_set_temperature_value:
				
				 g_tMain.read_temperature_value[0] =  g_tPro.pro_data;
				 g_tMain.gTimer_compare_temp = 62;
			     g_tModS.answering_signal_flag = 0xff;

			break;

			
	    }

	  
   	}
}
/********************************************************************************
	**
	*Function Name:
	*Function :UART callback function  for UART interrupt for transmit data
	*Input Ref: structure UART_HandleTypeDef pointer
	*Return Ref:NO
	*
*******************************************************************************/
void Answerback_RS485_Signal(uint16_t addr,uint8_t fun_code,uint8_t len,uint8_t data)
{
	g_tModS.TxCount = 0;
	g_tModS.TxBuf[g_tModS.TxCount++] = PRO_HEAD;
    g_tModS.TxBuf[g_tModS.TxCount++] = 0x0;		/* host address */
	g_tModS.TxBuf[g_tModS.TxCount++] = MASTER_ADDRESS;  /* host address */
	g_tModS.TxBuf[g_tModS.TxCount++] = addr >> 8;  /* slave address */
	g_tModS.TxBuf[g_tModS.TxCount++] = addr ;  /* 应答地址 low byte of address */
	g_tModS.TxBuf[g_tModS.TxCount++] = fun_code;		/* 功能砄1�7 等离子开或��关闄1�7 */	
	g_tModS.TxBuf[g_tModS.TxCount++] = len;	/* 数据长度*/
	g_tModS.TxBuf[g_tModS.TxCount++] = data;		/* 数据 */
	
	//MODS_SendAckWithCRC();		/* 发��数据，自动加CRC */
	MODS_SendWithCRC(g_tModS.TxBuf, g_tModS.TxCount);
	
	//g_tModH.RegNum = _num;		/* 寄存器个敄1�7 */
	//g_tModH.Reg02H = _reg;		/* 保存02H指令中的寄存器地坢�，方便对应答数据进行分类 */	



}
/********************************************************************************
	**
	*Function Name:void MODS_SendError_Signal(uint8_t err)
	*Function: 
	*Input Ref: error of data 0x01 -crc16 error,0xA0- fan fault,0xB0- ptc too hot
	*Return Ref:NO
	*
*******************************************************************************/
void MODS_SendHostError_Signal(uint8_t err)
{
	g_tModS.TxCount = 0;
	g_tModS.TxBuf[g_tModS.TxCount++] = PRO_HEAD;
	g_tModS.TxBuf[g_tModS.TxCount++] = 0x00;
	g_tModS.TxBuf[g_tModS.TxCount++] = MASTER_ADDRESS;		/* 广播模式 */
	g_tModS.TxBuf[g_tModS.TxCount++] = cpuId.slave_address >> 8;  /* 应答地址 */
	g_tModS.TxBuf[g_tModS.TxCount++] = cpuId.slave_address ;  /* 应答地址 low byte of address */
	g_tModS.TxBuf[g_tModS.TxCount++] = 0xff;	/* function: error of code */	
	g_tModS.TxBuf[g_tModS.TxCount++] = 0x01;	/* 数据长度*/
	g_tModS.TxBuf[g_tModS.TxCount++] = err;		/* error数据 */
	
	//MODS_SendAckWithCRC();		/* 发��数据，自动加CRC */
	MODS_SendWithCRC(g_tModS.TxBuf, g_tModS.TxCount);
	
	g_tModS.rs485_send_answering_signal_flag = 0;		/* 寄存器个敄1�7 */
	//g_tModH.Reg02H = _reg;		/* 保存02H指令中的寄存器地坢�，方便对应答数据进行分类 */	

}


