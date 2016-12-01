/****************************
*********Written by Jack********
****************************/

#ifndef J_PORTING_OS_H_
#define J_PORTING_OS_H_

#include <stdio.h>
#include <stdarg.h>
//#include <termios.h>
#include <unistd.h>
#include "jtypedef.h"

typedef j_s32 j_threadID;
typedef j_s32 j_mutexID;
typedef j_s32 j_semID;
typedef j_u32 j_keyID;
typedef j_s32 j_onceID;

#define J_OS_O_RDONLY 0
#define J_OS_SEEK_SET 0
#define J_OS_O_WRONLY 1
#define SIZEOF_J_THREAD_ATTR_T	36
#define SIZEOF_J_SEM_T			16
#define __PIPE_BUF 512


typedef union
{
	j_s8 __size[SIZEOF_J_THREAD_ATTR_T];
	j_s64 __align;
} j_porting_threadAttr_t;

typedef union
{
	j_s8 __size[SIZEOF_J_SEM_T];
	j_s64 __align;
} j_porting_sem_t;

#ifdef sched_param
#undef sched_param
#endif
struct _j_sched_param_t
{
	j_s32 sched_priority;
};

typedef enum _j_porting_detachState_e
{
	J_PORTING_STATE_JOINABLE 	= 0,
	J_PORTING_STATE_DEATACHED = 1
} j_porting_detachState_e;

typedef enum _j_porting_schedPolicy_e
{
	J_PORTING_SCHED_POLICY_OTHER 	= 0,
	J_PORTING_SCHED_POLICY_FIFO  	= 1,
	J_PORTING_SCHED_POLICY_RR		= 2
} j_porting_schedPolicy_e;

typedef enum j_porting_threadPriority_e
{
    J_PORTING_THREADPRIORITY_MIN = 0,
    J_PORTING_THREADPRIORITY_NOR = 5,
    J_PORTING_THREADPRIORITY_MAX = 10
} j_porting_threadPriority_e;

typedef enum j_porting_threadStackSize_e
{
    J_PORTING_THREADSTACKSIZE_NOR = 32*1024,
    J_PORTING_THREADSTACKSIZE_LARGE = 128*1024
} j_porting_threadStackSize_e;

typedef enum _j_porting_eventType_e
{
    J_PORTING_EVENTTYPE_ANDSET=1,		//事件都满足时触发
    J_PORTING_EVENTTYPE_ORSET=2,		//事件中有一个满足时触发
    J_PORTING_EVENTTYPE_CLEARSET=4,	//事件触发之后清除标志位
    
} j_porting_eventType_e;

typedef enum
{
	J_PORTING_EVENT_MOUSE_BUTTON_PRESSED		=		1
,	J_PORTING_EVENT_MOUSE_BUTTON_RELEASED		=		2
,	J_PORTING_EVENT_MOUSE_MOVED				=		3
,	J_PORTING_EVENT_KEY_PRESSED				=		4
,	J_PORTING_EVENT_KEY_RELEASED				=		5
}j_porting_io_event_type_e;

typedef enum
{
	J_PORTING_MOUSE_BUTTON_LEFT 				= 		0
,	J_PORTING_MOUSE_BUTTON_MIDDLE 			= 		1
,	J_PORTING_MOUSE_BUTTON_RIGHT 				=		2
} j_porting_io_mouse_button_e;

typedef enum
{
    J_PORTING_OS_SYS_TYPE_LINUX_E    = 0,
    J_PORTING_OS_SYS_TYPE_ANDROID_E  = 1,
}j_porting_OsSysType_e;

typedef struct _j_porting_io_mouse_button_event_t
{
	j_porting_io_event_type_e type;
	j_porting_io_mouse_button_e button;
} j_porting_io_mouse_button_event_t;

typedef struct _j_porting_io_mouse_move_event_t
{
	j_porting_io_event_type_e type;
	j_s32 x, y;
} j_porting_io_mouse_move_event_t;

typedef struct _j_porting_io_keyboard_event_t
{
	j_porting_io_event_type_e type;
	j_u32 keyScanCode;
} j_porting_io_keyboard_event_t;

typedef union _j_porting_event_u
{
	j_porting_io_event_type_e	type;
	j_porting_io_mouse_button_event_t mouseButton;
	j_porting_io_mouse_move_event_t mouseMove;
	j_porting_io_keyboard_event_t keyboard;
} j_porting_event_u;

