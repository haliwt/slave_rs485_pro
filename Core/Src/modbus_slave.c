/*
*********************************************************************************************************
*
*	Ä£¿éÃû³Æ : MODSÍ¨ÐÅÄ£¿é. ´ÓÕ¾Ä£Ê½¡¾Ô­´´¡¿
*	ÎÄ¼þÃû³Æ : modbus_slave.c
*	°æ    ±¾ : V1.5
*	Ëµ    Ã÷ : Í·ÎÄ¼þ
*
*	Copyright (C), 2020-2030, °²¸»À³µç×Ó www.armfly.com
*
*********************************************************************************************************
*/
#include "bsp.h"
#include "modbus_slave.h"

MAINBOARD_T g_tMain;

/*
*********************************************************************************************************
*	                                   º¯ÊýÉùÃ÷
*********************************************************************************************************
*/
static void MODS_SendWithCRC(uint8_t *_pBuf, uint8_t _ucLen);
static void MODS_SendAckOk(void);
static void MODS_SendAckErr(uint8_t _ucErrCode);

static void MODS_AnalyzeApp(void);

static void MODS_RxTimeOut(void);

static void MODS_01H(void);
static void MODS_02H(void);
static void MODS_03H(void);
static void MODS_04H(void);
static void MODS_05H(void);
static void MODS_06H(void);
static void MODS_10H(void);

static uint8_t MODS_ReadRegValue(uint16_t reg_addr, uint8_t *reg_value);
static uint8_t MODS_WriteRegValue(uint16_t reg_addr, uint16_t reg_value);

void MODS_ReciveNew(uint8_t _byte);


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

static MODS_Read_Slave_Address_Info(void); //MODS -> Modbus - slave machine ,MODH--> host machine

static uint8_t g_mods_timeout = 0;
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
static void MODS_RxTimeOut(void)
{
	g_mods_timeout = 1;
}

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
static MODS_Read_Slave_Address_Info(void)
{
    uint8_t bytes_zero,byte_load_addr,byte_fun_code,byte_len,byte_data,fun_byte;


	  
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
                       g_tMain.gPower_On = power_off;
				      
				       
				      
				   break;

				   case 1:
				      g_tMain.gPower_On = power_on;
					 

				   break;

				}	
					
				g_tModS.fAck01H = 1;
				
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
                g_tModS.fAck02H = 1;
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
                g_tModS.fAck03H = 1;
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
                g_tModS.fAck04H = 1;

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
	g_tModS.TxBuf[g_tModS.TxCount++] = fun_code;		/* åŠŸèƒ½ç  ç­‰ç¦»å­å¼€æˆ–è€…å…³é—­ */	
	g_tModS.TxBuf[g_tModS.TxCount++] = len;	/* æ•°æ®é•¿åº¦*/
	g_tModS.TxBuf[g_tModS.TxCount++] = data;		/* æ•°æ® */
	
	//MODS_SendAckWithCRC();		/* å‘é€æ•°æ®ï¼Œè‡ªåŠ¨åŠ CRC */
	MODS_SendWithCRC(g_tModS.TxBuf, g_tModS.TxCount);
	
	//g_tModH.RegNum = _num;		/* å¯„å­˜å™¨ä¸ªæ•° */
	//g_tModH.Reg02H = _reg;		/* ä¿å­˜02HæŒ‡ä»¤ä¸­çš„å¯„å­˜å™¨åœ°å€ï¼Œæ–¹ä¾¿å¯¹åº”ç­”æ•°æ®è¿›è¡Œåˆ†ç±» */	



}

