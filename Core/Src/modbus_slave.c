#include "bsp.h"
#include "modbus_slave.h"

MAINBOARD_T g_tMain;

/*
*********************************************************************************************************
*	                                   º¯ÊýÉùÃ÷
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

static void MODS_Read_Slave_Address_Info(void); //MODS -> Modbus - slave machine ,MODH--> host machine

//static uint8_t g_mods_timeout = 0;
MODS_T g_tModS = {0};
VAR_T g_tVar;

/*
*********************************************************************************************************
*	º¯ Êý Ãû: MODS_Poll
*	¹¦ÄÜËµÃ÷: ½âÎöÊý¾Ý°ü. ÔÚÖ÷³ÌÐòÖÐÂÖÁ÷µ÷ÓÃ¡£
*	ÐÎ    ²Î: ÎÞ
*	·µ »Ø Öµ: ÎÞ
*********************************************************************************************************
*/
void MODS_Poll(void)
{
	uint16_t addr;
	uint16_t crc1;

#if 0
	
	/* ³¬¹ý3.5¸ö×Ö·ûÊ±¼äºóÖ´ÐÐMODH_RxTimeOut()º¯Êý¡£È«¾Ö±äÁ¿ g_rtu_timeout = 1; Í¨ÖªÖ÷³ÌÐò¿ªÊ¼½âÂë */
//	if (g_mods_timeout == 0)	
//	{
//		return;								/* Ã»ÓÐ³¬Ê±£¬¼ÌÐø½ÓÊÕ¡£²»ÒªÇåÁã g_tModS.RxCount */
//	}
//	
//	g_mods_timeout = 0;	 					/* Çå±êÖ¾ */

	if (g_tModS.RxCount < 7)				/* ½ÓÊÕµ½µÄÊý¾ÝÐ¡ÓÚ4¸ö×Ö½Ú¾ÍÈÏÎª´íÎó£¬µØÖ·£¨8bit£©+Ö¸Áî£¨8bit£©+²Ù×÷¼Ä´æÆ÷£¨16bit£© */
	{
		goto err_ret;
	}

	/* ¼ÆËãCRCÐ£ÑéºÍ£¬ÕâÀïÊÇ½«½ÓÊÕµ½µÄÊý¾Ý°üº¬CRC16ÖµÒ»Æð×öCRC16£¬½á¹ûÊÇ0£¬±íÊ¾ÕýÈ·½ÓÊÕ */
	crc1 = CRC16_Modbus(g_tModS.RxBuf, g_tModS.RxCount);
	if (crc1 != 0)
	{
		goto err_ret;
	}

	/* Õ¾µØÖ· (1×Ö½Ú£© */
	addr = g_tModS.RxBuf[0];				/* µÚ1×Ö½Ú Õ¾ºÅ */
	if (addr != MASTER_ADDRESS && addr !=0)		 			/* ÅÐ¶ÏÖ÷»ú·¢ËÍµÄÃüÁîµØÖ·ÊÇ·ñ·ûºÏ */
	{
		goto err_ret;
	}

	/* ·ÖÎöÓ¦ÓÃ²ãÐ­Òé */
	MODS_AnalyzeApp();						
	
err_ret:
	g_tModS.RxCount = 0;					/* ±ØÐëÇåÁã¼ÆÊýÆ÷£¬·½±ãÏÂ´ÎÖ¡Í¬²½ */
    #endif 
  if(g_tModS.Rx_rs485_data_flag ==  rs485_receive_data_success){
		/* è®¡ç®—CRCæ ¡éªŒå’Œï¼Œè¿™é‡Œæ˜¯å°†æŽ¥æ”¶åˆ°çš„æ•°æ®åŒ…å«CRC16å€¼ä¸€èµ·åšCRC16ï¼Œç»“æžœæ˜¯0ï¼Œè¡¨ç¤ºæ­£ç¡®æŽ¥æ”¶ */
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
			g_tModS.Rx_rs485_data_flag=0;
		    g_tModS.RxCount =0;
		    

		}
    }
	if(crc16_check_flag==1){
		/* åˆ†æžåº”ç”¨å±‚åè®® */
		MODS_AnalyzeApp();
		crc16_check_flag=0;
	 
	
   	}
	
}

