#include "bsp.h"
#include "modbus_slave.h"

MAINBOARD_T g_tMain;

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

static void MODS_Read_Slave_Address_Info(void); //MODS -> Modbus - slave machine ,MODH--> host machine

//static uint8_t g_mods_timeout = 0;
MODS_T g_tModS = {0};
VAR_T g_tVar;

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
	uint16_t addr;
	uint16_t crc1;

#if 0
	
	/* 超过3.5个字符时间后执行MODH_RxTimeOut()函数。全局变量 g_rtu_timeout = 1; 通知主程序开始解码 */
//	if (g_mods_timeout == 0)	
//	{
//		return;								/* 没有超时，继续接收。不要清零 g_tModS.RxCount */
//	}
//	
//	g_mods_timeout = 0;	 					/* 清标志 */

	if (g_tModS.RxCount < 7)				/* 接收到的数据小于4个字节就认为错误，地址（8bit）+指令（8bit）+操作寄存器（16bit） */
	{
		goto err_ret;
	}

	/* 计算CRC校验和，这里是将接收到的数据包含CRC16值一起做CRC16，结果是0，表示正确接收 */
	crc1 = CRC16_Modbus(g_tModS.RxBuf, g_tModS.RxCount);
	if (crc1 != 0)
	{
		goto err_ret;
	}

	/* 站地址 (1字节） */
	addr = g_tModS.RxBuf[0];				/* 第1字节 站号 */
	if (addr != MASTER_ADDRESS && addr !=0)		 			/* 判断主机发送的命令地址是否符合 */
	{
		goto err_ret;
	}

	/* 分析应用层协议 */
	MODS_AnalyzeApp();						
	
err_ret:
	g_tModS.RxCount = 0;					/* 必须清零计数器，方便下次帧同步 */
    #endif 
  if(g_tModS.Rx_rs485_data_flag ==  rs485_receive_data_success){
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
			g_tModS.Rx_rs485_data_flag=0;
		    g_tModS.RxCount =0;
		    

		}
    }
	if(crc16_check_flag==1){
		/* 鍒嗘瀽搴旂敤灞傚崗璁� */
		MODS_AnalyzeApp();
		crc16_check_flag=0;
	 
	
   	}
	
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

/*
*********************************************************************************************************
*	函 数 名: MODS_RxTimeOut
*	功能说明: 超过3.5个字符时间后执行本函数。 设置全局变量 g_mods_timeout = 1，通知主程序开始解码。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
//static void MODS_RxTimeOut(void)
//{
//	g_mods_timeout = 1;
//}

/*
*********************************************************************************************************
*	函 数 名: MODS_SendWithCRC
*	功能说明: 发送一串数据, 自动追加2字节CRC
*	形    参: _pBuf 数据；
*			  _ucLen 数据长度（不带CRC）
*	返 回 值: 无
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
*	函 数 名: MODS_SendAckErr
*	功能说明: 发送错误应答
*	形    参: _ucErrCode : 错误代码
*	返 回 值: 无
*********************************************************************************************************
*/
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
/*
*********************************************************************************************************
*	函 数 名: MODS_AnalyzeApp
*	功能说明: 分析应用层协议
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
#if 0
static void MODS_AnalyzeApp(void)
{
	switch (g_tModS.RxBuf[1])				/* 第2个字节 功能码 */
	{
		case 0x01:							/* 读取线圈状态（此例程用led代替）*/
			MODS_01H();
			bsp_PutMsg(MSG_MODS_01H, 0);	/* 发送消息,主程序处理 */
			break;

		case 0x02:							/* 读取输入状态（按键状态）*/
			MODS_02H();
			//bsp_PutMsg(MSG_MODS_02H, 0);
			break;
		
		case 0x03:							/* 读取保持寄存器（此例程存在g_tVar中）*/
			MODS_03H();
			//bsp_PutMsg(MSG_MODS_03H, 0);
			break;
		
		case 0x04:							/* 读取输入寄存器（ADC的值）*/
			MODS_04H();
			//bsp_PutMsg(MSG_MODS_04H, 0);
			break;
		
		case 0x05:							/* 强制单线圈（设置led）*/
			MODS_05H();
			//bsp_PutMsg(MSG_MODS_05H, 0);
			break;
		
		case 0x06:							/* 写单个保存寄存器（此例程改写g_tVar中的参数）*/
			MODS_06H();	
			//bsp_PutMsg(MSG_MODS_06H, 0);
			break;
			
		case 0x10:							/* 写多个保存寄存器（此例程存在g_tVar中的参数）*/
			MODS_10H();
			//bsp_PutMsg(MSG_MODS_10H, 0);
			break;
		
		default:
			//g_tModS.RspCode = RSP_ERR_CMD;
			MODS_SendAckErr(g_tModS.RspCode);	/* 告诉主机命令错误 */
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
*	函 数 名: MODS_01H
*	功能说明: 读取线圈状态（对应远程开关D01/D02/D03）
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void MODS_Read_Slave_Address_Info(void)
{
    uint8_t bytes_zero,byte_load_addr,byte_fun_code,byte_len,byte_data;


	  
	   bytes_zero = g_tModS.RxBuf[0];	/* 0x00 骞挎挱妯″紡 */
	   byte_load_addr = g_tModS.RxBuf[1]; /* 涓绘満  鍦板潃   0x01*/
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
	g_tModS.TxBuf[g_tModS.TxCount++] = 0x0;		/* 骞挎挱妯″紡 */
	g_tModS.TxBuf[g_tModS.TxCount++] = MASTER_ADDRESS;  /* 搴旂瓟鍦板潃 */
	g_tModS.TxBuf[g_tModS.TxCount++] = fun_code;		/* 鍔熻兘鐮� 绛夌瀛愬紑鎴栬€呭叧闂� */	
	g_tModS.TxBuf[g_tModS.TxCount++] = len;	/* 鏁版嵁闀垮害*/
	g_tModS.TxBuf[g_tModS.TxCount++] = data;		/* 鏁版嵁 */
	
	//MODS_SendAckWithCRC();		/* 鍙戦€佹暟鎹紝鑷姩鍔燙RC */
	MODS_SendWithCRC(g_tModS.TxBuf, g_tModS.TxCount);
	
	//g_tModH.RegNum = _num;		/* 瀵勫瓨鍣ㄤ釜鏁� */
	//g_tModH.Reg02H = _reg;		/* 淇濆瓨02H鎸囦护涓殑瀵勫瓨鍣ㄥ湴鍧€锛屾柟渚垮搴旂瓟鏁版嵁杩涜鍒嗙被 */	



}