/*
*********************************************************************************************************
*	º¯ Êý Ãû: MODS_01H
*	¹¦ÄÜËµÃ÷: ¶ÁÈ¡ÏßÈ¦×´Ì¬£¨¶ÔÓ¦Ô¶³Ì¿ª¹ØD01/D02/D03£©
*	ÐÎ    ²Î: ÎÞ
*	·µ »Ø Öµ: ÎÞ
*********************************************************************************************************
*/
/* ËµÃ÷:ÕâÀïÓÃLED´úÌæ¼ÌµçÆ÷,±ãÓÚ¹Û²ìÏÖÏó */
static void MODS_01H(void)
{
	/*
	 ¾ÙÀý£º
		Ö÷»ú·¢ËÍ:
			11 ´Ó»úµØÖ·
			01 ¹¦ÄÜÂë
			00 ¼Ä´æÆ÷ÆðÊ¼µØÖ·¸ß×Ö½Ú
			13 ¼Ä´æÆ÷ÆðÊ¼µØÖ·µÍ×Ö½Ú
			00 ¼Ä´æÆ÷ÊýÁ¿¸ß×Ö½Ú
			25 ¼Ä´æÆ÷ÊýÁ¿µÍ×Ö½Ú
			0E CRCÐ£Ñé¸ß×Ö½Ú
			84 CRCÐ£ÑéµÍ×Ö½Ú

		´Ó»úÓ¦´ð: 	1´ú±íON£¬0´ú±íOFF¡£Èô·µ»ØµÄÏßÈ¦Êý²»Îª8µÄ±¶Êý£¬ÔòÔÚ×îºóÊý¾Ý×Ö½ÚÎ´Î²Ê¹ÓÃ0´úÌæ. BIT0¶ÔÓ¦µÚ1¸ö
			11 ´Ó»úµØÖ·
			01 ¹¦ÄÜÂë
			05 ·µ»Ø×Ö½ÚÊý
			CD Êý¾Ý1(ÏßÈ¦0013H-ÏßÈ¦001AH)
			6B Êý¾Ý2(ÏßÈ¦001BH-ÏßÈ¦0022H)
			B2 Êý¾Ý3(ÏßÈ¦0023H-ÏßÈ¦002AH)
			0E Êý¾Ý4(ÏßÈ¦0032H-ÏßÈ¦002BH)
			1B Êý¾Ý5(ÏßÈ¦0037H-ÏßÈ¦0033H)
			45 CRCÐ£Ñé¸ß×Ö½Ú
			E6 CRCÐ£ÑéµÍ×Ö½Ú

		Àý×Ó:
			01 01 10 01 00 03   29 0B	--- ²éÑ¯D01¿ªÊ¼µÄ3¸ö¼ÌµçÆ÷×´Ì¬
			01 01 10 03 00 01   09 0A   --- ²éÑ¯D03¼ÌµçÆ÷µÄ×´Ì¬
	*/
	uint16_t reg;
	uint16_t num;
	uint16_t i;
	uint16_t m;
	uint8_t status[10];
	
	g_tModS.RspCode = RSP_OK;

	/** µÚ1²½£º ÅÐ¶Ï½Óµ½Ö¸¶¨¸öÊýÊý¾Ý ===============================================================*/
	/*  Ã»ÓÐÍâ²¿¼ÌµçÆ÷£¬Ö±½ÓÓ¦´ð´íÎó 
		µØÖ·£¨8bit£©+Ö¸Áî£¨8bit£©+¼Ä´æÆ÷ÆðÊ¼µØÖ·¸ßµÍ×Ö½Ú£¨16bit£©+¼Ä´æÆ÷¸öÊý£¨16bit£©+ CRC16
	*/
	if (g_tModS.RxCount != 8)
	{
		g_tModS.RspCode = RSP_ERR_VALUE;				/* Êý¾ÝÖµÓò´íÎó */
		return;
	}

	/** µÚ2²½£º Êý¾Ý½âÎö ===========================================================================*/
	/* Êý¾ÝÊÇ´ó¶Ë£¬Òª×ª»»ÎªÐ¡¶Ë */
	reg = BEBufToUint16(&g_tModS.RxBuf[2]); 			/* ¼Ä´æÆ÷ºÅ */
	num = BEBufToUint16(&g_tModS.RxBuf[4]);				/* ¼Ä´æÆ÷¸öÊý */

	/* ²»×ã×Ö½ÚÕûÊý±¶£¬²¹Æë */
	m = (num + 7) / 8;
	
	/* ½âÎöÖ÷»úÃüÁîÒª¶ÁÈ¡µÄ×´Ì¬ */
	if ((reg >= REG_D01) && (num > 0) && (reg + num <= REG_DXX + 1))
	{
		for (i = 0; i < m; i++)
		{
			status[i] = 0;
		}
		
		for (i = 0; i < num; i++)
		{
			//if (bsp_IsLedOn(i + 1 + reg - REG_D01))		/* ¶ÁLEDµÄ×´Ì¬£¬Ð´Èë×´Ì¬¼Ä´æÆ÷µÄÃ¿Ò»Î» */
			{  
				status[i / 8] |= (1 << (i % 8));
			}
		}
	}
	else
	{
		g_tModS.RspCode = RSP_ERR_REG_ADDR;				/* ¼Ä´æÆ÷µØÖ·´íÎó */
	}

	/** µÚ3²½£º Ó¦´ð»Ø¸´ =========================================================================*/
	if (g_tModS.RspCode == RSP_OK)						/* ÕýÈ·Ó¦´ð */
	{
		g_tModS.TxCount = 0;
		g_tModS.TxBuf[g_tModS.TxCount++] = g_tModS.RxBuf[0]; /* ·µ»Ø´Ó»úµØÖ· */
		g_tModS.TxBuf[g_tModS.TxCount++] = g_tModS.RxBuf[1]; /* ·µ»Ø´Ó»úÖ¸Áî */
		g_tModS.TxBuf[g_tModS.TxCount++] = m;				 /* ·µ»Ø×Ö½ÚÊý */

		for (i = 0; i < m; i++)
		{
			g_tModS.TxBuf[g_tModS.TxCount++] = status[i];	/* ·µ»Ø¼ÌµçÆ÷×´Ì¬ */
		}
		MODS_SendWithCRC(g_tModS.TxBuf, g_tModS.TxCount);
	}
	else
	{
		MODS_SendAckErr(g_tModS.RspCode);				/* ¸æËßÖ÷»úÃüÁî´íÎó */
	}
}

