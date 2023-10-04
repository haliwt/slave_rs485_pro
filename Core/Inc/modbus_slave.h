#ifndef __MODBUY_SLAVE_H
#define __MODBUY_SLAVE_H
#include "main.h"

#define MASTER_ADDRESS 	0x01

#define SBAUD485	UART1_BAUD

/* 01H ��ǿ�Ƶ���Ȧ */
/* 05H дǿ�Ƶ���Ȧ */
#define REG_D01		0x0101
#define REG_D02		0x0102
#define REG_D03		0x0103
#define REG_D04		0x0104
#define REG_DXX 	REG_D04

/* 02H ��ȡ����״̬ */
#define REG_T01		0x0201
#define REG_T02		0x0202
#define REG_T03		0x0203
#define REG_TXX		REG_T03

/* 03H �����ּĴ��� */
/* 06H д���ּĴ��� */
/* 10H д�������Ĵ��� */
#define SLAVE_REG_P01		0x0301
#define SLAVE_REG_P02		0x0302

/* 04H ��ȡ����Ĵ���(ģ���ź�) */
#define REG_A01		0x0401
#define REG_AXX		REG_A01


/* RTU Ӧ����� */
#define RSP_OK				0		/* �ɹ� */
#define RSP_ERR_CRC_CHECK   0x01	/* crc16 ceck code is error */
#define RSP_ERR_CMD			0x02	/* ��֧�ֵĹ����� */
#define RSP_ERR_REG_ADDR	0x03	/* �Ĵ�����ַ���� */
#define RSP_ERR_VALUE		0x04	/* ����ֵ����� */
#define RSP_ERR_WRITE		0x05	/* д��ʧ�� */

#define S_RX_BUF_SIZE		30
#define S_TX_BUF_SIZE		20

typedef struct
{
	uint8_t RxBuf[S_RX_BUF_SIZE];
	uint8_t RxCount;
	uint8_t RxStatus;
	uint8_t RxNewFlag;

	uint8_t RspCode;

	uint8_t TxBuf[S_TX_BUF_SIZE];
	uint8_t TxCount;
}MODS_T;

typedef struct
{
	/* 03H 06H ��д���ּĴ��� */
	uint16_t P01;
	uint16_t P02;

	/* 04H ��ȡģ�����Ĵ��� */
	uint16_t A01;

	/* 01H 05H ��д����ǿ����Ȧ */
	uint16_t D01;
	uint16_t D02;
	uint16_t D03;
	uint16_t D04;

}VAR_T;

typedef struct {

    uint8_t gPower_On;
	uint8_t gFan_continueRun;
	uint8_t  gFan_counter;
	uint8_t gTimer_fan_adc_times;
	uint8_t gFan_level;

	//adc 
	uint8_t ADC_channel_No;

	uint8_t  gPlasma;
    uint8_t  gPtc;
    uint8_t  gUltrasonic;

	uint8_t  gTemperature;
    uint8_t  gHumidity;

	//fault 
	uint8_t fan_warning ;
	uint8_t ptc_warning;


}MAINBOARD_T;


typedef enum {

  mod_power =0x01,
  mod_ptc,
  mod_plasma,
  mod_ulrasonic,
  mod_fan,
  mod_set_timer_power_on,
  mod_set_timer_power_off,
  mod_set_temperature_value,
  mod_fan_error,
  mod_ptc_error,
   
}_mod_fun;

typedef enum{
  
  power_off,
  power_on,


}power_state;


void MODS_Poll(void);
extern MODS_T g_tModS;
extern VAR_T g_tVar;
extern MAINBOARD_T g_tMain;

void Answerback_RS485_Signal(uint8_t addr,uint8_t fun_code,uint8_t len,uint8_t data);

#endif


