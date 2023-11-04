#ifndef __MODBUY_SLAVE_H
#define __MODBUY_SLAVE_H
#include "main.h"

#define PRO_HEAD        0xAA

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

    uint8_t rs485_send_signal_flag;
	uint8_t answering_signal_flag;

	uint8_t RspCode;

	uint8_t TxBuf[S_TX_BUF_SIZE];
	uint8_t rs485_RxInputBuf[1];
	uint8_t Rx_rs485_data_flag;
	uint8_t TxCount;
	uint8_t rs485_send_answering_signal_flag;

}MODS_T;

extern MODS_T g_tModS;


typedef struct
{
	/* 03H 06H ��д���ּĴ��� */
	uint8_t pro_head;
	uint8_t pro_boadcast;
	
	uint8_t pro_fun_code;
	uint8_t pro_data_len;
	uint8_t pro_data;

	uint16_t pro_addr;
	uint16_t pro_local_addr;


}Protocol_t;

typedef enum{

  rs485_send_err_fan_signal=0x01,
  rs485_send_err_ptc_signal,

}rs485_send_state;


typedef enum{

   rs485_receive_data_fail,
   rs485_receive_data_success,
   rs485_answering_signal_success,
   rs485_answering_signal_data,
   rs485_broadcast_mode
   
 }rs485_receive_state;


typedef enum {

  mod_power =0x01,
  mod_ptc,
  mod_plasma,
  mod_ulrasonic,
  mod_fan,
  mod_set_timer_power_on,
  mod_set_timer_power_off,
  mod_set_temperature_value =0xb0,
  mod_fan_error,
  mod_ptc_error,
   
}_mod_fun;




void MODS_Poll(void);

extern Protocol_t g_tPro;


void Answerback_RS485_Signal(uint16_t addr,uint8_t fun_code,uint8_t len,uint8_t data);
void MODS_SendHostError_Signal(uint8_t err);



#endif


