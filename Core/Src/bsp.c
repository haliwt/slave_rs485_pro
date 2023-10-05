#include "bsp.h"

CPUID cpuId;


/*
*********************************************************************************************************
*	函 数 名: SystemClock_Config
*	功能说明: 初始化系统时钟
*            	System Clock source            = PLL (HSE)
*            	SYSCLK(Hz)                     = 400000000 (CPU Clock)
*           	HCLK(Hz)                       = 200000000 (AXI and AHBs Clock)
*            	AHB Prescaler                  = 2
*            	D1 APB3 Prescaler              = 2 (APB3 Clock  100MHz)
*            	D2 APB1 Prescaler              = 2 (APB1 Clock  100MHz)
*            	D2 APB2 Prescaler              = 2 (APB2 Clock  100MHz)
*            	D3 APB4 Prescaler              = 2 (APB4 Clock  100MHz)
*            	HSE Frequency(Hz)              = 25000000
*           	PLL_M                          = 5
*            	PLL_N                          = 160
*            	PLL_P                          = 2
*            	PLL_Q                          = 4
*            	PLL_R                          = 2
*            	VDD(V)                         = 3.3
*            	Flash Latency(WS)              = 4
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/

void bsp_GetCpuID(void)
{
  static uint8_t id;
  uint32_t CPU_Sn0, CPU_Sn1, CPU_Sn2;
  uint8_t i;
 /* 妫�娴婥PU ID */
	
	  
	   CPU_Sn0 =  HAL_GetUIDw0();
	   CPU_Sn1 =  HAL_GetUIDw1();
	   CPU_Sn2 =  HAL_GetUIDw2();

		//printf("CPU : STM32H743XIH6, BGA240, 涓婚: %dMHz\r\n", SystemCoreClock / 1000000);
		//printf("UID = %08X %08X %08X\r\n", CPU_Sn2, CPU_Sn1, CPU_Sn0);
	


   do{
   	 i++;
	 if(i==0)
	   id = (uint8_t)CPU_Sn0 + (uint8_t)CPU_Sn1 + (uint8_t)CPU_Sn2;
	 else if(i==1)
       id = (uint8_t)CPU_Sn0 + (uint8_t)CPU_Sn1 +(uint8_t)(CPU_Sn2 >> 8);
	 else if(i==2)
       id = (uint8_t)CPU_Sn0 + (uint8_t)CPU_Sn1 +(uint8_t)(CPU_Sn2 >> 16);
	 else if(i==3) id=10;


   }while(id ==0);


   cpuId.slave_address = id;
}



/*
*********************************************************************************************************
*	函 数 名: bsp_RunPer10ms
*	功能说明: 该函数每隔10ms被Systick中断调用1次。详见 bsp_timer.c的定时中断服务程序。一些处理时间要求不严格的
*			任务可以放在此函数。比如：按键扫描、蜂鸣器鸣叫控制等。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_RunPer10ms(void)
{
	//bsp_KeyScan10ms();
}

/*
*********************************************************************************************************
*	函 数 名: bsp_RunPer1ms
*	功能说明: 该函数每隔1ms被Systick中断调用1次。详见 bsp_timer.c的定时中断服务程序。一些需要周期性处理的事务
*			 可以放在此函数。比如：触摸坐标扫描。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_RunPer1ms(void)
{
	
}

/*
*********************************************************************************************************
*	函 数 名: bsp_Idle
*	功能说明: 空闲时执行的函数。一般主程序在for和while循环程序体中需要插入 CPU_IDLE() 宏来调用本函数。
*			 本函数缺省为空操作。用户可以添加喂狗、设置CPU进入休眠模式的功能。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_Idle(void)
{
	/* --- 喂狗 */

	/* --- 让CPU进入休眠，由Systick定时中断唤醒或者其他中断唤醒 */

	/* 例如 emWin 图形库，可以插入图形库需要的轮询函数 */
	//GUI_Exec();

	/* 例如 uIP 协议，可以插入uip轮询函数 */
    MODS_Poll();
}

/*
*********************************************************************************************************
*	函 数 名: HAL_Delay
*	功能说明: 重定向毫秒延迟函数。替换HAL中的函数。因为HAL中的缺省函数依赖于Systick中断，如果在USB、SD卡
*             中断中有延迟函数，则会锁死。也可以通过函数HAL_NVIC_SetPriority提升Systick中断
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
/* 当前例子使用stm32h7xx_hal.c默认方式实现，未使用下面重定向的函数 */
#if 0
void HAL_Delay(uint32_t Delay)
{
	bsp_DelayUS(Delay * 1000);
}
#endif


