#include "bsp.h"
#include "modbus_slave.h"

MAINBOARD_T g_tMain;

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

static void MODS_Read_Slave_Address_Info(void); //MODS -> Modbus - slave machine ,MODH--> host machine

//static uint8_t g_mods_timeout = 0;
MODS_T g_tModS = {0};
VAR_T g_tVar;

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
	uint16_t addr;
	uint16_t crc1;

#if 0
	
	/* ����3.5���ַ�ʱ���ִ��MODH_RxTimeOut()������ȫ�ֱ��� g_rtu_timeout = 1; ֪ͨ������ʼ���� */
//	if (g_mods_timeout == 0)	
//	{
//		return;								/* û�г�ʱ���������ա���Ҫ���� g_tModS.RxCount */
//	}
//	
//	g_mods_timeout = 0;	 					/* ���־ */

	if (g_tModS.RxCount < 7)				/* ���յ�������С��4���ֽھ���Ϊ���󣬵�ַ��8bit��+ָ�8bit��+�����Ĵ�����16bit�� */
	{
		goto err_ret;
	}

	/* ����CRCУ��ͣ������ǽ����յ������ݰ���CRC16ֵһ����CRC16�������0����ʾ��ȷ���� */
	crc1 = CRC16_Modbus(g_tModS.RxBuf, g_tModS.RxCount);
	if (crc1 != 0)
	{
		goto err_ret;
	}

	/* վ��ַ (1�ֽڣ� */
	addr = g_tModS.RxBuf[0];				/* ��1�ֽ� վ�� */
	if (addr != MASTER_ADDRESS && addr !=0)		 			/* �ж��������͵������ַ�Ƿ���� */
	{
		goto err_ret;
	}

	/* ����Ӧ�ò�Э�� */
	MODS_AnalyzeApp();						
	
err_ret:
	g_tModS.RxCount = 0;					/* ��������������������´�֡ͬ�� */
    #endif 
  if(g_tModS.Rx_rs485_data_flag ==  rs485_receive_data_success){
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
			g_tModS.Rx_rs485_data_flag=0;
		    g_tModS.RxCount =0;
		    

		}
    }
	if(crc16_check_flag==1){
		/* 分析应用层协议 */
		MODS_AnalyzeApp();
		crc16_check_flag=0;
	 
	
   	}
	
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