/*
*********************************************************************************************************
*	º¯ Êý Ãû: MODS_02H
*	¹¦ÄÜËµÃ÷: ¶ÁÈ¡ÊäÈë×´Ì¬£¨¶ÔÓ¦K01¡«K03£©
*	ÐÎ    ²Î: ÎÞ
*	·µ »Ø Öµ: ÎÞ
*********************************************************************************************************
*/
static void MODS_02H(void)
{
	/*
		Ö÷»ú·¢ËÍ:
			11 ´Ó»úµØÖ·
			02 ¹¦ÄÜÂë
			00 ¼Ä´æÆ÷µØÖ·¸ß×Ö½Ú
			C4 ¼Ä´æÆ÷µØÖ·µÍ×Ö½Ú
			00 ¼Ä´æÆ÷ÊýÁ¿¸ß×Ö½Ú
			16 ¼Ä´æÆ÷ÊýÁ¿µÍ×Ö½Ú
			BA CRCÐ£Ñé¸ß×Ö½Ú
			A9 CRCÐ£ÑéµÍ×Ö½Ú

		´Ó»úÓ¦´ð:  ÏìÓ¦¸÷ÀëÉ¢ÊäÈë¼Ä´æÆ÷×´Ì¬£¬·Ö±ð¶ÔÓ¦Êý¾ÝÇøÖÐµÄÃ¿Î»Öµ£¬1 ´ú±íON£»0 ´ú±íOFF¡£
		           µÚÒ»¸öÊý¾Ý×Ö½ÚµÄLSB(×îµÍ×Ö½Ú)Îª²éÑ¯µÄÑ°Ö·µØÖ·£¬ÆäËûÊäÈë¿Ú°´Ë³ÐòÔÚ¸Ã×Ö½ÚÖÐÓÉµÍ×Ö½Ú
		           Ïò¸ß×Ö½ÚÅÅÁÐ£¬Ö±µ½Ìî³äÂú8Î»¡£ÏÂÒ»¸ö×Ö½ÚÖÐµÄ8¸öÊäÈëÎ»Ò²ÊÇ´ÓµÍ×Ö½Úµ½¸ß×Ö½ÚÅÅÁÐ¡£
		           Èô·µ»ØµÄÊäÈëÎ»Êý²»ÊÇ8µÄ±¶Êý£¬ÔòÔÚ×îºóµÄÊý¾Ý×Ö½ÚÖÐµÄÊ£ÓàÎ»ÖÁ¸Ã×Ö½ÚµÄ×î¸ßÎ»Ê¹ÓÃ0Ìî³ä¡£
			11 ´Ó»úµØÖ·
			02 ¹¦ÄÜÂë
			03 ·µ»Ø×Ö½ÚÊý
			AC Êý¾Ý1(00C4H-00CBH)
			DB Êý¾Ý2(00CCH-00D3H)
			35 Êý¾Ý3(00D4H-00D9H)
			20 CRCÐ£Ñé¸ß×Ö½Ú
			18 CRCÐ£ÑéµÍ×Ö½Ú

		Àý×Ó:
		01 02 20 01 00 08  23CC  ---- ¶ÁÈ¡T01-08µÄ×´Ì¬
		01 02 20 04 00 02  B3CA  ---- ¶ÁÈ¡T04-05µÄ×´Ì¬
		01 02 20 01 00 12  A207   ---- ¶Á T01-18
	*/

	uint16_t reg;
	uint16_t num;
	uint16_t i;
	uint16_t m;
	uint8_t status[10];

	g_tModS.RspCode = RSP_OK;

    /** µÚ1²½£º ÅÐ¶Ï½Óµ½Ö¸¶¨¸öÊýÊý¾Ý ===============================================================*/
	/* µØÖ·£¨8bit£©+Ö¸Áî£¨8bit£©+¼Ä´æÆ÷ÆðÊ¼µØÖ·¸ßµÍ×Ö½Ú£¨16bit£©+¼Ä´æÆ÷¸öÊý£¨16bit£©+ CRC16 */
	if (g_tModS.RxCount != 8)
	{
		g_tModS.RspCode = RSP_ERR_VALUE;				/* Êý¾ÝÖµÓò´íÎó */
		return;
	}

	/** µÚ2²½£º Êý¾Ý½âÎö ===========================================================================*/
	/* Êý¾ÝÊÇ´ó¶Ë£¬Òª×ª»»ÎªÐ¡¶Ë */
	reg = BEBufToUint16(&g_tModS.RxBuf[2]); 			/* ¼Ä´æÆ÷ºÅ */
	num = BEBufToUint16(&g_tModS.RxBuf[4]);				/* ¼Ä´æÆ÷¸öÊý */

	/* ²»×ã×Ö½ÚÕûÊý±¶£¬²¹Æë */
	m = (num + 7) / 8;
	if ((reg >= REG_T01) && (num > 0) && (reg + num <= REG_TXX + 1))
	{
		for (i = 0; i < m; i++)
		{
			status[i] = 0;
		}
		for (i = 0; i < num; i++)
		{
//			if (bsp_GetKeyState((KEY_ID_E)(KID_K1 + reg - REG_T01 + i)))
//			{
//				status[i / 8] |= (1 << (i % 8));
//			}
		}
	}
	else
	{
		g_tModS.RspCode = RSP_ERR_REG_ADDR;				/* ¼Ä´æÆ÷µØÖ·´íÎó */
	}

	/** µÚ3²½£º Ó¦´ð»Ø¸´ =========================================================================*/
	if (g_tModS.RspCode == RSP_OK)						/* ÕýÈ·Ó¦´ð */
	{
		g_tModS.TxCount = 0;
		g_tModS.TxBuf[g_tModS.TxCount++] = g_tModS.RxBuf[0]; /* ·µ»Ø´Ó»úµØÖ· */
		g_tModS.TxBuf[g_tModS.TxCount++] = g_tModS.RxBuf[1]; /* ·µ»Ø´Ó»úÖ¸Áî */
		g_tModS.TxBuf[g_tModS.TxCount++] = m;				 /* ·µ»Ø×Ö½ÚÊý */

		for (i = 0; i < m; i++)
		{
			g_tModS.TxBuf[g_tModS.TxCount++] = status[i];	/* ·µ»ØT01-02×´Ì¬ */
		}
		MODS_SendWithCRC(g_tModS.TxBuf, g_tModS.TxCount);
	}
	else
	{
		MODS_SendAckErr(g_tModS.RspCode);				/* ¸æËßÖ÷»úÃüÁî´íÎó */
	}
}

/*
*********************************************************************************************************
*	º¯ Êý Ãû: MODS_ReadRegValue
*	¹¦ÄÜËµÃ÷: ¶ÁÈ¡±£³Ö¼Ä´æÆ÷µÄÖµ
*	ÐÎ    ²Î: reg_addr ¼Ä´æÆ÷µØÖ·
*			  reg_value ´æ·Å¼Ä´æÆ÷½á¹û
*	·µ »Ø Öµ: 1±íÊ¾OK 0±íÊ¾´íÎó
*********************************************************************************************************
*/
static uint8_t MODS_ReadRegValue(uint16_t reg_addr, uint8_t *reg_value)
{
	uint16_t value;
	
	switch (reg_addr)									/* ÅÐ¶Ï¼Ä´æÆ÷µØÖ· */
	{
		case SLAVE_REG_P01:
			value =	g_tVar.P01;	
			break;

		case SLAVE_REG_P02:
			value =	g_tVar.P02;							/* ½«¼Ä´æÆ÷Öµ¶Á³ö */
			break;
	
		default:
			return 0;									/* ²ÎÊýÒì³££¬·µ»Ø 0 */
	}

	reg_value[0] = value >> 8;                          /* ×¢ÒâÊý¾ÝÊÇ´ó¶Ë  */
	reg_value[1] = value;

	return 1;											/* ¶ÁÈ¡³É¹¦ */
}

