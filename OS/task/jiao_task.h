#ifndef __JIAO_TASK_H__
#define __JIAO_TASK_H__

#include "jiao_list.h"
#include <stdio.h>
//#include "jiao_FIFO.h"
#include "bsp_led.h"
#include "./lcd/bsp_xpt2046_lcd.h"
#include "jiao_os.h"
//这是一些任务初始化使用的参数

//任务的名字的最大个数
#define configMAX_TASK_NAME_LEN		            ( 16 )
//最大任务数
#define MAX_TASKS		            ( 5 )
#define portINITIAL_XPSR			        ( 0x01000000 )
#define portSTART_ADDRESS_MASK				( ( StackType_t ) 0xfffffffeUL )
#define configMAX_SYSCALL_INTERRUPT_PRIORITY 	191   /* 高四位有效，即等于0xb0，或者是11 */
#define MAX_TASKS_LV					5
#define MAX_TASKLEVELS					5


//任务控制器,单个任务控制使用,保存任务初始化需要的信息
typedef struct tskTaskControlBlock
{
	volatile uint32_t    *pxTopOfStack;    /* 栈顶 */
    
    uint32_t             *pxStack;         /* 任务栈起始地址 */
	                                          /* 任务名称，字符串形式 */
	char                    pcTaskName[ configMAX_TASK_NAME_LEN ];  
} tskTCB;

//一个任务,保存任务的属性
struct TASK {
	int flags;			//记录当前任务的状态,用于任务的申请,为1表示申请了没有运行,2表示运行中
	tskTCB * tss;		//这一点记录任务的控制块
	int priority, level;
};
//一个任务运行的等级
struct TASKLEVEL {
	int running; /* 正在运行的任务的数量数量 */
	int now; /* 记录当前正在运行的任务 */
	struct TASK *tasks[MAX_TASKS_LV];
};

//任务的控制模块
struct TASKCTL {
	int now_lv; /* 现在活动中的LEVEL */
	char lv_change; /* 下次切换的时候是否需要切换LEVEL */
	struct TASKLEVEL level[MAX_TASKLEVELS];
	struct TASK tasks0[MAX_TASKS];
};

//一些变量重新定义
typedef tskTCB TCB_t;
typedef void * TaskHandle_t;
typedef void (*TaskFunction_t)( void * );
typedef uint32_t StackType_t;
typedef long BaseType_t;
//内核控制寄存器
#define portNVIC_SYSPRI2_REG				( * ( ( volatile uint32_t * ) 0xe000ed20 ) )
//优先级设置
#define configKERNEL_INTERRUPT_PRIORITY 		255   /* 高四位有效，即等于0xff，或者是15 */

#define portNVIC_PENDSV_PRI					( ( ( uint32_t ) configKERNEL_INTERRUPT_PRIORITY ) << 16UL )
#define portNVIC_SYSTICK_PRI				( ( ( uint32_t ) configKERNEL_INTERRUPT_PRIORITY ) << 24UL )
//同意中断函数的名字
#define xPortPendSVHandler   PendSV_Handler
#define xPortSysTickHandler  SysTick_Handler
#define vPortSVCHandler      SVC_Handler




/* 中断控制状态寄存器：0xe000ed04
 * Bit 28 PENDSVSET: PendSV 悬起位
 */
 //PandSV的控制器的地址
#define portNVIC_INT_CTRL_REG		( * ( ( volatile uint32_t * ) 0xe000ed04 ) )
//悬起的位置
#define portNVIC_PENDSVSET_BIT		( 1UL << 28UL )

#define portSY_FULL_READ_WRITE		( 15 )
//设置悬起位的函数,之后会调用PandSV函数
#define portYIELD()																\
{																				\
	/* 触发PendSV，产生上下文切换 */								                \
	portNVIC_INT_CTRL_REG = portNVIC_PENDSVSET_BIT;								\
	__dsb( portSY_FULL_READ_WRITE );											\
	__isb( portSY_FULL_READ_WRITE );											\
}
#define taskYIELD()			portYIELD()



#define pdFALSE			( ( BaseType_t ) 0 )
#define pdTRUE			( ( BaseType_t ) 1 )

StackType_t *pxPortInitialiseStack( StackType_t *pxTopOfStack, TaskFunction_t pxCode, void *pvParameters );

TaskHandle_t xTaskCreateStatic(	TaskFunction_t pxTaskCode,           /* 任务入口 */
					            const char * const pcName,           /* 任务名称，字符串形式 */
					            const uint32_t ulStackDepth,         /* 任务栈大小，单位为字 */
					            void * const pvParameters,           /* 任务形参 */
					            uint32_t * const puxStackBuffer,  /* 任务栈起始地址 */
					            TCB_t * const pxTaskBuffer );         /* 任务控制块指针 */
static void prvInitialiseNewTask( 	TaskFunction_t pxTaskCode,              /* 任务入口 */
									const char * const pcName,              /* 任务名称，字符串形式 */
									const uint32_t ulStackDepth,            /* 任务栈大小，单位为字 */
									void * const pvParameters,              /* 任务形参 */
									TaskHandle_t * const pxCreatedTask,     /* 任务句柄 */
									TCB_t *pxNewTCB );                     /* 任务控制块指针 */
//void prvInitialiseTaskLists( void );
BaseType_t xPortStartScheduler( void );
void vTaskSwitchContext( void );
struct TASK *task_alloc(void);
void task_switch(void);
void task_sleep(struct TASK *task);
void task_run(struct TASK *task, int level, int priority);


/* 临界区管理 */
extern void vPortEnterCritical( void );
extern void vPortExitCritical( void );
									
/*******************临界区***********************/

#ifndef portFORCE_INLINE
	#define portFORCE_INLINE __forceinline
#endif
//没有返回值,不能嵌套使用			
#define portDISABLE_INTERRUPTS()				vPortRaiseBASEPRI()
#define portENABLE_INTERRUPTS()					vPortSetBASEPRI( 0 )

//有返回值可以嵌套使用的开关中断
#define portSET_INTERRUPT_MASK_FROM_ISR()		ulPortRaiseBASEPRI()
#define portCLEAR_INTERRUPT_MASK_FROM_ISR(x)	vPortSetBASEPRI(x)
//这个是临界区
#define portENTER_CRITICAL()					vPortEnterCritical()
#define portEXIT_CRITICAL()						vPortExitCritical()

static portFORCE_INLINE void vPortRaiseBASEPRI( void )//没有返回值,不能在中断中使用
{
uint32_t ulNewBASEPRI = configMAX_SYSCALL_INTERRUPT_PRIORITY;

	__asm
	{
		/* Set BASEPRI to the max syscall priority to effect a critical
		section. */
		msr basepri, ulNewBASEPRI
		dsb
		isb
	}
}


static portFORCE_INLINE uint32_t ulPortRaiseBASEPRI( void )//有返回值,会先保存之前的屏蔽等级
{
uint32_t ulReturn, ulNewBASEPRI = configMAX_SYSCALL_INTERRUPT_PRIORITY;

	__asm
	{
		/* Set BASEPRI to the max syscall priority to effect a critical
		section. */
		mrs ulReturn, basepri
		msr basepri, ulNewBASEPRI
		dsb
		isb
	}

	return ulReturn;
}


static portFORCE_INLINE void vPortSetBASEPRI( uint32_t ulBASEPRI )
{
	__asm
	{
		/* Barrier instructions are not used as this function is only used to
		lower the BASEPRI value. */
		msr basepri, ulBASEPRI
	}
}


#if USE_TASK_MODE

void Task_main(void);
#endif

#endif
