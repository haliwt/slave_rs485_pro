#include "bsp.h"
#include "modbus_slave.h"

MAINBOARD_T g_tMain;
Protocol_t  g_tPro;


static void MODS_Read_Host_Analysis_Info(void);


/*
*********************************************************************************************************
*	                                   函数声明
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
*	                                   变量
*********************************************************************************************************
*/
/*
Baud rate	Bit rate	 Bit time	 Character time	  3.5 character times
  2400	    2400 bits/s	  417 us	      4.6 ms	      16 ms
  4800	    4800 bits/s	  208 us	      2.3 ms	      8.0 ms
  9600	    9600 bits/s	  104 us	      1.2 ms	      4.0 ms
 19200	   19200 bits/s    52 us	      573 us	      2.0 ms
 38400	   38400 bits/s	   26 us	      286 us	      1.75 ms(1.0 ms)
 115200	   115200 bit/s	  8.7 us	       95 us	      1.75 ms(0.33 ms) 后面固定都为1750us
*/
typedef struct
{
	uint32_t Bps;
	uint32_t usTimeOut;
}MODBUSBPS_T;

const MODBUSBPS_T ModbusBaudRate[] =
{	
    {2400,	16000}, /* 波特率2400bps, 3.5字符延迟时间16000us */
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
*	函 数 名: MODS_Poll
*	功能说明: 解析数据包. 在主程序中轮流调用。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void MODS_Poll(void)
{

	uint16_t crc1;

  if(g_tModS.Rx_rs485_data_flag ==  rs485_receive_data_success){

       g_tPro.pro_addr = BEBufToUint16(g_tModS.RxBuf); //澶х鏁版嵁鍙戦�� ,1,2
	   g_tPro.pro_local_addr = BEBufToUint16(g_tModS.RxBuf); /* 澶х鏁版嵁锛�3锛�4 */
  	    
		/* 璁＄畻CRC鏍￠獙鍜岋紝杩欓噷鏄皢鎺ユ敹鍒扮殑鏁版嵁鍖呭惈CRC16鍊间竴璧峰仛CRC16锛岀粨鏋滄槸0锛岃〃绀烘纭帴鏀� */
		crc1 = CRC16_Modbus(g_tModS.RxBuf,g_tModS.RxCount);
		if (crc1 != 0)
		{
			g_tModS.RxCount = 0;	/* 蹇呴』娓呴浂璁℃暟鍣紝鏂逛究涓嬫甯у悓姝� */
			g_tModS.Rx_rs485_data_flag=0;
			//if(g_tModH.RxBuf[0]==0)run_t.broadcast_response_signal = FAIL_BROADCAST;
			Answerback_RS485_Signal(g_tModS.RxBuf[1],0xff,g_tModS.RxBuf[3],0x01);
		}
		else{
        	crc16_check_flag = 1;
			
		    }
    }
	if(crc16_check_flag==1){
		/* 鍒嗘瀽搴旂敤灞傚崗璁� */
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
*	函 数 名: MODS_ReciveNew
*	功能说明: 串口接收中断服务程序会调用本函数。当收到一个字节时，执行一次本函数。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void MODS_ReciveNew(uint8_t _byte)
{
#if 0
	/*
		3.5个字符的时间间隔，只是用在RTU模式下面，因为RTU模式没有开始符和结束符，
		两个数据包之间只能靠时间间隔来区分，Modbus定义在不同的波特率下，间隔时间是不一样的，
		详情看此C文件开头
	*/
	uint8_t i;
	
	/* 根据波特率，获取需要延迟的时间 */
	for(i = 0; i < (sizeof(ModbusBaudRate)/sizeof(ModbusBaudRate[0])); i++)
	{
		if(SBAUD485 == ModbusBaudRate[i].Bps)
		{
			break;
		}	
	}

	g_mods_timeout = 0;
	
	/* 硬件定时中断，定时精度us 硬件定时器1用于MODBUS从机, 定时器2用于MODBUS主机*/
	bsp_StartHardTimer(1, ModbusBaudRate[i].usTimeOut, (void *)MODS_RxTimeOut);
#endif 
	if (g_tModS.RxCount < S_RX_BUF_SIZE)
	{
		g_tModS.RxBuf[g_tModS.RxCount++] = _byte;
	}
}


/*******************************************************************************************************
	*
	*	函 数 名: MODS_SendWithCRC
	*	功能说明: 发送一串数据, 自动追加2字节CRC
	*	形    参: _pBuf 数据；
	*			  _ucLen 数据长度（不带CRC）
	*	返 回 值: 无
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
*	函 数 名: MODS_SendAckErr
*	功能说明: 发送错误应答
*	形    参: _ucErrCode : 错误代码
*	返 回 值: 无
**********************************************************************************************************/
static void MODS_SendAckErr(uint8_t _ucErrCode)
{
	uint8_t txbuf[3];

	txbuf[0] = g_tModS.RxBuf[0];					/* 485地址 */
	txbuf[1] = g_tModS.RxBuf[1] | 0x80;				/* 异常的功能码 */
	txbuf[2] = _ucErrCode;							/* 错误代码(01,02,03,04) */

	MODS_SendWithCRC(txbuf, 3);
}

/*
*********************************************************************************************************
*	函 数 名: MODS_SendAckOk
*	功能说明: 发送正确的应答.
*	形    参: 无
*	返 回 值: 无
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
*	Function: 读取线圈状态（对应远程开关D01/D02/D03）
*	Input Ref: 无
*	Return Ref: 无
*
********************************************************************************************************/
static void MODS_Read_Host_Analysis_Info(void)
{
   
	   
	   g_tPro.pro_fun_code = g_tModS.RxBuf[5]; /* 涓绘満  鍦板潃   0x01*/
	   g_tPro.pro_data_len = g_tModS.RxBuf[6];
	   g_tPro.pro_data = g_tModS.RxBuf[7];


   //RS485 ANSWERING SIGNAL Grama Analysis
  if(g_tPro.pro_addr ==0x5555){//broadcast mode
   
          g_tModS.answering_signal_flag = rs485_broadcast_mode;
		  g_tPro.pro_addr = 0;
   
		  // Answerback_RS485_Signal(byte_load_addr,byte_fun_code,byte_len,byte_data);
  }
  else if(g_tPro.pro_addr == 0x001){ //host answering slave signal .

      g_tModS.rs485_send_answering_signal_flag = rs485_answering_signal_data;

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
	g_tModS.TxBuf[g_tModS.TxCount++] = addr ;  /* 搴旂瓟鍦板潃 low byte of address */
	g_tModS.TxBuf[g_tModS.TxCount++] = fun_code;		/* 鍔熻兘鐮� 绛夌瀛愬紑鎴栬€呭叧闂� */	
	g_tModS.TxBuf[g_tModS.TxCount++] = len;	/* 鏁版嵁闀垮害*/
	g_tModS.TxBuf[g_tModS.TxCount++] = data;		/* 鏁版嵁 */
	
	//MODS_SendAckWithCRC();		/* 鍙戦€佹暟鎹紝鑷姩鍔燙RC */
	MODS_SendWithCRC(g_tModS.TxBuf, g_tModS.TxCount);
	
	//g_tModH.RegNum = _num;		/* 瀵勫瓨鍣ㄤ釜鏁� */
	//g_tModH.Reg02H = _reg;		/* 淇濆瓨02H鎸囦护涓殑瀵勫瓨鍣ㄥ湴鍧€锛屾柟渚垮搴旂瓟鏁版嵁杩涜鍒嗙被 */	



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
	g_tModS.TxBuf[g_tModS.TxCount++] = MASTER_ADDRESS;		/* 骞挎挱妯″紡 */
	g_tModS.TxBuf[g_tModS.TxCount++] = cpuId.slave_address >> 8;  /* 搴旂瓟鍦板潃 */
	g_tModS.TxBuf[g_tModS.TxCount++] = cpuId.slave_address ;  /* 搴旂瓟鍦板潃 low byte of address */
	g_tModS.TxBuf[g_tModS.TxCount++] = 0xff;	/* function: error of code */	
	g_tModS.TxBuf[g_tModS.TxCount++] = 0x01;	/* 鏁版嵁闀垮害*/
	g_tModS.TxBuf[g_tModS.TxCount++] = err;		/* error鏁版嵁 */
	
	//MODS_SendAckWithCRC();		/* 鍙戦€佹暟鎹紝鑷姩鍔燙RC */
	MODS_SendWithCRC(g_tModS.TxBuf, g_tModS.TxCount);
	
	g_tModS.rs485_send_answering_signal_flag = 0;		/* 瀵勫瓨鍣ㄤ釜鏁� */
	//g_tModH.Reg02H = _reg;		/* 淇濆瓨02H鎸囦护涓殑瀵勫瓨鍣ㄥ湴鍧€锛屾柟渚垮搴旂瓟鏁版嵁杩涜鍒嗙被 */	

}