/*
*********************************************************************************************************
*	º¯ Êý Ãû: MODS_WriteRegValue
*	¹¦ÄÜËµÃ÷: Ð´±£³Ö¼Ä´æÆ÷µÄÖµ
*	ÐÎ    ²Î: reg_addr ¼Ä´æÆ÷µØÖ·
*			  reg_value ¼Ä´æÆ÷Öµ
*	·µ »Ø Öµ: 1±íÊ¾OK 0±íÊ¾´íÎó
*********************************************************************************************************
*/
static uint8_t MODS_WriteRegValue(uint16_t reg_addr, uint16_t reg_value)
{
	switch (reg_addr)							/* ÅÐ¶Ï¼Ä´æÆ÷µØÖ· */
	{	
		case SLAVE_REG_P01:
			g_tVar.P01 = reg_value;				/* ½«ÖµÐ´Èë±£´æ¼Ä´æÆ÷ */
			break;
		
		case SLAVE_REG_P02:
			g_tVar.P02 = reg_value;				/* ½«ÖµÐ´Èë±£´æ¼Ä´æÆ÷ */
			break;
		
		default:
			return 0;		/* ²ÎÊýÒì³££¬·µ»Ø 0 */
	}

	return 1;		/* ¶ÁÈ¡³É¹¦ */
}

/*
*********************************************************************************************************
*	º¯ Êý Ãû: MODS_03H
*	¹¦ÄÜËµÃ÷: ¶ÁÈ¡±£³Ö¼Ä´æÆ÷ ÔÚÒ»¸ö»ò¶à¸ö±£³Ö¼Ä´æÆ÷ÖÐÈ¡µÃµ±Ç°µÄ¶þ½øÖÆÖµ
*	ÐÎ    ²Î: ÎÞ
*	·µ »Ø Öµ: ÎÞ
*********************************************************************************************************
*/
static void MODS_03H(void)
{
	/*
		´Ó»úµØÖ·Îª11H¡£±£³Ö¼Ä´æÆ÷µÄÆðÊ¼µØÖ·Îª006BH£¬½áÊøµØÖ·Îª006DH¡£¸Ã´Î²éÑ¯×Ü¹²·ÃÎÊ3¸ö±£³Ö¼Ä´æÆ÷¡£

		Ö÷»ú·¢ËÍ:
			11 ´Ó»úµØÖ·
			03 ¹¦ÄÜÂë
			00 ¼Ä´æÆ÷µØÖ·¸ß×Ö½Ú
			6B ¼Ä´æÆ÷µØÖ·µÍ×Ö½Ú
			00 ¼Ä´æÆ÷ÊýÁ¿¸ß×Ö½Ú
			03 ¼Ä´æÆ÷ÊýÁ¿µÍ×Ö½Ú
			76 CRC¸ß×Ö½Ú
			87 CRCµÍ×Ö½Ú

		´Ó»úÓ¦´ð: 	±£³Ö¼Ä´æÆ÷µÄ³¤¶ÈÎª2¸ö×Ö½Ú¡£¶ÔÓÚµ¥¸ö±£³Ö¼Ä´æÆ÷¶øÑÔ£¬¼Ä´æÆ÷¸ß×Ö½ÚÊý¾ÝÏÈ±»´«Êä£¬
					µÍ×Ö½ÚÊý¾Ýºó±»´«Êä¡£±£³Ö¼Ä´æÆ÷Ö®¼ä£¬µÍµØÖ·¼Ä´æÆ÷ÏÈ±»´«Êä£¬¸ßµØÖ·¼Ä´æÆ÷ºó±»´«Êä¡£
			11 ´Ó»úµØÖ·
			03 ¹¦ÄÜÂë
			06 ×Ö½ÚÊý
			00 Êý¾Ý1¸ß×Ö½Ú(006BH)
			6B Êý¾Ý1µÍ×Ö½Ú(006BH)
			00 Êý¾Ý2¸ß×Ö½Ú(006CH)
			13 Êý¾Ý2 µÍ×Ö½Ú(006CH)
			00 Êý¾Ý3¸ß×Ö½Ú(006DH)
			00 Êý¾Ý3µÍ×Ö½Ú(006DH)
			38 CRC¸ß×Ö½Ú
			B9 CRCµÍ×Ö½Ú

		Àý×Ó:
			01 03 30 06 00 01  6B0B      ---- ¶Á 3006H, ´¥·¢µçÁ÷
			01 03 4000 0010 51C6         ---- ¶Á 4000H µ¹ÊýµÚ1ÌõÀËÓ¿¼ÇÂ¼ 32×Ö½Ú
			01 03 4001 0010 0006         ---- ¶Á 4001H µ¹ÊýµÚ1ÌõÀËÓ¿¼ÇÂ¼ 32×Ö½Ú

			01 03 F000 0008 770C         ---- ¶Á F000H µ¹ÊýµÚ1Ìõ¸æ¾¯¼ÇÂ¼ 16×Ö½Ú
			01 03 F001 0008 26CC         ---- ¶Á F001H µ¹ÊýµÚ2Ìõ¸æ¾¯¼ÇÂ¼ 16×Ö½Ú

			01 03 7000 0020 5ED2         ---- ¶Á 7000H µ¹ÊýµÚ1Ìõ²¨ÐÎ¼ÇÂ¼µÚ1¶Î 64×Ö½Ú
			01 03 7001 0020 0F12         ---- ¶Á 7001H µ¹ÊýµÚ1Ìõ²¨ÐÎ¼ÇÂ¼µÚ2¶Î 64×Ö½Ú

			01 03 7040 0020 5F06         ---- ¶Á 7040H µ¹ÊýµÚ2Ìõ²¨ÐÎ¼ÇÂ¼µÚ1¶Î 64×Ö½Ú
	*/
	uint16_t reg;
	uint16_t num;
	uint16_t i;
	uint8_t reg_value[64];

	g_tModS.RspCode = RSP_OK;

    /** µÚ1²½£º ÅÐ¶Ï½Óµ½Ö¸¶¨¸öÊýÊý¾Ý ===============================================================*/
	/* µØÖ·£¨8bit£©+Ö¸Áî£¨8bit£©+¼Ä´æÆ÷ÆðÊ¼µØÖ·¸ßµÍ×Ö½Ú£¨16bit£©+¼Ä´æÆ÷¸öÊý£¨16bit£©+ CRC16 */
	if (g_tModS.RxCount != 8)								/* 03HÃüÁî±ØÐëÊÇ8¸ö×Ö½Ú */
	{
		g_tModS.RspCode = RSP_ERR_VALUE;					/* Êý¾ÝÖµÓò´íÎó */
		goto err_ret;
	}

	/** µÚ2²½£º Êý¾Ý½âÎö ===========================================================================*/
	/* Êý¾ÝÊÇ´ó¶Ë£¬Òª×ª»»ÎªÐ¡¶Ë */
	reg = BEBufToUint16(&g_tModS.RxBuf[2]); 				/* ¼Ä´æÆ÷ºÅ */
	num = BEBufToUint16(&g_tModS.RxBuf[4]);					/* ¼Ä´æÆ÷¸öÊý */
	
	/* ¶ÁÈ¡µÄÊý¾Ý¸öÊýÒªÔÚ·¶Î§ÄÚ */
	if (num > sizeof(reg_value) / 2)
	{
		g_tModS.RspCode = RSP_ERR_VALUE;					/* Êý¾ÝÖµÓò´íÎó */
		goto err_ret;
	}

	/* ¶ÁÈ¡µÄÊý¾Ý´æÈëµ½reg_valueÀïÃæ */
	for (i = 0; i < num; i++)
	{
		if (MODS_ReadRegValue(reg, &reg_value[2 * i]) == 0)	/* ¶Á³ö¼Ä´æÆ÷Öµ·ÅÈëreg_value£¬´Ëº¯ÊýÒÑ¾­×öÁË´ó¶Ë×ªÐ¡¶Ë´¦Àí */
		{
			g_tModS.RspCode = RSP_ERR_REG_ADDR;				/* ¼Ä´æÆ÷µØÖ·´íÎó */
			break;
		}
		reg++;
	}

	/** µÚ3²½£º Ó¦´ð»Ø¸´ =========================================================================*/
err_ret:
	if (g_tModS.RspCode == RSP_OK)							 /* ÕýÈ·Ó¦´ð */
	{
		g_tModS.TxCount = 0;
		g_tModS.TxBuf[g_tModS.TxCount++] = g_tModS.RxBuf[0]; /* ·µ»Ø´Ó»úµØÖ· */
		g_tModS.TxBuf[g_tModS.TxCount++] = g_tModS.RxBuf[1]; /* ·µ»Ø´Ó»úÖ¸Áî */
		g_tModS.TxBuf[g_tModS.TxCount++] = num * 2;			 /* ·µ»Ø×Ö½ÚÊý */

		for (i = 0; i < num; i++)                            /* ·µ»ØÊý¾Ý*/ 
		{
			g_tModS.TxBuf[g_tModS.TxCount++] = reg_value[2*i];
			g_tModS.TxBuf[g_tModS.TxCount++] = reg_value[2*i+1];
		}
		MODS_SendWithCRC(g_tModS.TxBuf, g_tModS.TxCount);	/* ·¢ËÍÕýÈ·Ó¦´ð */
	}
	else
	{
		MODS_SendAckErr(g_tModS.RspCode);					/* ·¢ËÍ´íÎóÓ¦´ð */
	}
}