/*
*********************************************************************************************************
*	º¯ Êý Ãû: MODS_ReciveNew
*	¹¦ÄÜËµÃ÷: ´®¿Ú½ÓÊÕÖÐ¶Ï·þÎñ³ÌÐò»áµ÷ÓÃ±¾º¯Êý¡£µ±ÊÕµ½Ò»¸ö×Ö½ÚÊ±£¬Ö´ÐÐÒ»´Î±¾º¯Êý¡£
*	ÐÎ    ²Î: ÎÞ
*	·µ »Ø Öµ: ÎÞ
*********************************************************************************************************
*/
void MODS_ReciveNew(uint8_t _byte)
{
#if 0
	/*
		3.5¸ö×Ö·ûµÄÊ±¼ä¼ä¸ô£¬Ö»ÊÇÓÃÔÚRTUÄ£Ê½ÏÂÃæ£¬ÒòÎªRTUÄ£Ê½Ã»ÓÐ¿ªÊ¼·ûºÍ½áÊø·û£¬
		Á½¸öÊý¾Ý°üÖ®¼äÖ»ÄÜ¿¿Ê±¼ä¼ä¸ôÀ´Çø·Ö£¬Modbus¶¨ÒåÔÚ²»Í¬µÄ²¨ÌØÂÊÏÂ£¬¼ä¸ôÊ±¼äÊÇ²»Ò»ÑùµÄ£¬
		ÏêÇé¿´´ËCÎÄ¼þ¿ªÍ·
	*/
	uint8_t i;
	
	/* ¸ù¾Ý²¨ÌØÂÊ£¬»ñÈ¡ÐèÒªÑÓ³ÙµÄÊ±¼ä */
	for(i = 0; i < (sizeof(ModbusBaudRate)/sizeof(ModbusBaudRate[0])); i++)
	{
		if(SBAUD485 == ModbusBaudRate[i].Bps)
		{
			break;
		}	
	}

	g_mods_timeout = 0;
	
	/* Ó²¼þ¶¨Ê±ÖÐ¶Ï£¬¶¨Ê±¾«¶Èus Ó²¼þ¶¨Ê±Æ÷1ÓÃÓÚMODBUS´Ó»ú, ¶¨Ê±Æ÷2ÓÃÓÚMODBUSÖ÷»ú*/
	bsp_StartHardTimer(1, ModbusBaudRate[i].usTimeOut, (void *)MODS_RxTimeOut);
#endif 
	if (g_tModS.RxCount < S_RX_BUF_SIZE)
	{
		g_tModS.RxBuf[g_tModS.RxCount++] = _byte;
	}
}

/*
*********************************************************************************************************
*	º¯ Êý Ãû: MODS_RxTimeOut
*	¹¦ÄÜËµÃ÷: ³¬¹ý3.5¸ö×Ö·ûÊ±¼äºóÖ´ÐÐ±¾º¯Êý¡£ ÉèÖÃÈ«¾Ö±äÁ¿ g_mods_timeout = 1£¬Í¨ÖªÖ÷³ÌÐò¿ªÊ¼½âÂë¡£
*	ÐÎ    ²Î: ÎÞ
*	·µ »Ø Öµ: ÎÞ
*********************************************************************************************************
*/
//static void MODS_RxTimeOut(void)
//{
//	g_mods_timeout = 1;
//}