#define J_PORTING_WAITFOREVER (0xffffffff)
#define J_EVENT_NUM_MAX 256 			//最大支持Event个数

ret_code_e j_porting_os_Init();

ret_code_e j_porting_OsDeInit();

#ifdef J_MEMORY_DEBUG
extern int g_m;
extern int g_c;
extern int g_f;
extern int g_r;
extern int g_ff;

#define j_porting_OsFree(x)     ( J_MEMORY_DEBUG(YELLOW"free [%s] %s %d count: %d  diff: %d\n"NONE, __J_FILE__, __FUNCTION__, __LINE__, ++g_f, g_c + g_m - (++g_ff)), free(x))
#define j_porting_OsMalloc(x)     ( J_MEMORY_DEBUG(BLUE"malloc [%s] %s %d count: %d[%d]\n"NONE, __J_FILE__, __FUNCTION__, __LINE__, ++g_m, x), malloc(x))
#define j_porting_OsRealloc(x, y)     ( J_MEMORY_DEBUG(BLUE"ralloc [%s] %s %d count: %d[%d]\n"NONE, __J_FILE__, __FUNCTION__, __LINE__, ++g_r, y), realloc(x, y))
#define j_porting_OsCalloc(x, y)     ( J_MEMORY_DEBUG(PURPLE"calloc [%s] %s %d count: %d[%d] \n"NONE, __J_FILE__, __FUNCTION__, __LINE__, ++g_c, (x) * (y)), calloc(x, y))

#else
//////////////////////////////memory operation///////////////////////////////////////
/*
 * Function: j_porting_OsMalloc
 * Description: allocate memory
 * Input:
 *        size: memory size in byte to allocate
 * Output: none
 * Return: pointer to memory allocated if success, J_NULL if failed
 */
void* j_porting_OsMalloc(j_u32 size);

/*
 * Function: j_porting_OsCalloc
 * Description: allocate memory and initialize every byte with zero
 * Input:
 *        n: objects count
 *        size: every object size in byte
 * Output: none
 * Return: pointer to memory allocated if success, J_NULL if failed
 */
void* j_porting_OsCalloc(j_u32 n,j_u32 size);

/*
 * Function: evm_porting_OsRealloc
 * Description: allocate memory with some origin content
 * Input:
 *        addr: origin content
 *        size: memory size in byte to allocate
 * Output: none
 * Return: pointer to memory allocated and deallocate memory in addr if success, J_NULL and stay addr unchanged if failed
 */
void* j_porting_OsRealloc(void* addr, j_u32 size);

/*
 * Function: j_porting_OsFree
 * Description: deallocate memory
 * Input:
 *        addr: memory to deallocate
 * Output: none
 * Return: none
 */
void j_porting_OsFree(void* addr);

#endif
/*
 * Function: j_porting_OsMemCpy
 * Description:  memory copy
 * Input:
 *        addr: memory addr
 * Output: none
 * Return: none
 */
void* j_porting_OsMemCpy(void* pDestBuf, const void* pSrcBuf, j_u32 wSize );

/*
 * Function: j_porting_OsMemSet
 * Description:  memory content set value
 * Input:
 *        addr: memory addr value,size
 * Output: none
 * Return: none
 */
void* j_porting_OsMemSet(void* pDestBuf, j_u8 c, j_u32 wSize);

/*
 * Function: j_porting_OsMemCmp
 * Description:  memory compare
 * Input:
 *        addr: memory addr and size
 * Output: none
 * Return: none
 */
j_s32 j_porting_OsMemCmp(const void* pDestBuf, const void* pSrcBuf, j_u32 wSize);

void* j_porting_OsMemMove(void *dest, const void *src, j_u32 n);

//////////////////////////////////////////Debug operation/////////////////////////////////////////////////////////////
/*
 * Function:j_porting_OsPrintf
 * Description: print some information with specific format
 * Input:
 *        fmt: format
 * Output: none
 * Return: numbers of characters written
 */
j_s32 j_porting_OsPrintf(const j_s8* fmt, ...);

/*
 * Function: j_porting_OsVprintf
 * Description: print some information with specific format, use variable params
 * Input:
 *        fmt: format
 *        args: params
 * Output: none
 * Return: numbers of characters written
 */
j_s32 j_porting_OsVprintf(const j_s8* fmt, va_list args);