/*
*********************************************************************************************************
*	º¯ Êý Ãû: MODS_04H
*	¹¦ÄÜËµÃ÷: ¶ÁÈ¡ÊäÈë¼Ä´æÆ÷£¨¶ÔÓ¦A01/A02£© SMA
*	ÐÎ    ²Î: ÎÞ
*	·µ »Ø Öµ: ÎÞ
*********************************************************************************************************
*/
static void MODS_04H(void)
{
	/*
		Ö÷»ú·¢ËÍ:
			11 ´Ó»úµØÖ·
			04 ¹¦ÄÜÂë
			00 ¼Ä´æÆ÷ÆðÊ¼µØÖ·¸ß×Ö½Ú
			08 ¼Ä´æÆ÷ÆðÊ¼µØÖ·µÍ×Ö½Ú
			00 ¼Ä´æÆ÷¸öÊý¸ß×Ö½Ú
			02 ¼Ä´æÆ÷¸öÊýµÍ×Ö½Ú
			F2 CRC¸ß×Ö½Ú
			99 CRCµÍ×Ö½Ú

		´Ó»úÓ¦´ð:  ÊäÈë¼Ä´æÆ÷³¤¶ÈÎª2¸ö×Ö½Ú¡£¶ÔÓÚµ¥¸öÊäÈë¼Ä´æÆ÷¶øÑÔ£¬¼Ä´æÆ÷¸ß×Ö½ÚÊý¾ÝÏÈ±»´«Êä£¬
				µÍ×Ö½ÚÊý¾Ýºó±»´«Êä¡£ÊäÈë¼Ä´æÆ÷Ö®¼ä£¬µÍµØÖ·¼Ä´æÆ÷ÏÈ±»´«Êä£¬¸ßµØÖ·¼Ä´æÆ÷ºó±»´«Êä¡£
			11 ´Ó»úµØÖ·
			04 ¹¦ÄÜÂë
			04 ×Ö½ÚÊý
			00 Êý¾Ý1¸ß×Ö½Ú(0008H)
			0A Êý¾Ý1µÍ×Ö½Ú(0008H)
			00 Êý¾Ý2¸ß×Ö½Ú(0009H)
			0B Êý¾Ý2µÍ×Ö½Ú(0009H)
			8B CRC¸ß×Ö½Ú
			80 CRCµÍ×Ö½Ú

		Àý×Ó:

			01 04 2201 0006 2BB0  --- ¶Á 2201H A01Í¨µÀÄ£ÄâÁ¿ ¿ªÊ¼µÄ6¸öÊý¾Ý
			01 04 2201 0001 6A72  --- ¶Á 2201H

	*/
	uint16_t reg;
	uint16_t num;
	uint16_t i;
	uint16_t status[10];

	memset(status, 0, 20);

    /** µÚ1²½£º ÅÐ¶Ï½Óµ½Ö¸¶¨¸öÊýÊý¾Ý ===============================================================*/
	/* µØÖ·£¨8bit£©+Ö¸Áî£¨8bit£©+¼Ä´æÆ÷ÆðÊ¼µØÖ·¸ßµÍ×Ö½Ú£¨16bit£©+¼Ä´æÆ÷¸öÊý£¨16bit£©+ CRC16 */
	g_tModS.RspCode = RSP_OK;

	if (g_tModS.RxCount != 8)
	{
		g_tModS.RspCode = RSP_ERR_VALUE;	/* Êý¾ÝÖµÓò´íÎó */
		goto err_ret;
	}

	/** µÚ2²½£º Êý¾Ý½âÎö ===========================================================================*/
	/* Êý¾ÝÊÇ´ó¶Ë£¬Òª×ª»»ÎªÐ¡¶Ë */
	reg = BEBufToUint16(&g_tModS.RxBuf[2]); /* ¼Ä´æÆ÷ºÅ */
	num = BEBufToUint16(&g_tModS.RxBuf[4]);	/* ¼Ä´æÆ÷¸öÊý */
	
	/* ¶ÁÈ¡Êý¾Ý */
	if ((reg >= REG_A01) && (num > 0) && (reg + num <= REG_AXX + 1))
	{	
		for (i = 0; i < num; i++)
		{
			switch (reg)
			{
				/* ²âÊÔ²ÎÊý */
				case REG_A01:
					status[i] = g_tVar.A01;
					break;
					
				default:
					status[i] = 0;
					break;
			}
			reg++;
		}
	}
	else
	{
		g_tModS.RspCode = RSP_ERR_REG_ADDR;		/* ¼Ä´æÆ÷µØÖ·´íÎó */
	}

	/** µÚ3²½£º Ó¦´ð»Ø¸´ =========================================================================*/
err_ret:
	if (g_tModS.RspCode == RSP_OK)		/* ÕýÈ·Ó¦´ð */
	{
		g_tModS.TxCount = 0;
		g_tModS.TxBuf[g_tModS.TxCount++] = g_tModS.RxBuf[0]; /* ·µ»Ø´Ó»úµØÖ· */
		g_tModS.TxBuf[g_tModS.TxCount++] = g_tModS.RxBuf[1]; /* ·µ»Ø´Ó»úÖ¸Áî */ 
		g_tModS.TxBuf[g_tModS.TxCount++] = num * 2;			 /* ·µ»Ø×Ö½ÚÊý */

		for (i = 0; i < num; i++)                            /* ·µ»ØÊý¾Ý */
		{
			g_tModS.TxBuf[g_tModS.TxCount++] = status[i] >> 8;
			g_tModS.TxBuf[g_tModS.TxCount++] = status[i] & 0xFF;
		}
		MODS_SendWithCRC(g_tModS.TxBuf, g_tModS.TxCount);   /* ·¢ËÍÕýÈ·Ó¦´ð */
	}
	else
	{
		MODS_SendAckErr(g_tModS.RspCode);	/* ¸æËßÖ÷»úÃüÁî´íÎó */
	}
}