/*
*********************************************************************************************************
*	º¯ Êý Ãû: MODS_SendWithCRC
*	¹¦ÄÜËµÃ÷: ·¢ËÍÒ»´®Êý¾Ý, ×Ô¶¯×·¼Ó2×Ö½ÚCRC
*	ÐÎ    ²Î: _pBuf Êý¾Ý£»
*			  _ucLen Êý¾Ý³¤¶È£¨²»´øCRC£©
*	·µ »Ø Öµ: ÎÞ
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
*	º¯ Êý Ãû: MODS_SendAckErr
*	¹¦ÄÜËµÃ÷: ·¢ËÍ´íÎóÓ¦´ð
*	ÐÎ    ²Î: _ucErrCode : ´íÎó´úÂë
*	·µ »Ø Öµ: ÎÞ
*********************************************************************************************************
*/
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
*	º¯ Êý Ãû: MODS_SendAckOk
*	¹¦ÄÜËµÃ÷: ·¢ËÍÕýÈ·µÄÓ¦´ð.
*	ÐÎ    ²Î: ÎÞ
*	·µ »Ø Öµ: ÎÞ
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
*	º¯ Êý Ãû: MODS_AnalyzeApp
*	¹¦ÄÜËµÃ÷: ·ÖÎöÓ¦ÓÃ²ãÐ­Òé
*	ÐÎ    ²Î: ÎÞ
*	·µ »Ø Öµ: ÎÞ
*********************************************************************************************************
*/
#if 0
static void MODS_AnalyzeApp(void)
{
	switch (g_tModS.RxBuf[1])				/* µÚ2¸ö×Ö½Ú ¹¦ÄÜÂë */
	{
		case 0x01:							/* ¶ÁÈ¡ÏßÈ¦×´Ì¬£¨´ËÀý³ÌÓÃled´úÌæ£©*/
			MODS_01H();
			bsp_PutMsg(MSG_MODS_01H, 0);	/* ·¢ËÍÏûÏ¢,Ö÷³ÌÐò´¦Àí */
			break;

		case 0x02:							/* ¶ÁÈ¡ÊäÈë×´Ì¬£¨°´¼ü×´Ì¬£©*/
			MODS_02H();
			//bsp_PutMsg(MSG_MODS_02H, 0);
			break;
		
		case 0x03:							/* ¶ÁÈ¡±£³Ö¼Ä´æÆ÷£¨´ËÀý³Ì´æÔÚg_tVarÖÐ£©*/
			MODS_03H();
			//bsp_PutMsg(MSG_MODS_03H, 0);
			break;
		
		case 0x04:							/* ¶ÁÈ¡ÊäÈë¼Ä´æÆ÷£¨ADCµÄÖµ£©*/
			MODS_04H();
			//bsp_PutMsg(MSG_MODS_04H, 0);
			break;
		
		case 0x05:							/* Ç¿ÖÆµ¥ÏßÈ¦£¨ÉèÖÃled£©*/
			MODS_05H();
			//bsp_PutMsg(MSG_MODS_05H, 0);
			break;
		
		case 0x06:							/* Ð´µ¥¸ö±£´æ¼Ä´æÆ÷£¨´ËÀý³Ì¸ÄÐ´g_tVarÖÐµÄ²ÎÊý£©*/
			MODS_06H();	
			//bsp_PutMsg(MSG_MODS_06H, 0);
			break;
			
		case 0x10:							/* Ð´¶à¸ö±£´æ¼Ä´æÆ÷£¨´ËÀý³Ì´æÔÚg_tVarÖÐµÄ²ÎÊý£©*/
			MODS_10H();
			//bsp_PutMsg(MSG_MODS_10H, 0);
			break;
		
		default:
			//g_tModS.RspCode = RSP_ERR_CMD;
			MODS_SendAckErr(g_tModS.RspCode);	/* ¸æËßÖ÷»úÃüÁî´íÎó */
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
*	º¯ Êý Ãû: MODS_01H
*	¹¦ÄÜËµÃ÷: ¶ÁÈ¡ÏßÈ¦×´Ì¬£¨¶ÔÓ¦Ô¶³Ì¿ª¹ØD01/D02/D03£©
*	ÐÎ    ²Î: ÎÞ
*	·µ »Ø Öµ: ÎÞ
*********************************************************************************************************
*/
static void MODS_Read_Slave_Address_Info(void)
{
    uint8_t bytes_zero,byte_load_addr,byte_fun_code,byte_len,byte_data;


	  
	   bytes_zero = g_tModS.RxBuf[0];	/* 0x00 å¹¿æ’­æ¨¡å¼ */
	   byte_load_addr = g_tModS.RxBuf[1]; /* ä¸»æœº  åœ°å€   0x01*/
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
	g_tModS.TxBuf[g_tModS.TxCount++] = 0x0;		/* å¹¿æ’­æ¨¡å¼ */
	g_tModS.TxBuf[g_tModS.TxCount++] = MASTER_ADDRESS;  /* åº”ç­”åœ°å€ */
	g_tModS.TxBuf[g_tModS.TxCount++] = fun_code;		/* åŠŸèƒ½ç „1¤7 ç­‰ç¦»å­å¼€æˆ–è¢ã…å…³é—„1¤7 */	
	g_tModS.TxBuf[g_tModS.TxCount++] = len;	/* æ•°æ®é•¿åº¦*/
	g_tModS.TxBuf[g_tModS.TxCount++] = data;		/* æ•°æ® */
	
	//MODS_SendAckWithCRC();		/* å‘é¢ãæ•°æ®ï¼Œè‡ªåŠ¨åŠ CRC */
	MODS_SendWithCRC(g_tModS.TxBuf, g_tModS.TxCount);
	
	//g_tModH.RegNum = _num;		/* å¯„å­˜å™¨ä¸ªæ•„1¤7 */
	//g_tModH.Reg02H = _reg;		/* ä¿å­˜02HæŒ‡ä»¤ä¸­çš„å¯„å­˜å™¨åœ°å¢ãï¼Œæ–¹ä¾¿å¯¹åº”ç­”æ•°æ®è¿›è¡Œåˆ†ç±» */	



}