//////////////////////////////////////////String operation/////////////////////////////////////////////////////////////
j_s32 j_porting_OsVsprintf(j_s8 *str, const j_s8 *format, va_list ap);

j_s32 j_porting_OsVsnprintf(j_s8 *str, j_u32 size, const j_s8 *format, va_list ap);

j_s32 j_porting_OsSnprintf(j_s8 *buffer, j_u32 size, const j_s8 *format , ...);

j_s32 j_porting_OsSprintf(j_s8 *buffer, const j_s8 *format , ...);

j_s8* j_porting_OsStrDup(const j_s8* src);

j_s8* j_porting_OsStrnDup(const j_s8* src, j_u32 size);

j_u32 j_porting_OsStrLen(const j_s8* str);

j_s8* j_porting_OsStrCpy(j_s8* dest, const j_s8* src);

j_s8* j_porting_OsStrnCpy(j_s8* dest , const j_s8* src, j_u32 size);

j_s8* j_porting_OsStrStr(const j_s8* haystack, const j_s8* needle);

j_s8* j_porting_OsStrCaseStr(const j_s8* haystack, const j_s8* needle);

j_s8* j_porting_OsStrChr(const j_s8 *src, j_s32 ch);

j_s8* j_porting_OsStrrChr(const j_s8 *src, j_s32 ch);

j_s32 j_porting_OsSscanf(const j_s8 *str, const j_s8* fmt, ...);

j_s8* j_porting_OsStrCat(j_s8 *dest, const j_s8 *src);

j_s8* j_porting_OsStrnCat(j_s8 *dest, const j_s8 *src, j_u32 n);

j_s32 j_porting_OsIsSpace(j_s32 ch);

j_s32 j_porting_OsIsDigit(j_s32 ch);

j_s32 j_porting_OsStrCmp(const j_s8* s1, const j_s8* s2);

j_s32 j_porting_OsStrnCmp(const j_s8* s1, const j_s8* s2, j_u32 size);

j_s32 j_porting_OsStrCaseCmp(const j_s8* s1, const j_s8* s2);

j_s32 j_porting_OsStrnCaseCmp(const j_s8* s1, const j_s8* s2, j_u32 size);

j_s32 j_porting_OsAtoi(const j_s8* s1);

j_u32 j_porting_OsHtonl(j_u32 u32HostByteOrder);

j_s32 j_porting_OsStrtol(const char *nptr, char **endptr, int base);

//////////////////////////////////////////Thread operation/////////////////////////////////////////////////////////////
/*
 * Function: j_porting_OsCreateThreadPriv
 * Description: create a new thread
 * Input:
 *        name: name of new thread
 *        pfn: entrance function of new thread
 *        param: param  of pfn
 *        stackSize: stack size of new thread
 *        priority: priority of new thread
 * Output:
 *        pThreadID: ID of thread created
 * Return: see enum ret_code_e
 */
ret_code_e j_porting_OsCreateThreadPriv(j_threadID* pThreadID, const j_s8* name, void* (*pfn)(void*)\
       , void* param,j_s32 stackSize, j_porting_threadPriority_e priority);

/*
 * Function: j_porting_OsDestroyThread
 * Description: destory a thread
 * Input:
 *        pThreadID: ID of thread
 * Output: none
 * Return: see enum ret_code_e
 */
ret_code_e j_porting_OsDestroyThread(j_threadID threadID);

/*
 * Function: j_porting_OsThreadCreate
 * Description: create a new thread with pthread rules
 * Input:
 *        	pfn: entrance function of new thread
 *       	param: param  of pfn
 * Output:
 *        	pThreadID: ID of thread created
 * Return: see enum ret_code_e
 */
ret_code_e j_porting_OsThreadCreate(j_threadID* pThreadID, j_porting_threadAttr_t* attr, void* (*pfn)(void*)\
       , void* param);


/*
 * Function: j_porting_OsThreadJoin
 * Description: 		以阻塞的方式等待指定的线程结束。
 * Input:
 *        	pThreadID: 	线程ID标识符
 * Output:
 *        	retval: 		用户自定义指针，用于存储被等待线程返回值。
 * Return: see enum ret_code_e
 */
ret_code_e j_porting_OsThreadJoin(j_threadID pThreadID, void** retval);

/*
 * Function: j_porting_OsThreadSelf
 * Description: 		获取当前线程ID
 * Input:
 *        	无
 * Output:
 *        	retval: 		当前线程ID
 * Return: see enum ret_code_e
 */
