#ifndef __BSP_MSG_H
#define __BSP_MSG_H
#include "main.h"

#define MSG_FIFO_SIZE    20//40	   		/* ��Ϣ���� */

enum 
{
	MSG_NONE = 0,
	
	MSG_MODS_01H,
	MSG_MODS_02H,
	MSG_MODS_03H,
	MSG_MODS_04H,
	MSG_MODS_05H,
	MSG_MODS_06H,
	MSG_MODS_10H,
};

/* ����FIFO�õ����� */
typedef struct
{
	uint16_t MsgCode;		/* ��Ϣ���� */
	uint32_t MsgParam;		/* ��Ϣ��������, Ҳ������ָ�루ǿ��ת���� */
}MSG_T;

/* ����FIFO�õ����� */
typedef struct
{
	MSG_T Buf[MSG_FIFO_SIZE];	/* ��Ϣ������ */
	uint8_t Read;					/* ��������ָ��1 */
	uint8_t Write;					/* ������дָ�� */
	uint8_t Read2;					/* ��������ָ��2 */
}MSG_FIFO_T;

/* ���ⲿ���õĺ������� */
void bsp_InitMsg(void);
void bsp_PutMsg(uint16_t _MsgCode, uint32_t _MsgParam);
uint8_t bsp_GetMsg(MSG_T *_pMsg);
uint8_t bsp_GetMsg2(MSG_T *_pMsg);
void bsp_ClearMsg(void);


#endif