/*
*********************************************************************************************************
*	º¯ Êý Ãû: MODS_05H
*	¹¦ÄÜËµÃ÷: Ç¿ÖÆÐ´µ¥ÏßÈ¦£¨¶ÔÓ¦D01/D02/D03£©
*	ÐÎ    ²Î: ÎÞ
*	·µ »Ø Öµ: ÎÞ
*********************************************************************************************************
*/
static void MODS_05H(void)
{
	/*
		Ö÷»ú·¢ËÍ: Ð´µ¥¸öÏßÈ¦¼Ä´æÆ÷¡£FF00HÖµÇëÇóÏßÈ¦´¦ÓÚON×´Ì¬£¬0000HÖµÇëÇóÏßÈ¦´¦ÓÚOFF×´Ì¬
		¡£05HÖ¸ÁîÉèÖÃµ¥¸öÏßÈ¦µÄ×´Ì¬£¬15HÖ¸Áî¿ÉÒÔÉèÖÃ¶à¸öÏßÈ¦µÄ×´Ì¬¡£
			11 ´Ó»úµØÖ·
			05 ¹¦ÄÜÂë
			00 ¼Ä´æÆ÷µØÖ·¸ß×Ö½Ú
			AC ¼Ä´æÆ÷µØÖ·µÍ×Ö½Ú
			FF Êý¾Ý1¸ß×Ö½Ú
			00 Êý¾Ý2µÍ×Ö½Ú
			4E CRCÐ£Ñé¸ß×Ö½Ú
			8B CRCÐ£ÑéµÍ×Ö½Ú

		´Ó»úÓ¦´ð:
			11 ´Ó»úµØÖ·
			05 ¹¦ÄÜÂë
			00 ¼Ä´æÆ÷µØÖ·¸ß×Ö½Ú
			AC ¼Ä´æÆ÷µØÖ·µÍ×Ö½Ú
			FF ¼Ä´æÆ÷1¸ß×Ö½Ú
			00 ¼Ä´æÆ÷1µÍ×Ö½Ú
			4E CRCÐ£Ñé¸ß×Ö½Ú
			8B CRCÐ£ÑéµÍ×Ö½Ú

		Àý×Ó:
		01 05 10 01 FF 00   D93A   -- D01´ò¿ª
		01 05 10 01 00 00   98CA   -- D01¹Ø±Õ

		01 05 10 02 FF 00   293A   -- D02´ò¿ª
		01 05 10 02 00 00   68CA   -- D02¹Ø±Õ

		01 05 10 03 FF 00   78FA   -- D03´ò¿ª
		01 05 10 03 00 00   390A   -- D03¹Ø±Õ
	*/
	uint16_t reg;
	uint16_t value;

	g_tModS.RspCode = RSP_OK;
	
    /** µÚ1²½£º ÅÐ¶Ï½Óµ½Ö¸¶¨¸öÊýÊý¾Ý ===============================================================*/
	/* µØÖ·£¨8bit£©+Ö¸Áî£¨8bit£©+¼Ä´æÆ÷ÆðÊ¼µØÖ·¸ßµÍ×Ö½Ú£¨16bit£©+¼Ä´æÆ÷¸öÊý£¨16bit£©+ CRC16 */
	if (g_tModS.RxCount != 8)
	{
		g_tModS.RspCode = RSP_ERR_VALUE;		/* Êý¾ÝÖµÓò´íÎó */
		goto err_ret;
	}

	/** µÚ2²½£º Êý¾Ý½âÎö ===========================================================================*/
	/* Êý¾ÝÊÇ´ó¶Ë£¬Òª×ª»»ÎªÐ¡¶Ë */
	reg = BEBufToUint16(&g_tModS.RxBuf[2]); 	/* ¼Ä´æÆ÷ºÅ */
	value = BEBufToUint16(&g_tModS.RxBuf[4]);	/* Êý¾Ý */
	
	if (value != 0x0000 && value != 0xFF00)
	{
		g_tModS.RspCode = RSP_ERR_VALUE;		/* Êý¾ÝÖµÓò´íÎó */
		goto err_ret;
	}
	
	/* ÉèÖÃÊýÖµ */
	if (reg == REG_D01)
	{
		g_tVar.D01 = value;
	}
	else if (reg == REG_D02)
	{
		g_tVar.D02 = value;
	}
	else if (reg == REG_D03)
	{
		g_tVar.D03 = value;
	}
	else if (reg == REG_D04)
	{
		g_tVar.D04 = value;
	}
	else
	{
		g_tModS.RspCode = RSP_ERR_REG_ADDR;		/* ¼Ä´æÆ÷µØÖ·´íÎó */
	}
	
	/** µÚ3²½£º Ó¦´ð»Ø¸´ =========================================================================*/
err_ret:
	if (g_tModS.RspCode == RSP_OK)				/* ÕýÈ·Ó¦´ð */
	{
		MODS_SendAckOk();
	}
	else
	{
		MODS_SendAckErr(g_tModS.RspCode);		/* ¸æËßÖ÷»úÃüÁî´íÎó */
	}
}