j_threadID j_porting_OsThreadSelf();


/*
 * Function: j_porting_OsThreadCancel
 * Description: 		发送终止信号给线程
 * Input:
 *        	pThreadID: 	线程ID标识符
 * Output:
 *        	无
 * Return: see enum ret_code_e
 */
ret_code_e j_porting_OsThreadCancel(j_threadID pThreadID);


/*
 * Function: j_porting_OsThreadKeyCreate
 * Description: 		创建键值
 * Input:
 *        	key: 		键值ID
 *		*:			线程结束时回调函数
 * Output:
 *        	无
 * Return: see enum ret_code_e
 */
ret_code_e j_porting_OsThreadKeyCreate(j_keyID * key, void (*) (void *));

/*
 * Function: 			j_porting_OsThreadKeyDelete
 * Description: 		删除键值
 * Input:
 *        	key: 		键值ID
 * Output:
 *        	无
 * Return: see enum ret_code_e
 */
ret_code_e j_porting_OsThreadKeyDelete(j_keyID key);


/*
 * Function: 			j_porting_OsSetSpecific
 * Description: 		为指定键值设置线程绑定数据
 * Input:
 *        	key: 		绑定的键值
 *		pointer:		数据地址
 * Output:
 *        	无
 * Return: see enum ret_code_e
 */
 
ret_code_e j_porting_OsThreadSetSpecific(j_keyID key, void* pointer);

/*
 * Function: 			j_porting_OsGetSpecific
 * Description: 		获取指定键值的线程绑定数据
 * Input:
 *   		key: 		绑定的键值
 * Output:
 *        	无
 * Return: 			绑定的数据
 */
void* j_porting_OsThreadGetSpecific(j_keyID key);

/*
 * Function: 			j_porting_OsThreadAttrInit
 * Description: 		初始化线程属性
 * Input:
 *        	attr: 		属性结构
 * Output:
 *        	无
 * Return: 			see enum ret_code_e
 */
ret_code_e j_porting_OsThreadAttrInit(j_porting_threadAttr_t* attr);


/*
 * Function: 			j_porting_OsThreadAttrDestroy
 * Description: 		销毁线程属性
 * Input:
 *        	attr: 		属性结构
 * Output:
 *        	无
 * Return: 			see enum ret_code_e
 */
ret_code_e j_porting_OsThreadAttrDestroy(j_porting_threadAttr_t* attr);


/*
 * Function: 			evm_porting_OsThreadSetSchedParam
 * Description: 		设置线程优先级
 * Input:
 *        	attr: 		属性结构
 *		param:		优先级结构
 * Output:
 *        	无
 * Return: 			see enum ret_code_e
 */
ret_code_e j_porting_OsThreadSetSchedParam(j_porting_threadAttr_t* attr, struct _j_sched_param_t* param);

/*
 * Function: 			j_porting_OsThreadSetDetachState
 * Description: 		设置线程detach状态属性
 * Input:
 *        	attr: 		属性结构
 *		state:		状态枚举值
 * Output:
 *        	无
 * Return: 			see enum ret_code_e
 */
ret_code_e j_porting_OsThreadSetDetachState(j_porting_threadAttr_t* attr, j_porting_detachState_e state);

/*
 * Function: 			j_porting_OsThreadSetSchedPolicy
 * Description: 		设置线程调度策略
 * Input:
 *        	attr: 		属性结构
 *		policy:		优先级枚举值
 * Output:
 *        	无
 * Return: 			see enum ret_code_e
 */
ret_code_e j_porting_OsThreadSetSchedPolicy(j_porting_threadAttr_t* attr, j_porting_schedPolicy_e policy);


j_threadID j_porting_OsThreadSelf();


/*
 * Function: 			j_porting_OsGetEvent
 * Description: 		获取IO事件
 * Input:
 *        	event: 		事件结构
 * Output:
 *        	无
 * Return: 			see enum ret_code_e
 */
ret_code_e j_porting_OsGetEvent(j_porting_event_u *event);

/*
 * Function: 			j_porting_OsOSDChangesSize
 * Description: 		切换OSD的输出分辨率
 * Input:
 *        	width: 		输出宽度
 *		height:		输出高度
 * Output:
 *        	无
 * Return: 			N/A
 */
void j_porting_OsOSDChangesSize(j_s32 width, j_s32 height);

