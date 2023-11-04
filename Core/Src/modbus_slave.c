#include "bsp.h"
#include "modbus_slave.h"

MAINBOARD_T g_tMain;
Protocol_t  g_tPro;


static void MODS_Read_Host_Analysis_Info(void);


/*
*********************************************************************************************************
*	                                   º¯ÊıÉùÃ÷
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
*	                                   ±äÁ¿
*********************************************************************************************************
*/
/*
Baud rate	Bit rate	 Bit time	 Character time	  3.5 character times
  2400	    2400 bits/s	  417 us	      4.6 ms	      16 ms
  4800	    4800 bits/s	  208 us	      2.3 ms	      8.0 ms
  9600	    9600 bits/s	  104 us	      1.2 ms	      4.0 ms
 19200	   19200 bits/s    52 us	      573 us	      2.0 ms
 38400	   38400 bits/s	   26 us	      286 us	      1.75 ms(1.0 ms)
 115200	   115200 bit/s	  8.7 us	       95 us	      1.75 ms(0.33 ms) ºóÃæ¹Ì¶¨¶¼Îª1750us
*/
typedef struct
{
	uint32_t Bps;
	uint32_t usTimeOut;
}MODBUSBPS_T;

const MODBUSBPS_T ModbusBaudRate[] =
{	
    {2400,	16000}, /* ²¨ÌØÂÊ2400bps, 3.5×Ö·ûÑÓ³ÙÊ±¼ä16000us */
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
*	º¯ Êı Ãû: MODS_Poll
*	¹¦ÄÜËµÃ÷: ½âÎöÊı¾İ°ü. ÔÚÖ÷³ÌĞòÖĞÂÖÁ÷µ÷ÓÃ¡£
*	ĞÎ    ²Î: ÎŞ
*	·µ »Ø Öµ: ÎŞ
*********************************************************************************************************
*/
void MODS_Poll(void)
{

	uint16_t crc1;

  if(g_tModS.Rx_rs485_data_flag ==  rs485_receive_data_success){

       g_tPro.pro_addr = BEBufToUint16(g_tModS.RxBuf); //å¤§ç«¯æ•°æ®å‘é€ ,1,2
	   g_tPro.pro_local_addr = BEBufToUint16(g_tModS.RxBuf); /* å¤§ç«¯æ•°æ®ï¼Œ3ï¼Œ4 */
  	    
		/* è®¡ç®—CRCæ ¡éªŒå’Œï¼Œè¿™é‡Œæ˜¯å°†æ¥æ”¶åˆ°çš„æ•°æ®åŒ…å«CRC16å€¼ä¸€èµ·åšCRC16ï¼Œç»“æœæ˜¯0ï¼Œè¡¨ç¤ºæ­£ç¡®æ¥æ”¶ */
		crc1 = CRC16_Modbus(g_tModS.RxBuf,g_tModS.RxCount);
		if (crc1 != 0)
		{
			g_tModS.RxCount = 0;	/* å¿…é¡»æ¸…é›¶è®¡æ•°å™¨ï¼Œæ–¹ä¾¿ä¸‹æ¬¡å¸§åŒæ­¥ */
			g_tModS.Rx_rs485_data_flag=0;
			//if(g_tModH.RxBuf[0]==0)run_t.broadcast_response_signal = FAIL_BROADCAST;
			Answerback_RS485_Signal(g_tModS.RxBuf[1],0xff,g_tModS.RxBuf[3],0x01);
		}
		else{
        	crc16_check_flag = 1;
			
		    }
    }
	if(crc16_check_flag==1){
		/* åˆ†æåº”ç”¨å±‚åè®® */
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
*	º¯ Êı Ãû: MODS_ReciveNew
*	¹¦ÄÜËµÃ÷: ´®¿Ú½ÓÊÕÖĞ¶Ï·şÎñ³ÌĞò»áµ÷ÓÃ±¾º¯Êı¡£µ±ÊÕµ½Ò»¸ö×Ö½ÚÊ±£¬Ö´ĞĞÒ»´Î±¾º¯Êı¡£
*	ĞÎ    ²Î: ÎŞ
*	·µ »Ø Öµ: ÎŞ
*********************************************************************************************************
*/
void MODS_ReciveNew(uint8_t _byte)
{
#if 0
	/*
		3.5¸ö×Ö·ûµÄÊ±¼ä¼ä¸ô£¬Ö»ÊÇÓÃÔÚRTUÄ£Ê½ÏÂÃæ£¬ÒòÎªRTUÄ£Ê½Ã»ÓĞ¿ªÊ¼·ûºÍ½áÊø·û£¬
		Á½¸öÊı¾İ°üÖ®¼äÖ»ÄÜ¿¿Ê±¼ä¼ä¸ôÀ´Çø·Ö£¬Modbus¶¨ÒåÔÚ²»Í¬µÄ²¨ÌØÂÊÏÂ£¬¼ä¸ôÊ±¼äÊÇ²»Ò»ÑùµÄ£¬
		ÏêÇé¿´´ËCÎÄ¼ş¿ªÍ·
	*/
	uint8_t i;
	
	/* ¸ù¾İ²¨ÌØÂÊ£¬»ñÈ¡ĞèÒªÑÓ³ÙµÄÊ±¼ä */
	for(i = 0; i < (sizeof(ModbusBaudRate)/sizeof(ModbusBaudRate[0])); i++)
	{
		if(SBAUD485 == ModbusBaudRate[i].Bps)
		{
			break;
		}	
	}

	g_mods_timeout = 0;
	
	/* Ó²¼ş¶¨Ê±ÖĞ¶Ï£¬¶¨Ê±¾«¶Èus Ó²¼ş¶¨Ê±Æ÷1ÓÃÓÚMODBUS´Ó»ú, ¶¨Ê±Æ÷2ÓÃÓÚMODBUSÖ÷»ú*/
	bsp_StartHardTimer(1, ModbusBaudRate[i].usTimeOut, (void *)MODS_RxTimeOut);
#endif 
	if (g_tModS.RxCount < S_RX_BUF_SIZE)
	{
		g_tModS.RxBuf[g_tModS.RxCount++] = _byte;
	}
}


/*******************************************************************************************************
	*
	*	º¯ Êı Ãû: MODS_SendWithCRC
	*	¹¦ÄÜËµÃ÷: ·¢ËÍÒ»´®Êı¾İ, ×Ô¶¯×·¼Ó2×Ö½ÚCRC
	*	ĞÎ    ²Î: _pBuf Êı¾İ£»
	*			  _ucLen Êı¾İ³¤¶È£¨²»´øCRC£©
	*	·µ »Ø Öµ: ÎŞ
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
*	º¯ Êı Ãû: MODS_SendAckErr
*	¹¦ÄÜËµÃ÷: ·¢ËÍ´íÎóÓ¦´ğ
*	ĞÎ    ²Î: _ucErrCode : ´íÎó´úÂë
*	·µ »Ø Öµ: ÎŞ
**********************************************************************************************************/
static void MODS_SendAckErr(uint8_t _ucErrCode)
{
	uint8_t txbuf[3];

	txbuf[0] = g_tModS.RxBuf[0];					/* 485µØÖ· */
	txbuf[1] = g_tModS.RxBuf[1] | 0x80;				/* Òì³£µÄ¹¦ÄÜÂë */
	txbuf[2] = _ucErrCode;							/* ´íÎó´úÂë(01,02,03,04) */

	MODS_SendWithCRC(txbuf, 3);
}

/*
*********************************************************************************************************
*	º¯ Êı Ãû: MODS_SendAckOk
*	¹¦ÄÜËµÃ÷: ·¢ËÍÕıÈ·µÄÓ¦´ğ.
*	ĞÎ    ²Î: ÎŞ
*	·µ »Ø Öµ: ÎŞ
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
*	Function: ¶ÁÈ¡ÏßÈ¦×´Ì¬£¨¶ÔÓ¦Ô¶³Ì¿ª¹ØD01/D02/D03£©
*	Input Ref: ÎŞ
*	Return Ref: ÎŞ
*
********************************************************************************************************/
static void MODS_Read_Host_Analysis_Info(void)
{
   
	   
	   g_tPro.pro_fun_code = g_tModS.RxBuf[5]; /* ä¸»æœº  åœ°å€   0x01*/
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
	g_tModS.TxBuf[g_tModS.TxCount++] = addr ;  /* åº”ç­”åœ°å€ low byte of address */
	g_tModS.TxBuf[g_tModS.TxCount++] = fun_code;		/* åŠŸèƒ½ç „1¤7 ç­‰ç¦»å­å¼€æˆ–è¢ã…å…³é—„1¤7 */	
	g_tModS.TxBuf[g_tModS.TxCount++] = len;	/* æ•°æ®é•¿åº¦*/
	g_tModS.TxBuf[g_tModS.TxCount++] = data;		/* æ•°æ® */
	
	//MODS_SendAckWithCRC();		/* å‘é¢ãæ•°æ®ï¼Œè‡ªåŠ¨åŠ CRC */
	MODS_SendWithCRC(g_tModS.TxBuf, g_tModS.TxCount);
	
	//g_tModH.RegNum = _num;		/* å¯„å­˜å™¨ä¸ªæ•„1¤7 */
	//g_tModH.Reg02H = _reg;		/* ä¿å­˜02HæŒ‡ä»¤ä¸­çš„å¯„å­˜å™¨åœ°å¢ãï¼Œæ–¹ä¾¿å¯¹åº”ç­”æ•°æ®è¿›è¡Œåˆ†ç±» */	



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
	g_tModS.TxBuf[g_tModS.TxCount++] = MASTER_ADDRESS;		/* å¹¿æ’­æ¨¡å¼ */
	g_tModS.TxBuf[g_tModS.TxCount++] = cpuId.slave_address >> 8;  /* åº”ç­”åœ°å€ */
	g_tModS.TxBuf[g_tModS.TxCount++] = cpuId.slave_address ;  /* åº”ç­”åœ°å€ low byte of address */
	g_tModS.TxBuf[g_tModS.TxCount++] = 0xff;	/* function: error of code */	
	g_tModS.TxBuf[g_tModS.TxCount++] = 0x01;	/* æ•°æ®é•¿åº¦*/
	g_tModS.TxBuf[g_tModS.TxCount++] = err;		/* erroræ•°æ® */
	
	//MODS_SendAckWithCRC();		/* å‘é¢ãæ•°æ®ï¼Œè‡ªåŠ¨åŠ CRC */
	MODS_SendWithCRC(g_tModS.TxBuf, g_tModS.TxCount);
	
	g_tModS.rs485_send_answering_signal_flag = 0;		/* å¯„å­˜å™¨ä¸ªæ•„1¤7 */
	//g_tModH.Reg02H = _reg;		/* ä¿å­˜02HæŒ‡ä»¤ä¸­çš„å¯„å­˜å™¨åœ°å¢ãï¼Œæ–¹ä¾¿å¯¹åº”ç­”æ•°æ®è¿›è¡Œåˆ†ç±» */	

}