/*
*********************************************************************************************************
*	�� �� ��: MODS_RxTimeOut
*	����˵��: ����3.5���ַ�ʱ���ִ�б������� ����ȫ�ֱ��� g_mods_timeout = 1��֪ͨ������ʼ���롣
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
//static void MODS_RxTimeOut(void)
//{
//	g_mods_timeout = 1;
//}

/*
*********************************************************************************************************
*	�� �� ��: MODS_SendWithCRC
*	����˵��: ����һ������, �Զ�׷��2�ֽ�CRC
*	��    ��: _pBuf ���ݣ�
*			  _ucLen ���ݳ��ȣ�����CRC��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
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
/*
*********************************************************************************************************
*	�� �� ��: MODS_SendAckErr
*	����˵��: ���ʹ���Ӧ��
*	��    ��: _ucErrCode : �������
*	�� �� ֵ: ��
*********************************************************************************************************
*/
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
/*
*********************************************************************************************************
*	�� �� ��: MODS_AnalyzeApp
*	����˵��: ����Ӧ�ò�Э��
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
#if 0
static void MODS_AnalyzeApp(void)
{
	switch (g_tModS.RxBuf[1])				/* ��2���ֽ� ������ */
	{
		case 0x01:							/* ��ȡ��Ȧ״̬����������led���棩*/
			MODS_01H();
			bsp_PutMsg(MSG_MODS_01H, 0);	/* ������Ϣ,�������� */
			break;

		case 0x02:							/* ��ȡ����״̬������״̬��*/
			MODS_02H();
			//bsp_PutMsg(MSG_MODS_02H, 0);
			break;
		
		case 0x03:							/* ��ȡ���ּĴ����������̴���g_tVar�У�*/
			MODS_03H();
			//bsp_PutMsg(MSG_MODS_03H, 0);
			break;
		
		case 0x04:							/* ��ȡ����Ĵ�����ADC��ֵ��*/
			MODS_04H();
			//bsp_PutMsg(MSG_MODS_04H, 0);
			break;
		
		case 0x05:							/* ǿ�Ƶ���Ȧ������led��*/
			MODS_05H();
			//bsp_PutMsg(MSG_MODS_05H, 0);
			break;
		
		case 0x06:							/* д��������Ĵ����������̸�дg_tVar�еĲ�����*/
			MODS_06H();	
			//bsp_PutMsg(MSG_MODS_06H, 0);
			break;
			
		case 0x10:							/* д�������Ĵ����������̴���g_tVar�еĲ�����*/
			MODS_10H();
			//bsp_PutMsg(MSG_MODS_10H, 0);
			break;
		
		default:
			//g_tModS.RspCode = RSP_ERR_CMD;
			MODS_SendAckErr(g_tModS.RspCode);	/* ��������������� */
			break;
	}
}
#endif
static void MODS_AnalyzeApp(void)
{

  MODS_Read_Slave_Address_Info();


}
/*
*********************************************************************************************************
*	�� �� ��: MODS_01H
*	����˵��: ��ȡ��Ȧ״̬����ӦԶ�̿���D01/D02/D03��
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void MODS_Read_Slave_Address_Info(void)
{
    uint8_t bytes_zero,byte_load_addr,byte_fun_code,byte_len,byte_data;


	  
	   bytes_zero = g_tModS.RxBuf[0];	/* 0x00 广播模式 */
	   byte_load_addr = g_tModS.RxBuf[1]; /* 主机  地址   0x01*/
	   byte_fun_code = g_tModS.RxBuf[2];
	   byte_len = g_tModS.RxBuf[3];
	   byte_data = g_tModS.RxBuf[4];

	 if(bytes_zero == 0 ){

	   	Answerback_RS485_Signal(byte_load_addr,byte_fun_code,byte_len,byte_data);
	   
	   switch (byte_fun_code)
		{
			case mod_power: //0x0101
				
				switch(byte_data){

                   case 0:
                       
				      g_tMain.rs485_Command_label = power_off;
				       
				      
				   break;

				   case 1:
				      g_tMain.rs485_Command_label = power_on;
					 

				   break;

				}	
					
	        break;

			case mod_ptc:

			   if(g_tMain.gPower_On == power_on){
			  
			   switch(byte_data){

                   case 0:
                      g_tMain.gPtc = 0;
			         
				   break;

				   case 1:
				      
					 g_tMain.gPtc = 1;
					 
				   break;

				}	
               
			   }
			break;

			case mod_plasma:

				 if(g_tMain.gPower_On == power_on){
			   
			     switch(byte_data){

                   case 0:
                  g_tMain.gPlasma=0; 
				    
				   break;

				   case 1:
				      
				    g_tMain.gPlasma=1;
					

				   break;

				}	
              
				}

			break;

			case mod_ulrasonic:

			    if(g_tMain.gPower_On == power_on){
				
				 switch(byte_data){

                   case 0:
                       g_tMain.gUltrasonic = 0;
				     
				   break;

				   case 1:
				     g_tMain.gUltrasonic = 1;
					 
				   

				   break;

				}	
               

			   }

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
void Answerback_RS485_Signal(uint8_t addr,uint8_t fun_code,uint8_t len,uint8_t data)
{
	g_tModS.TxCount = 0;
	g_tModS.TxBuf[g_tModS.TxCount++] = 0x0;		/* 广播模式 */
	g_tModS.TxBuf[g_tModS.TxCount++] = MASTER_ADDRESS;  /* 应答地址 */
	g_tModS.TxBuf[g_tModS.TxCount++] = fun_code;		/* 功能砄1�7 等离子开或��关闄1�7 */	
	g_tModS.TxBuf[g_tModS.TxCount++] = len;	/* 数据长度*/
	g_tModS.TxBuf[g_tModS.TxCount++] = data;		/* 数据 */
	
	//MODS_SendAckWithCRC();		/* 发��数据，自动加CRC */
	MODS_SendWithCRC(g_tModS.TxBuf, g_tModS.TxCount);
	
	//g_tModH.RegNum = _num;		/* 寄存器个敄1�7 */
	//g_tModH.Reg02H = _reg;		/* 保存02H指令中的寄存器地坢�，方便对应答数据进行分类 */	



}