/*
 * Function: j_porting_OsDealy
 * Description: delay current thread
 * Input:
 *        ms: time in millisecond to delay
 * Output: none
 * Return: see enum ret_code_e
 */
ret_code_e j_porting_OsDelay(j_u32 ms);

/*
 * Function: j_porting_OsUSecDealy
 * Description: delay current thread
 * Input:
 *        ms: time in microsecond to delay
 * Output: none
 * Return: see enum ret_code_e
 */
ret_code_e j_porting_OsUSecDelay(j_u32 us);

/*
 * Function: j_porting_OsTickTime
 * Description: get tick time after boot up
 * Input: none
 * Output: none
 * Return: time in millisecond
 */
j_s32 j_porting_OsTickTime();


ret_code_e j_porting_OsTimeInit();

/*
 * Function: j_porting_OsSetSClock
 * Description: set current GMT time , seconds
 * Input: updateTime
 * Output: none
 * Return: if success
 */
ret_code_e j_porting_OsSetSClock(j_u32 updateTime);


/*
 * Function: j_porting_OsSetTimeZone
 * Description: set current time zone
 * Input: timeZone fixme: how many minutes ?
 * Output:
 * Return: if success
 */
ret_code_e j_porting_OsSetTimeZone(j_s32 timeZone);

/*
 * Function: j_porting_OsGetTimeZone
 * Description: get current time zone
 * Input: none
 * Output: none
 * Return: difference with GMT  (minutes)
 */
j_s32 j_porting_OsGetTimeZone();



//////////////////////////////////////////Mutex operation/////////////////////////////////////////////////////////////
/*
 * Function: j_porting_OsCreateMutex
 * Description: create a mutex
 * Input: none
 * Output:
 *        pMutexID: ID of mutex created
 * Return: see enum ret_code_e
 */
ret_code_e j_porting_OsCreateMutex(j_mutexID* pMutexID);

/*
 * Function: j_porting_OsDestroyMutex
 * Description: destroy a mutex
 * Input:
 *        mutexID: ID of mutex to destroy
 * Output: none
 * Return: see enum ret_code_e
 */
ret_code_e j_porting_OsDestroyMutex(j_mutexID mutexID);

/*
 * Function: j_porting_OsMutexLock
 * Description: lock a mutex
 * Input:
 *        mutexID: ID of mutex to lock
 * Output: none
 * Return: see enum ret_code_e
 */
ret_code_e j_porting_OsMutexLock(j_mutexID mutexID);

/*
 * Function:j_porting_OsMutexTryLock
 * Description: try lock a mutex
 * Input:
 *        mutexID: ID of mutex to lock
 * Output: none
 * Return: 
 */

ret_code_e j_porting_OsMutexTryLock(j_mutexID mutexID);

/*
 * Function: j_porting_OsMutexUnlock
 * Description: unlock a mutex
 * Input:
 *        mutexID: ID of mutex to unlock
 * Output: none
 * Return: see enum ret_code_e
 */
ret_code_e j_porting_OsMutexUnlock(j_mutexID mutexID);


//////////////////////////////////////////Semaphore operation/////////////////////////////////////////////////////////////
/*
 * Function: j_porting_OsCreateSem
 * Description: create a new semaphore
 * Input:
 *        count: semaphore count when init
 * Output:
 *        pSemID: ID of semaphore created
 * Return: see enum ret_code_e
 */
ret_code_e j_porting_OsCreateSem(j_semID* pSemID, j_u32 count);

/*
 * Function: j_porting_OsDestroySem
 * Description: destroy a semaphore
 * Input:
 *        semID: ID of semaphore to destroy
 * Output: none
 * Return: see enum ret_code_e
 */
ret_code_e j_porting_OsDestroySem(j_semID semID);

/*
 * Function: j_porting_OsSemWait
 * Description: wait on a semaphore
 * Input:
 *        semID: ID of semaphore to wait on
 *        ms: time out, always wait while this value equals -1.
 * Output: none
 * Return: see enum ret_code_e
 */
ret_code_e j_porting_OsSemWait(j_semID semID, j_u32 ms);

/*
 * Function: j_porting_OsSemPost
 * Description: notify on a semaphore
 * Input:
 *        semID: ID of semaphore to notify on
 * Output: none
 * Return: see enum ret_code_e
 */
ret_code_e j_porting_OsSemPost(j_semID semID);

ret_code_e j_porting_OsSemGetCount(j_semID semID,j_s32 *count);