/*
*********************************************************************************************************
*	º¯ Êý Ãû: MODS_06H
*	¹¦ÄÜËµÃ÷: Ð´µ¥¸ö¼Ä´æÆ÷
*	ÐÎ    ²Î: ÎÞ
*	·µ »Ø Öµ: ÎÞ
*********************************************************************************************************
*/
static void MODS_06H(void)
{

	/*
		Ð´±£³Ö¼Ä´æÆ÷¡£×¢Òâ06Ö¸ÁîÖ»ÄÜ²Ù×÷µ¥¸ö±£³Ö¼Ä´æÆ÷£¬16Ö¸Áî¿ÉÒÔÉèÖÃµ¥¸ö»ò¶à¸ö±£³Ö¼Ä´æÆ÷

		Ö÷»ú·¢ËÍ:
			11 ´Ó»úµØÖ·
			06 ¹¦ÄÜÂë
			00 ¼Ä´æÆ÷µØÖ·¸ß×Ö½Ú
			01 ¼Ä´æÆ÷µØÖ·µÍ×Ö½Ú
			00 Êý¾Ý1¸ß×Ö½Ú
			01 Êý¾Ý1µÍ×Ö½Ú
			9A CRCÐ£Ñé¸ß×Ö½Ú
			9B CRCÐ£ÑéµÍ×Ö½Ú

		´Ó»úÏìÓ¦:
			11 ´Ó»úµØÖ·
			06 ¹¦ÄÜÂë
			00 ¼Ä´æÆ÷µØÖ·¸ß×Ö½Ú
			01 ¼Ä´æÆ÷µØÖ·µÍ×Ö½Ú
			00 Êý¾Ý1¸ß×Ö½Ú
			01 Êý¾Ý1µÍ×Ö½Ú
			1B CRCÐ£Ñé¸ß×Ö½Ú
			5A	CRCÐ£ÑéµÍ×Ö½Ú

		Àý×Ó:
			01 06 30 06 00 25  A710    ---- ´¥·¢µçÁ÷ÉèÖÃÎª 2.5
			01 06 30 06 00 10  6707    ---- ´¥·¢µçÁ÷ÉèÖÃÎª 1.0


			01 06 30 1B 00 00  F6CD    ---- SMA ÂË²¨ÏµÊý = 0 ¹Ø±ÕÂË²¨
			01 06 30 1B 00 01  370D    ---- SMA ÂË²¨ÏµÊý = 1
			01 06 30 1B 00 02  770C    ---- SMA ÂË²¨ÏµÊý = 2
			01 06 30 1B 00 05  36CE    ---- SMA ÂË²¨ÏµÊý = 5

			01 06 30 07 00 01  F6CB    ---- ²âÊÔÄ£Ê½ÐÞ¸ÄÎª T1
			01 06 30 07 00 02  B6CA    ---- ²âÊÔÄ£Ê½ÐÞ¸ÄÎª T2

			01 06 31 00 00 00  8736    ---- ²Á³ýÀËÓ¿¼ÇÂ¼Çø
			01 06 31 01 00 00  D6F6    ---- ²Á³ý¸æ¾¯¼ÇÂ¼Çø

*/

	uint16_t reg;
	uint16_t value;

	g_tModS.RspCode = RSP_OK;

    /** µÚ1²½£º ÅÐ¶Ï½Óµ½Ö¸¶¨¸öÊýÊý¾Ý ===============================================================*/
	/* µØÖ·£¨8bit£©+Ö¸Áî£¨8bit£©+¼Ä´æÆ÷ÆðÊ¼µØÖ·¸ßµÍ×Ö½Ú£¨16bit£©+¼Ä´æÆ÷¸öÊý£¨16bit£©+ CRC16 */
	if (g_tModS.RxCount != 8)
	{
		g_tModS.RspCode = RSP_ERR_VALUE;		/* Êý¾ÝÖµÓò´íÎó */
		goto err_ret;
	}

	/** µÚ2²½£º Êý¾Ý½âÎö ===========================================================================*/
	/* Êý¾ÝÊÇ´ó¶Ë£¬Òª×ª»»ÎªÐ¡¶Ë */
	reg = BEBufToUint16(&g_tModS.RxBuf[2]); 	/* ¼Ä´æÆ÷ºÅ */
	value = BEBufToUint16(&g_tModS.RxBuf[4]);	/* ¼Ä´æÆ÷Öµ */

	if (MODS_WriteRegValue(reg, value) == 1)	/* ¸Ãº¯Êý»á°ÑÐ´ÈëµÄÖµ´æÈë¼Ä´æÆ÷ */
	{
		;
	}
	else
	{
		g_tModS.RspCode = RSP_ERR_REG_ADDR;		/* ¼Ä´æÆ÷µØÖ·´íÎó */
	}

	/** µÚ3²½£º Ó¦´ð»Ø¸´ =========================================================================*/
err_ret:
	if (g_tModS.RspCode == RSP_OK)				/* ÕýÈ·Ó¦´ð */
	{
		MODS_SendAckOk();
	}
	else
	{
		MODS_SendAckErr(g_tModS.RspCode);		/* ¸æËßÖ÷»úÃüÁî´íÎó */
	}
}

