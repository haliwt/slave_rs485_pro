#ifndef _BSP_H_
#define _BSP_H

#define STM32G030F6      


/* 检查是否定义了开发板型号 */
#if !defined (STM32G030F6)
	#error "Please define the board model : STM32G030F6_V1"
#endif

/* 定义 BSP 版本号 */
#define __STM32G030_BSP_VERSION		"1.1"

/* CPU空闲时执行的函数 */
//#define CPU_IDLE()		bsp_Idle()

/* 开关全局中断的宏 */
#define ENABLE_INT()	__set_PRIMASK(0)	/* 使能全局中断 */
#define DISABLE_INT()	__set_PRIMASK(1)	/* 禁止全局中断 */

/* 这个宏仅用于调试阶段排错 */
#define BSP_Printf		printf
//#define BSP_Printf(...)

#define EXTI9_5_ISR_MOVE_OUT		/* bsp.h 中定义此行，表示本函数移到 stam32f4xx_it.c。 避免重复定义 */

#define ERROR_HANDLER()		Error_Handler(__FILE__, __LINE__);

/* 默认是关闭状态 */
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

/* 定义优先级分组 */
#define NVIC_PREEMPT_PRIORITY	4

/* 通过取消注释或者添加注释的方式控制是否包含底层驱动模块 */

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




/* 提供给其他C文件调用的函数 */
void bsp_Ref_Init(void);

void bsp_Idle(void);

void bsp_GetCpuID(void);
//void Error_Handler(char *file, uint32_t line);
extern void SystemClock_Config(void);
#endif