//////////////////////////////////////////Message queue operation/////////////////////////////////////////////////////////////
ret_code_e j_porting_OsMsgCreat(j_u32 *MsgId , j_u32 MsgNum , j_u32 MsgSize);


ret_code_e j_porting_OsMsgSend(j_u32 MsgId , void *Msg , j_u32 MsgSize ,j_u32 Timeout);


ret_code_e j_porting_OsMsgReceive(j_u32 MsgId , void *MsgBuf ,j_u32 *MsgSize , j_u32 Timeout);


ret_code_e j_porting_OsMsgDestroy(j_u32 MsgId);

//////////////////////////////////////////Timer operation/////////////////////////////////////////////////////////////


typedef void (*j_porting_os_timer_cb)(void *);

/*
 * init timer info
 */
ret_code_e j_porting_OsTimerInit();
/*
 * Function: j_porting_OsTimerCreat
 * Description: create a timer
 * Input:
 *        pcbCallback: callback function if time expired
 *        Param: param of timer_fun if called
 *        u32Dealy: first time timer_fun is called after timer created (in ms)
 *        u32Timeout: interval of timer_fun is called (in ms), only sensitive when bRepeat is true
 *        bRepeat: if timer is cyclical or not
 * Output:
 *        TimerId: id of timer if created successfully
 * Return: see enum ret_code_e
 */
ret_code_e j_porting_OsTimerCreate(j_u32 *TimerId, j_porting_os_timer_cb pcbCallback, void *Param, j_u32 u32Delay, j_u32 u32Interval, j_bool bRepeat);

/*
 * Function: j_porting_OsTimerReset
 * Description: reset a timer to change it's interval. If it's not cyclical, the timer will been restart.
 * Input:
 *        TimerId: id of timer to reset
 *        u32Interval: interval of timer_fun is called (in ms)
 * Output: none
 * Return: see enum ret_code_e
 */
ret_code_e j_porting_OsTimerReset(j_u32  TimerId, j_u32 u32Interval);

ret_code_e j_porting_OsTimerDestroy(j_u32  TimerId);

//////////////////////////////////////////Event operation/////////////////////////////////////////////////////////////
ret_code_e j_poriting_OsEventInit();

ret_code_e j_porting_OsEventCreate(j_u32 *EventId);

ret_code_e j_porting_OsEventWait(j_u32 EventId , j_u32 Pattern , j_porting_eventType_e eEventType,j_u32 *Value,j_u32 Timeout);

ret_code_e j_porting_OsEventSet(j_u32 EventId , j_u32 Mask);

ret_code_e j_porting_OsEventClear(j_u32 EventId, j_u32 Mask);

ret_code_e j_porting_OsEventDestroy(j_u32 EventId);

ret_code_e j_porting_OsGetCwd(j_s8 *Buff, j_s32 MaxLen);

/*
	function : 	j_porting_OsReboot
	description:	reboot stb
	param:		delay,	reboot after [delay] seconds
	return value:	error ret codes for exception, otherwise J_SUCCESS
*/

ret_code_e j_porting_OsReboot(j_s32 delay);

/*
Function:        j_porting_AVPlayerPowerOff
Description:     实现真待机处理接口
Input:         NA
Output:        NA
Return:        成功返回EVM_SUCCESS，否则返回J_FAILURE。
Others:        NA
*/

ret_code_e j_porting_OsTruePowerOff(void);



/*
Function:        j_porting_AVPlayerPowerOff
Description:     实现假待机处理接口
Input:         NA
Output:        NA
Return:        成功返回J_SUCCESS，否则返回J_FAILURE。
Others:        NA
*/

ret_code_e j_porting_OsFlasePowerOff(void);




j_s32 j_porting_OsStandby(j_bool bStandby);

/*
Function:        j_porting_OsGetNetDeviceInfo
Description:     get information of network device
Input:         pu32EthernetNum ps8EthernetNameList
Output:        pu32EthernetNum ps8EthernetNameList
Return:        成功返回EVM_SUCCESS，否则返回J_FAILURE。
Others:        *pu32EthernetNum has the max num of module support,  if device num more than *pu32EthernetNum, just return *pu32EthernetNum device
*                   max len of device name is 31
*/
//ret _code_e j_porting_OsPing(j_s8 *address);
ret_code_e j_porting_OsGetNetDeviceInfo(j_u32 *pu32EthernetNum, j_s8 **ps8EthernetNameList);