/*
*********************************************************************************************************
*	º¯ Êý Ãû: MODS_10H
*	¹¦ÄÜËµÃ÷: Á¬ÐøÐ´¶à¸ö¼Ä´æÆ÷.  ½øÓÃÓÚ¸ÄÐ´Ê±ÖÓ
*	ÐÎ    ²Î: ÎÞ
*	·µ »Ø Öµ: ÎÞ
*********************************************************************************************************
*/
static void MODS_10H(void)
{
	/*
		´Ó»úµØÖ·Îª11H¡£±£³Ö¼Ä´æÆ÷µÄÆäÊµµØÖ·Îª0001H£¬¼Ä´æÆ÷µÄ½áÊøµØÖ·Îª0002H¡£×Ü¹²·ÃÎÊ2¸ö¼Ä´æÆ÷¡£
		±£³Ö¼Ä´æÆ÷0001HµÄÄÚÈÝÎª000AH£¬±£³Ö¼Ä´æÆ÷0002HµÄÄÚÈÝÎª0102H¡£

		Ö÷»ú·¢ËÍ:
			11 ´Ó»úµØÖ·
			10 ¹¦ÄÜÂë
			00 ¼Ä´æÆ÷ÆðÊ¼µØÖ·¸ß×Ö½Ú
			01 ¼Ä´æÆ÷ÆðÊ¼µØÖ·µÍ×Ö½Ú
			00 ¼Ä´æÆ÷ÊýÁ¿¸ß×Ö½Ú
			02 ¼Ä´æÆ÷ÊýÁ¿µÍ×Ö½Ú
			04 ×Ö½ÚÊý
			00 Êý¾Ý1¸ß×Ö½Ú
			0A Êý¾Ý1µÍ×Ö½Ú
			01 Êý¾Ý2¸ß×Ö½Ú
			02 Êý¾Ý2µÍ×Ö½Ú
			C6 CRCÐ£Ñé¸ß×Ö½Ú
			F0 CRCÐ£ÑéµÍ×Ö½Ú

		´Ó»úÏìÓ¦:
			11 ´Ó»úµØÖ·
			06 ¹¦ÄÜÂë
			00 ¼Ä´æÆ÷µØÖ·¸ß×Ö½Ú
			01 ¼Ä´æÆ÷µØÖ·µÍ×Ö½Ú
			00 Êý¾Ý1¸ß×Ö½Ú
			01 Êý¾Ý1µÍ×Ö½Ú
			1B CRCÐ£Ñé¸ß×Ö½Ú
			5A	CRCÐ£ÑéµÍ×Ö½Ú

		Àý×Ó:
			01 10 30 00 00 06 0C  07 DE  00 0A  00 01  00 08  00 0C  00 00     389A    ---- Ð´Ê±ÖÓ 2014-10-01 08:12:00
			01 10 30 00 00 06 0C  07 DF  00 01  00 1F  00 17  00 3B  00 39     5549    ---- Ð´Ê±ÖÓ 2015-01-31 23:59:57

	*/
	uint16_t reg_addr;
	uint16_t reg_num;
	uint8_t byte_num;
	uint8_t i;
	uint16_t value;
	
	g_tModS.RspCode = RSP_OK;

    /** µÚ1²½£º ÅÐ¶Ï½Óµ½Ö¸¶¨¸öÊýÊý¾Ý ===============================================================*/
	/* µØÖ·£¨8bit£©+Ö¸Áî£¨8bit£©+¼Ä´æÆ÷ÆðÊ¼µØÖ·¸ßµÍ×Ö½Ú£¨16bit£©+¼Ä´æÆ÷¸öÊý£¨16bit£©+ ×Ö½ÚÊý£¨8bit£©+ Êý¾Ý¸ßµÍ×Ö½Ú£¨16bit£©+ CRC16 */
	if (g_tModS.RxCount < 11)
	{
		g_tModS.RspCode = RSP_ERR_VALUE;			/* Êý¾ÝÖµÓò´íÎó */
		goto err_ret;
	}

	/** µÚ2²½£º Êý¾Ý½âÎö ===========================================================================*/
	/* Êý¾ÝÊÇ´ó¶Ë£¬Òª×ª»»ÎªÐ¡¶Ë */
	reg_addr = BEBufToUint16(&g_tModS.RxBuf[2]); 	/* ¼Ä´æÆ÷ºÅ */
	reg_num = BEBufToUint16(&g_tModS.RxBuf[4]);		/* ¼Ä´æÆ÷¸öÊý */
	byte_num = g_tModS.RxBuf[6];					/* ºóÃæµÄÊý¾ÝÌå×Ö½ÚÊý */

	/* ÅÐ¶Ï¼Ä´æÆ÷¸öÊýºÍºóÃæÊý¾Ý×Ö½ÚÊýÊÇ·ñÒ»ÖÂ */
	if (byte_num != 2 * reg_num)
	{
		;
	}
	
	/* Êý¾ÝÐ´Èë */
	for (i = 0; i < reg_num; i++)
	{
		value = BEBufToUint16(&g_tModS.RxBuf[7 + 2 * i]);	/* ¼Ä´æÆ÷Öµ */

		if (MODS_WriteRegValue(reg_addr + i, value) == 1)
		{
			;
		}
		else
		{
			g_tModS.RspCode = RSP_ERR_REG_ADDR;		/* ¼Ä´æÆ÷µØÖ·´íÎó */
			break;
		}
	}

	/** µÚ3²½£º Ó¦´ð»Ø¸´ =========================================================================*/
err_ret:
	if (g_tModS.RspCode == RSP_OK)					/* ÕýÈ·Ó¦´ð */
	{
		MODS_SendAckOk();
	}
	else
	{
		MODS_SendAckErr(g_tModS.RspCode);			/* ¸æËßÖ÷»úÃüÁî´íÎó */
	}
}

/***************************** °²¸»À³µç×Ó www.armfly.com (END OF FILE) *********************************/
