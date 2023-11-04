#ifndef _BSP_H_
#define _BSP_H

#define STM32G030F6      


/* ����Ƿ����˿������ͺ� */
#if !defined (STM32G030F6)
	#error "Please define the board model : STM32G030F6_V1"
#endif

/* ���� BSP �汾�� */
#define __STM32G030_BSP_VERSION		"1.1"

/* CPU����ʱִ�еĺ��� */
//#define CPU_IDLE()		bsp_Idle()

/* ����ȫ���жϵĺ� */
#define ENABLE_INT()	__set_PRIMASK(0)	/* ʹ��ȫ���ж� */
#define DISABLE_INT()	__set_PRIMASK(1)	/* ��ֹȫ���ж� */

/* ���������ڵ��Խ׶��Ŵ� */
#define BSP_Printf		printf
//#define BSP_Printf(...)

#define EXTI9_5_ISR_MOVE_OUT		/* bsp.h �ж�����У���ʾ�������Ƶ� stam32f4xx_it.c�� �����ظ����� */

#define ERROR_HANDLER()		Error_Handler(__FILE__, __LINE__);

/* Ĭ���ǹر�״̬ */
#define  Enable_EventRecorder  0

#if Enable_EventRecorder == 1
	#include "EventRecorder.h"
#endif

#include "main.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifndef TRUE
	#define TRUE  1
#endif

#ifndef FALSE
	#define FALSE 0
#endif

/* �������ȼ����� */
#define NVIC_PREEMPT_PRIORITY	4

/* ͨ��ȡ��ע�ͻ������ע�͵ķ�ʽ�����Ƿ�����ײ�����ģ�� */

#include "tim.h"
#include "bsp_msg.h"
#include "bsp_uart_fifo.h"
#include "usart.h"
#include "modbus_slave.h"
#include "bsp_user_lib.h"
#include "bsp_control.h"
#include "delay.h"
#include "interrupt_manager.h"
#include "iwdg.h"
#include "bsp_sensor.h"
#include "bsp_beep.h"
#include "adc.h"

typedef struct _cpuId_{

   uint8_t cpu_id;
   uint16_t slave_address;
  

}CPUID;

extern CPUID cpuId;




/* �ṩ������C�ļ����õĺ��� */
void bsp_Ref_Init(void);

void bsp_Idle(void);

void bsp_GetCpuID(void);
//void Error_Handler(char *file, uint32_t line);
extern void SystemClock_Config(void);
#endif