ret_code_e j_porting_OsRunningDevice(j_u32 *pu32RunningNum, j_s8 **ps8RunninDeviceNameList);

ret_code_e j_porting_OsGetMac(j_s8 *ps8Eth, j_s8 *out_ps8Mac, j_s8 s8Dot);

j_s32 j_porting_OsSetMac(j_s8 *ps8Eth,j_s8 *mac);

ret_code_e j_porting_OsSetDns(j_s8 *ps8Domain, j_s8 **pps8NameServer, j_u32 u32Num);

ret_code_e j_porting_OsGetDns(j_s8 **out_pps8NameServer, j_u32 *out_pu32Num);

ret_code_e j_porting_OsGetGateway(j_s8 *ps8Eth, j_s8 **out_pps8Getway, j_u32 *pu32Num);

ret_code_e j_porting_OsSetGateway(j_s8 *ps8Eth, j_s8 **pps8Getaway, j_u32 u32Num);

ret_code_e j_porting_OsSetIp(j_s8 *ps8Eth, j_s8 *ps8Ip);

ret_code_e j_porting_OsGetIp(j_s8 *ps8Eth, j_s8 *ps8Ip);

ret_code_e j_porting_OsSetNetMask(j_s8 *ps8Eth, j_s8 *ps8Mask);

ret_code_e j_porting_OsGetNetMask(j_s8 *ps8Eth, j_s8 *ps8Mask);

ret_code_e j_porting_getNetLinkstatus(j_s8 *interface_name, j_s32 *pStatus);

ret_code_e j_porting_getIpAddrByName(const void *src, j_s8* dst, j_u32 size);

ret_code_e j_porting_OsDhcp(j_s8 *ps8Eth, j_s8 *ps8Cmd, j_s32 s32Timeout);

ret_code_e j_porting_OsExitDhcp();

j_s32 j_porting_OsOpen(const char *pathname,j_s32 flags);

j_s32 j_porting_OsClose(j_s32 fd);

j_s32 j_porting_OsRead(j_s32 fd,void *buf,j_u32 count);

j_s32 j_porting_OsWrite(j_s32 fd,const void *buf,j_u32 count);

j_u32 j_porting_OsLseek(j_s32 fildes,j_u32 offset,j_s32 whence);

j_s32 j_porting_OsFsync(j_s32 fd);

j_u64 j_porting_OsStroull(const j_s8 *nptr,j_s8 **endptr,j_s32 base);
ret_code_e j_porting_OsGetCpuId(j_u32 *CpuId);
ret_code_e j_porting_OsLancableDhcp(j_s8 *ps8Eth, j_u8 *mac,j_s32 s32Timeout);



/*
Function:        j_porting_OsChangeSystemType
Description:     设置修改系统类型标识，并重新启动
Input:         NA
Output:        NA
Return:        成功返回J_SUCCESS，否则返回J_FAILURE。
Others:        NA
*/

j_s32 j_porting_OsChangeSystemType(j_porting_OsSysType_e Type);


/*
Function:         j_porting_OsStartExtendedApp
Description:      启动扩展app
Input:         
    ps8AppName: app名称
Output:        NA
Return:        成功返回J_SUCCESS，否则返回J_FAILURE。
Others:        NA
*/
ret_code_e j_porting_OsStartExtendedApp(j_s8 *ps8AppName);


/*
Function:        j_porting_OsExecuteCmdWithRetCode
Description:      执行扩展命令
Input:         
    ps8CmdName: command名称  ps8Params: Json格式参数集
Output:        NA
Return:        成功返回J_SUCCESS，否则返回J_FAILURE。
Others:        NA
*/
ret_code_e j_porting_OsExecuteCmdWithRetCode(j_s8 *ps8CmdName, j_s8 *ps8Params);


/*
Function:        j_porting_OsExecuteCmdWithRetValue
Description:      执行扩展命令
Input:         
    ps8CmdName: command名称  ps8Params: Json格式参数集   ps8RetV: Json格式返回值  s32RetVLen: 返回值长度
Output:        NA
Return:        成功返回J_SUCCESS，否则返回J_FAILURE。
Others:         
*/
ret_code_e j_porting_OsExecuteCmdWithRetValue(j_s8 *ps8CmdName, j_s8 *ps8Params, j_s8 *ps8RetV, j_s32 s32RetVLen);

#endif /* J_PORTING_OS_H_ */
