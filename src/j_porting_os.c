/****************************
*********Written by Jack********
****************************/

#include <malloc.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/time.h>
#include <sys/times.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <fcntl.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <errno.h>
#include <sys/reboot.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <net/route.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/ip_icmp.h>
#include <linux/sockios.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/ioctl.h>
#include <sys/wait.h>  
#include <net/if.h>
#include <sys/socket.h>
#include <netdb.h>
#include "jtypedef.h"
#include "j_porting_os.h"

ret_code_e j_porting_os_Init()
{
    j_porting_OsTimeInit();
    j_poriting_OsEventInit();
    j_porting_OsTimerInit();
    return J_SUCCESS;
}

ret_code_e j_porting_OsDeInit()
{
    return J_SUCCESS;
}

//////////////////////////////memory operation///////////////////////////////////////
//int g_m = 0, g_c= 0, g_f = 0, g_r = 0, g_ff = 0, obj_c = 0, obj_d = 0;
//#ifdef EVM_MEMORY_DEBUG
//
//#else
//#define m_printf(fmt, ...)    do{}while(0)
//static void evm_porting_memroy_debug(j_s8 param)
//{
//    return ;
//    switch (param)
//    {
//        case 1:
//            g_m++;
//            if ((g_m % 10) == 0 && g_m)
//                m_printf(YELLOW"malloc num = %d"NONE"\n", g_m);
//            break;
//
//        case 2:
//            g_c++;
//            if ((g_c % 10) == 0 && g_c)
//                m_printf(YELLOW"calloc num = %d"NONE"\n", g_c);
//            break;
//
//        case 3:
//            g_r++;
//            if ((g_r % 10) == 0 && g_r)
//                m_printf(YELLOW"realloc num = %d"NONE"\n", g_r);
//            break;
//
//        case 4:
//            g_f++;
//            if ((g_f % 10) == 0 && g_f)
//                m_printf(YELLOW"free num = %d free diff = %d"NONE"\n", g_f, g_m+g_r+g_c - g_f);
//            break;
//
//        default:
//            break;
//    }
//}

#define J_PORTING_MEM_POOL_SIZE (1024)
#define J_PORTING_MMZ_MEM_SIZE (1024 * 512 * 3)
static pthread_mutex_t *spstMemoryMutex = J_NULL;
typedef struct
{
    int size;
    void* ptr;
} j_porting_memNode_t;
static int ss32MemNodeSize = 0;
static int ss32MemOperationTimes = 0;
static j_porting_memNode_t starrMemPool[J_PORTING_MEM_POOL_SIZE];

static void __memLock()
{
    if (!spstMemoryMutex)
    {
        spstMemoryMutex = calloc(1, sizeof(pthread_mutex_t));
        pthread_mutex_init(spstMemoryMutex, NULL);
    }

    pthread_mutex_lock(spstMemoryMutex);
}

static void __memUnlock()
{
    pthread_mutex_unlock(spstMemoryMutex);
}

static void __dumpMemory()
{
    ss32MemOperationTimes++;
    int total = 0;

    if (0 == (ss32MemOperationTimes % 100))
    {
        int i;

        for (i = 0; i < ss32MemNodeSize; i++)
            total += starrMemPool[i].size;

        //printf("####################mem total = %d\n", total);
    }
}

static void __addMemNode(int size, void* ptr)
{
    if (NULL == ptr)
        return;

    if (ss32MemNodeSize < J_PORTING_MEM_POOL_SIZE)
    {
        starrMemPool[ss32MemNodeSize].size = size;
        starrMemPool[ss32MemNodeSize].ptr = ptr;
        ss32MemNodeSize++;
        //__dumpMemory();
    }
    else
    {
        //printf("#############error memfull\n");
    }
}

static void __removeMemNode(void* ptr)
{
    int i;

    if (NULL == ptr)
        return;

    for (i = 0; i < ss32MemNodeSize; i++)
    {
        if (starrMemPool[i].ptr == ptr)
            break;
    }

    if (i < ss32MemNodeSize)
    {
        memmove(&starrMemPool[i], &starrMemPool[i + 1], (ss32MemNodeSize - i - 1) * sizeof(j_porting_memNode_t));
        ss32MemNodeSize--;
        (void)__dumpMemory;
        //__dumpMemory();
    }
    else
    {
        //printf("#############error ptr %s\n", (char*)ptr);
    }
}

void* j_porting_OsMalloc(j_u32 size)
{
    void *ptr = malloc(size);

    return ptr;
}

void* j_porting_OsCalloc(j_u32 n, j_u32 size)
{
    void *ptr = calloc(n, size);

    return ptr;
}

void* j_porting_OsRealloc(void* addr, j_u32 size)
{
    void *ptr = realloc(addr, size);

    return ptr;
}

void j_porting_OsFree(void* addr)
{
    free(addr);
}

void* j_porting_OsMemCpy(void* pDestBuf, const void* pSrcBuf, j_u32 wSize)
{
    return memcpy(pDestBuf, pSrcBuf, wSize);
}

void* j_porting_OsMemSet(void* pDestBuf, j_u8 c, j_u32 wSize)
{
    return memset(pDestBuf, c, wSize);
}

j_s32 j_porting_OsMemCmp(const void* pDestBuf, const void* pSrcBuf, j_u32 wSize)
{
    return memcmp(pDestBuf, pSrcBuf, wSize);
}

void* j_porting_OsMemMove(void *dest, const void *src, j_u32 n)
{
    return memmove(dest, src, n);
}

j_s32 j_porting_OsOpen(const char *pathname,j_s32 flags)
{
    return open(pathname,flags);
}

j_s32 j_porting_OsClose(j_s32 fd)
{
    return close(fd);
}

j_s32 j_porting_OsRead(j_s32 fd,void *buf,j_u32 count)
{
    return read(fd,buf,count);
}

j_s32 j_porting_OsWrite(j_s32 fd,const void *buf,j_u32 count)
{

    return write(fd,buf,count);
}

j_u32 j_porting_OsLseek(j_s32 fildes,j_u32 offset,j_s32 whence)
{
    return lseek(fildes,offset,whence);
}

j_s32 j_porting_OsFsync(j_s32 fd)
{
    return fsync(fd);
}



//////////////////////////////////////////Debug operation/////////////////////////////////////////////////////////////
j_s32 j_porting_OsPrintf(const char* fmt, ...)
{
    j_s32 s32Ret = -1;
#if 1
    va_list args;

    va_start(args, fmt);

    s32Ret = vprintf(fmt, args);

    va_end(args);
#endif
    return s32Ret;
}

j_s32 j_porting_OsSprintf(j_s8 *buffer, const j_s8 *format , ...)
{
    j_s32 s32Ret;
    va_list args;

    va_start(args, format);

    s32Ret = vsprintf(buffer, format, args);

    va_end(args);

    return s32Ret;
}


j_s32 j_porting_OsSnprintf(j_s8 *buffer, j_u32 size, const j_s8 *format , ...)
{
    int result;
    va_list args;
    va_start(args, format);
    result = vsnprintf(buffer, size, format, args);
    va_end(args);

    if (size > 0)
        buffer[size - 1] = '\0';

    return result;
}

j_s32 j_porting_OsVprintf(const char* fmt, va_list args)
{
    j_s32 s32Ret = -1;
#if 1
    s32Ret = vprintf(fmt, args);
#endif
    return s32Ret;
}

j_s32 j_porting_OsVsprintf(j_s8 *str, const j_s8 *format, va_list ap)
{
    return vsprintf(str, format, ap);
}

j_s32 j_porting_OsVsnprintf(j_s8 *str, j_u32 size, const j_s8 *format, va_list ap)
{
    j_s32 nsize = 0;
    nsize = vsnprintf(str, sizeof(str) - 1, format, ap);
    return nsize;
}

j_s32 j_porting_OsFprintf(int hFile, const char *format, ...)
{
    int s32Ret;
    va_list args;
    char s8arrBuff[1024];

    va_start(args, format);

    s32Ret = vsprintf(s8arrBuff, format, args);

    printf("%s", s8arrBuff);

    va_end(args);

    return s32Ret;
}


j_s8* j_porting_OsStrDup(const j_s8* src)
{
    return strdup(src);
}

j_u64 j_porting_OsStroull(const j_s8 *nptr,j_s8 **endptr,j_s32 base)
{
    return strtoull(nptr,endptr,base);
}

j_s8* j_porting_OsStrnDup(const j_s8* src, j_u32 size)
{
    //return strndup(src, size);
    return strdup(src);
}

j_u32 j_porting_OsStrLen(const j_s8* str)
{
    return strlen(str);
}

j_s8* j_porting_OsStrCpy(j_s8* dest, const j_s8* src)
{
    return strcpy(dest, src);
}

j_s8* j_porting_OsStrnCpy(j_s8* dest, const j_s8* src, j_u32 size)
{
    return strncpy(dest, src, size);
}

j_s8* j_porting_OsStrStr(const j_s8* haystack, const j_s8* needle)
{
    return strstr(haystack, needle);
}

j_s8* j_porting_OsStrCaseStr(const j_s8* haystack, const j_s8* needle)
{	
	extern char* strcasestr(const char*, const char*);
    return strcasestr(haystack, needle);
}

j_s8* j_porting_OsStrChr(const j_s8 *src, j_s32 ch)
{
    return strchr(src, ch);
}

j_s8* j_porting_OsStrrChr(const j_s8 *src, j_s32 ch)
{
    return strrchr(src, ch);
}

j_s32 j_porting_OsSscanf(const j_s8* str, const j_s8 *fmt, ...)
{
    j_s32 s32Ret = 0;
    va_list args;

    va_start(args, fmt);

    extern int vsscanf(const char *str, const char *format, va_list ap);
    s32Ret = vsscanf(str, fmt, args);

    va_end(args);

    return s32Ret;
}

j_s8* j_porting_OsStrCat(j_s8 *dest, const j_s8 *src)
{
    return strcat(dest, src);
}

j_s8* j_porting_OsStrnCat(j_s8 *dest, const j_s8 *src, j_u32 n)
{
    return strncat(dest, src, n);
}

j_s32 j_porting_OsIsSpace(j_s32 ch)
{
    return isspace(ch);
}

j_s32 j_porting_OsIsDigit(j_s32 ch)
{
    return isdigit(ch);
}

j_s32 j_porting_OsStrCmp(const j_s8* s1, const j_s8* s2)
{
    return strcmp(s1, s2);
}

j_s32 j_porting_OsStrnCmp(const j_s8* s1, const j_s8* s2, j_u32 size)
{
    return strncmp(s1, s2, size);
}

j_s32 j_porting_OsStrCaseCmp(const j_s8* s1, const j_s8* s2)
{
    return strcasecmp(s1, s2);
}

j_s32 j_porting_OsStrnCaseCmp(const j_s8* s1, const j_s8* s2, j_u32 size)
{
    return strncasecmp(s1, s2, size);
}

j_u32 j_porting_OsHtonl(j_u32 u32HostByteOrder)
{
    return htonl(u32HostByteOrder);
}

j_s32 j_porting_OsAtoi(const j_s8* s1)
{
	return atoi(s1);
}

j_s32 j_porting_OsStrtol(const char *nptr, char **endptr, int base)
{
	return strtol(nptr, endptr, base);
}
///////////////Thread operation////////////////////////////

ret_code_e j_porting_OsCreateThreadPriv(j_threadID* pThreadID, const char* name, void* (*pfn)(void*)\
        , void* param, j_s32 stackSize, j_porting_threadPriority_e priority)
{
    j_s32 s32Ret = J_FAILURE;
    pthread_t *pstThread;
    pthread_attr_t attr;
    struct sched_param theadParam;

    theadParam.sched_priority = priority * 10 - 5 > 0 ? priority * 10 - 5 : 1;

    pthread_attr_init(&attr);

    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    pthread_attr_setschedparam(&attr, &theadParam);

    pstThread = malloc(sizeof(pthread_t));
    if (!pstThread)
    {
        s32Ret = J_ERR_OUTOFMEMORY;
        goto end;
    }

    s32Ret = pthread_create(pstThread, &attr, pfn, (void*)param);
    if (0 == s32Ret)
    {
        *pThreadID = (j_threadID)pstThread;
    }

    pthread_attr_destroy(&attr);

end:
    return s32Ret;
}

ret_code_e j_porting_OsDestroyThread(j_threadID threadID)
{
    j_s32 s32Ret = J_FAILURE;
    pthread_t *pstThread;

    pstThread = (pthread_t *)threadID;
    if (!pstThread)
    {
        s32Ret = J_ERR_IDNOTEXIST;
        goto end;
    }
    

    s32Ret = pthread_cancel( *pstThread);
    free(pstThread);
    return J_SUCCESS;
//    return s32Ret;
end:
    return s32Ret;
}

ret_code_e j_porting_OsThreadCreate(j_threadID* pThreadID, j_porting_threadAttr_t* attr,
                                            void* (*pfn)(void*), void* param)
{
    return pthread_create((pthread_t *)pThreadID, (pthread_attr_t*)attr, pfn, param);
}

ret_code_e j_porting_OsThreadCancel(j_threadID pThreadID)
{
    return  pthread_cancel((pthread_t )pThreadID);
}

ret_code_e j_porting_OsThreadJoin(j_threadID pThreadID, void** retval)
{
    return pthread_join((pthread_t )pThreadID, retval);
}

j_threadID j_porting_OsThreadSelf()
{
    j_threadID threadId;
    
    threadId = pthread_self();;
    return threadId;
}


ret_code_e j_porting_OsDelay(j_u32 ms)
{
    return usleep(ms*1000);
}



j_u32 j_porting_OsSClock()
{
    struct timeval tv = {0};

    gettimeofday(&tv, J_NULL);

    return (tv.tv_sec);
}

j_s32 j_porting_OsGetTimeZone()
{
    struct timezone tz = {0};
    struct timeval tv = {0};

    gettimeofday(&tv, &tz);

    return (tz.tz_minuteswest / 60);
}

j_s32 j_porting_OsSetTimeZone(j_s32 time)
{
    struct timezone tz = {0};
    struct timeval tv = {0};

    gettimeofday(&tv, &tz);

    if (tv.tv_sec > (j_porting_OsGetTimeZone() - time) * 3600)
        tv.tv_sec = tv.tv_sec + ((time - j_porting_OsGetTimeZone()) * 60 * 60);
    tz.tz_minuteswest = time * 60;

    settimeofday(&tv, &tz);

    return J_SUCCESS;
}

static j_s32 ss32TickPerSecond = 100;

ret_code_e j_porting_OsTimeInit()
{
    //获取每秒的tick次数
    ss32TickPerSecond = sysconf(_SC_CLK_TCK);

    return J_SUCCESS;
}

j_s32 j_porting_OsTickTime()
{
    j_u32 ms = 0;
    struct timeval struTime = {0};

    gettimeofday(&struTime, NULL);
    ms = struTime.tv_sec * 1000 + struTime.tv_usec / 1000;

    return ms;
}
//////////////////////////////////////////Mutex operation/////////////////////////////////////////////////////////////
ret_code_e j_porting_OsCreateMutex(j_mutexID* pMutexID)
{
    j_s32 s32Ret = J_FAILURE;
    pthread_mutex_t *pstMutex = J_NULL;

    pstMutex = malloc(sizeof(pthread_mutex_t));
    if (!pstMutex)
    {
        s32Ret = J_ERR_OUTOFMEMORY;
        goto end;
    }

    s32Ret = pthread_mutex_init(pstMutex, J_NULL);
    if (0 == s32Ret)
    {
        *pMutexID = (j_mutexID)pstMutex;
    }

end:
    return s32Ret;
}

ret_code_e j_porting_OsDestroyMutex(j_mutexID mutexID)
{
    j_s32 s32Ret = J_FAILURE;
    pthread_mutex_t *pstMutex = J_NULL;

    pstMutex = (pthread_mutex_t *)mutexID;
    if (!pstMutex)
    {
        s32Ret = J_ERR_IDNOTEXIST;
        goto end;
    }

    s32Ret = pthread_mutex_destroy(pstMutex);
    free(pstMutex);

end:
    return s32Ret;
}

ret_code_e j_porting_OsMutexLock(j_mutexID mutexID)
{
    j_s32 s32Ret = J_FAILURE;
    pthread_mutex_t *pstMutex = J_NULL;

    pstMutex = (pthread_mutex_t *)mutexID;
    if (!pstMutex)
    {
        s32Ret = J_ERR_IDNOTEXIST;
        goto end;
    }

    s32Ret = pthread_mutex_lock(pstMutex);

end:
    return s32Ret;
}

ret_code_e j_porting_OsMutexUnlock(j_mutexID mutexID)
{
    j_s32 s32Ret = J_FAILURE;
    pthread_mutex_t *pstMutex = J_NULL;

    pstMutex = (pthread_mutex_t *)mutexID;
    if (!pstMutex)
    {
        s32Ret = J_ERR_IDNOTEXIST;
        goto end;
    }

    s32Ret = pthread_mutex_unlock(pstMutex);

end:
    return s32Ret;
}


ret_code_e j_porting_OsMutexTryLock(j_mutexID mutexID)
{
    j_s32 s32Ret = J_FAILURE;
    pthread_mutex_t *pstMutex = J_NULL;

    pstMutex = (pthread_mutex_t *)mutexID;
    if (!pstMutex)
    {
        s32Ret = J_ERR_IDNOTEXIST;
        goto end;
    }

    ;
    while((s32Ret = pthread_mutex_trylock(pstMutex) )!= J_SUCCESS)
    {
        printf(" pthread_mutex_trylock:%d \n",s32Ret);
        usleep(10*1000);
        if(s32Ret == 22)
        {
            break;
        }
    }
end:
    return s32Ret;
}



//////////////////////////////////////////Semaphore operation/////////////////////////////////////////////////////////////
ret_code_e j_porting_OsCreateSem(j_semID* pSemID, j_u32 count)
{
    j_s32 s32Ret = J_FAILURE;
    sem_t *pstSem = J_NULL;

    pstSem = malloc(sizeof(sem_t));
    if (!pstSem)
    {
        s32Ret = J_ERR_OUTOFMEMORY;
        goto end;
    }

    s32Ret = sem_init(pstSem, 0, count);
    if (0 == s32Ret)
    {
        *pSemID = (j_semID)pstSem;
    }

end:
    return s32Ret;
}

ret_code_e j_porting_OsDestroySem(j_semID semID)
{
    j_s32 s32Ret = J_FAILURE;
    sem_t *pstSem = J_NULL;

    pstSem = (sem_t *)semID;
    if (!pstSem)
    {
        s32Ret = J_ERR_IDNOTEXIST;
        goto end;
    }

    s32Ret = sem_destroy(pstSem);
    free(pstSem);

end:
    return s32Ret;
}

ret_code_e j_porting_OsSemWait(j_semID semID, j_u32 ms)
{
    j_s32 s32Ret = J_FAILURE;
    j_u32 waitTime = 0;
    sem_t *pstSem = J_NULL;

    pstSem = (sem_t *)semID;
    if (!pstSem)
    {
        s32Ret = J_ERR_IDNOTEXIST;
        goto end;
    }

    if (0 == sem_trywait(pstSem))
    {
        s32Ret = J_SUCCESS;
        goto end;
    }

    while (waitTime < ms)
    {
        if (0 == sem_trywait(pstSem))
        {
            s32Ret = J_SUCCESS;
            break;
        }

        waitTime += 10;
        usleep(10*1000);
    }

    if (J_SUCCESS != s32Ret)
    {
        
        s32Ret = J_ERR_TIMEOUT;
    }

end:
    return s32Ret;
}

ret_code_e j_porting_OsSemPost(j_semID semID)
{
    j_s32 s32Ret = J_FAILURE;
    sem_t *pstSem = J_NULL;

    pstSem = (sem_t *)semID;
    if (!pstSem)
    {
        s32Ret = J_ERR_IDNOTEXIST;
        goto end;
    }

    s32Ret = sem_post(pstSem);

end:
    return s32Ret;
}

ret_code_e j_porting_OsSemGetCount(j_semID semID,j_s32 *count)
{
    j_s32 s32Ret = J_FAILURE;
    sem_t *pstSem = J_NULL;

    pstSem = (sem_t *)semID;
    if (!pstSem)
    {
        s32Ret = J_ERR_IDNOTEXIST;
        goto end;
    }

    s32Ret = sem_getvalue(pstSem,count);

end:
    return s32Ret;
}

ret_code_e j_porting_OsMsgCreat(j_u32 *MsgId, j_u32 MsgNum, j_u32 MsgSize)
{
    int msgid;

    if( MsgNum >16384 )
    {
        printf( "### waring message max len is 16384 !\n");
    }

    msgid = msgget( IPC_PRIVATE, IPC_EXCL|IPC_CREAT|0660 ); //0666
    if( 0 > msgid )
    {
        printf( "### msgget failed ! msgid=%d errno=%d\n", msgid,errno );
        if( errno==ENOSPC)  //这时是无法恢复的了
        {
            perror("we are going to reboot now, because of msgget");
            //system("reboot");
        }
        return J_FAILURE;
    }

    *MsgId = msgid+1;
    /*linux 0 is valid*/
    return J_SUCCESS;
}

#define J_PORTING_MSG_TYPE (1)
typedef struct _j_porting_msgbuf_t_
{
    long mtype;
    char mtext[1];
} j_porting_msgbuf_t;

ret_code_e j_porting_OsMsgSend(j_u32 MsgId , void *Msg , j_u32 MsgSize ,j_u32 Timeout)
{
    int msgid;
    j_porting_msgbuf_t *pstMsg = NULL;

    if( MsgId <= 0 ){
        printf( "### handle error !\n" );
        return J_FAILURE;
    }

    if( J_NULL == Msg )
    {
        printf( "### msg is NULL !\n" );
        return J_FAILURE;
    }

    pstMsg = malloc(sizeof(j_porting_msgbuf_t) + MsgSize - 1);
    if (!pstMsg)
    {
        printf( "out of memory\n" );
        return J_FAILURE;
    }
    pstMsg->mtype = J_PORTING_MSG_TYPE;
    memcpy(pstMsg->mtext, Msg, MsgSize);

    msgid = MsgId-1;
    if( J_SUCCESS != msgsnd(msgid, pstMsg, MsgSize, IPC_NOWAIT) )
    {
        printf( "###msgsnd failed ! errno =%d \n",errno);
        free(pstMsg);
        return J_FAILURE;
    }
    free(pstMsg);
    return J_SUCCESS;
}

ret_code_e j_porting_OsMsgReceive(j_u32 MsgId , void *MsgBuf ,j_u32 *MsgSize , j_u32 Timeout)
{
    int ret;
    int msgid;
    struct timeval tv1;
    struct timeval tv2;
    struct timezone tz;
    j_porting_msgbuf_t *pstMsg = NULL;
    int s32MsgSize;

    if( MsgId <= 0 )
    {
        printf("handle error\n");
        return J_FAILURE;
    }

    if (!MsgBuf || !MsgSize)
    {
        printf("param error\n");
        return J_FAILURE;
    }

    pstMsg = malloc(sizeof(j_porting_msgbuf_t) + *MsgSize - 1);
    if (!pstMsg)
    {
        printf( "out of memory\n" );
        return J_FAILURE;
    }
    s32MsgSize = sizeof(j_porting_msgbuf_t) + *MsgSize - 1;

    msgid = MsgId-1;
    gettimeofday(&tv1, &tz);
    
    tv1.tv_sec += Timeout/1000;
    tv1.tv_usec +=0;
    do
    {
        gettimeofday(&tv2, &tz);
        ret = msgrcv(msgid, pstMsg, s32MsgSize, J_PORTING_MSG_TYPE, IPC_NOWAIT);
        if (J_FAILURE  == ret)
        {
            if( ENOMSG != errno)
            {
                printf("msgrcv failed ! errno=%d\n", errno );
                free(pstMsg);
                return J_FAILURE;
            }
        }
        else
        {
            memcpy(MsgBuf, pstMsg->mtext, *MsgSize);
            free(pstMsg);
            return J_SUCCESS;
        }
        if( (tv2.tv_sec>tv1.tv_sec) )
        {
            break;
        }
        else if( (tv2.tv_sec==tv1.tv_sec) && (tv2.tv_usec>=tv1.tv_usec) )
        {
            break;
        }
        usleep(1000);
    } while(1);

    free(pstMsg);
    return J_ERR_TIMEOUT;
}


ret_code_e j_porting_OsMsgDestroy(j_u32 MsgId)
{
    j_s32 s32Ret = J_FAILURE;
    j_s32 s32MsgId = MsgId-1;
    
    s32Ret= msgctl(s32MsgId, IPC_RMID, NULL);
    if(s32Ret != J_SUCCESS)
    {
        return J_FAILURE;
    }
    else
    {
        return J_SUCCESS;
    }
}

//////////////////////////////////////////Timer operation/////////////////////////////////////////////////////////////
#define J_PORTING_OS_TIMER_MAX_CNT (256)
#define J_PORTING_OS_TIMER_MAX_DELAY_TIME (100)

typedef struct _j_porting_timerObject_t_
{
    j_u32 u32Dealy;
    j_u32 u32Interval;
    j_bool bRepeat;
    j_porting_os_timer_cb pcbCallback;
    void *pPrivate;

    j_s32 s32NextInvokeTime;
    j_bool bInUse;
} j_porting_timerObject_t;

typedef struct _j_porting_timerPool_t_
{
    j_mutexID mutex;
    j_threadID thread;

    j_porting_timerObject_t stTimers[J_PORTING_OS_TIMER_MAX_CNT];
} j_porting_timerPool_t;

static j_porting_timerPool_t *spstTimerPool = J_NULL;

static void* __j_porting_OsTimerTask(void* param)
{
    j_u32 u32DelayTime;
    j_s32 i, s32CurrentTime, s32MinInvokeTime = 0;

    while (1)
    {
        j_porting_OsMutexLock(spstTimerPool->mutex);

        //计算时间最近的
        s32MinInvokeTime = 0x7fffffff;
        for (i = 0; i < J_PORTING_OS_TIMER_MAX_CNT; i++)
        {
            if (spstTimerPool->stTimers[i].bInUse)
                s32MinInvokeTime = J_MIN(s32MinInvokeTime, spstTimerPool->stTimers[i].s32NextInvokeTime);
        }

        s32CurrentTime = j_porting_OsTickTime();
        if (s32CurrentTime < s32MinInvokeTime)
        {
            u32DelayTime = s32MinInvokeTime - s32CurrentTime;
            //防止某个timer间隔太长，可能卡住后面加入的间隔短的timer
            if (u32DelayTime > J_PORTING_OS_TIMER_MAX_DELAY_TIME)
                u32DelayTime = J_PORTING_OS_TIMER_MAX_DELAY_TIME;
        }
        else
        {
            //有timer触发
            for (i = 0; i < J_PORTING_OS_TIMER_MAX_CNT; i++)
            {
                if (spstTimerPool->stTimers[i].bInUse && (s32CurrentTime >= spstTimerPool->stTimers[i].s32NextInvokeTime))
                {
                    j_porting_os_timer_cb pcbCallback = spstTimerPool->stTimers[i].pcbCallback;
                    void *pPrivate = spstTimerPool->stTimers[i].pPrivate;

                    if (spstTimerPool->stTimers[i].bRepeat)
                    {
                        spstTimerPool->stTimers[i].s32NextInvokeTime += spstTimerPool->stTimers[i].u32Interval;
                    }
                    else
                    {
                        spstTimerPool->stTimers[i].s32NextInvokeTime = 0x7fffffff;
                    }

                    j_porting_OsMutexUnlock(spstTimerPool->mutex);

                    pcbCallback(pPrivate);

                    j_porting_OsMutexLock(spstTimerPool->mutex);
                }
            }
        }
        j_porting_OsMutexUnlock(spstTimerPool->mutex);

        //等待
        if (0 != u32DelayTime)
            j_porting_OsDelay(u32DelayTime);
    }

    return J_NULL;
}

ret_code_e j_porting_OsTimerInit()
{
    j_s32 s32Ret = J_FAILURE;

    if (spstTimerPool)
    {
        J_INFO_PRINT("timer has been inited already\n");
        s32Ret = J_SUCCESS;
        goto end;
    }

    spstTimerPool = j_porting_OsCalloc(1, sizeof(j_porting_timerPool_t));
    if (!spstTimerPool)
    {
        J_ERROR_PRINT("allocate memory for spstTimerPool failed\n");
        s32Ret = J_ERR_OUTOFMEMORY;
        goto end;
    }

    s32Ret = j_porting_OsCreateMutex(&spstTimerPool->mutex);
    if (J_SUCCESS != s32Ret)
    {
        J_ERROR_PRINT("create mutex failed\n");
        goto end;
    }

    s32Ret = j_porting_OsCreateThreadPriv(&spstTimerPool->thread, "timer task", __j_porting_OsTimerTask, \
            J_NULL, J_PORTING_THREADSTACKSIZE_NOR, J_PORTING_THREADPRIORITY_MAX);
    if (J_SUCCESS != s32Ret)
    {
        J_ERROR_PRINT("create thread failed\n");
        goto end;
    }

end:
    return s32Ret;
}
 
ret_code_e j_porting_OsTimerCreate(j_u32 *TimerId, j_porting_os_timer_cb pcbCallback, void *Param, j_u32 u32Delay, j_u32 u32Interval, j_bool bRepeat)
{
    j_s32 s32Ret = J_FAILURE, i;

    if (!spstTimerPool)
    {
        printf("timer has not been inited already\n");
        goto end;
    }

    if (!TimerId)
    {
        printf("param error\n");
        goto end;
    }

    j_porting_OsMutexLock(spstTimerPool->mutex);

    for (i = 0; i < J_PORTING_OS_TIMER_MAX_CNT; i++)
    {
        if (!spstTimerPool->stTimers[i].bInUse)
        {
            break;
        }
    }

    if (i < J_PORTING_OS_TIMER_MAX_CNT)
    {
        spstTimerPool->stTimers[i].bRepeat = bRepeat;
        spstTimerPool->stTimers[i].pPrivate = Param;
        spstTimerPool->stTimers[i].pcbCallback = pcbCallback;
        spstTimerPool->stTimers[i].u32Dealy = u32Delay;

        //防止数据溢出，我们只允许这么大的timer
        if (u32Interval > 0x40000000)
            u32Interval = 0x40000000;
        spstTimerPool->stTimers[i].u32Interval = u32Interval;

        if (u32Delay > 0x40000000)
            u32Delay = 0x40000000;
        spstTimerPool->stTimers[i].s32NextInvokeTime = j_porting_OsTickTime() + u32Delay;

        spstTimerPool->stTimers[i].bInUse = J_TRUE;

        *TimerId = i;

        s32Ret= J_SUCCESS;
    }
    else
    {
        printf("timer resource is not enough\n");

        s32Ret = J_FAILURE;
    }

    j_porting_OsMutexUnlock(spstTimerPool->mutex);

end:
    return s32Ret;
}

ret_code_e j_porting_OsTimerReset(j_u32 TimerId, j_u32 u32Interval)
{
    j_s32 s32Ret = J_FAILURE;

    if (!spstTimerPool)
    {
        J_INFO_PRINT("timer has not been inited already\n");
        goto end;
    }

    if (TimerId < 0 || TimerId >= J_PORTING_OS_TIMER_MAX_CNT)
    {
        J_INFO_PRINT("timer id error\n");
        goto end;
    }

    j_porting_OsMutexLock(spstTimerPool->mutex);

    if (u32Interval > 0x40000000)
        u32Interval = 0x40000000;

    spstTimerPool->stTimers[TimerId].s32NextInvokeTime = j_porting_OsTickTime() + u32Interval;
    spstTimerPool->stTimers[TimerId].u32Interval = u32Interval;

    s32Ret = J_SUCCESS;

    j_porting_OsMutexUnlock(spstTimerPool->mutex);

end:
    return s32Ret;
}

ret_code_e j_porting_OsTimerDestroy(j_u32  TimerId)
{
    j_s32 s32Ret = J_FAILURE;

    if (!spstTimerPool)
    {
        J_INFO_PRINT("timer has not been inited already\n");
        goto end;
    }

    if (TimerId >= J_PORTING_OS_TIMER_MAX_CNT)
    {
        J_INFO_PRINT("timer id error\n");
        goto end;
    }

    j_porting_OsMutexLock(spstTimerPool->mutex);

    spstTimerPool->stTimers[TimerId].bInUse = J_FAILURE;

    s32Ret = J_SUCCESS;

    j_porting_OsMutexUnlock(spstTimerPool->mutex);

end:
    return s32Ret;
}

//////////////////////////////////////////Event operation/////////////////////////////////////////////////////////////
typedef struct _j_porting_waittask_s
{
    j_semID semID;
    j_porting_eventType_e u32Opt;      /* operation, and, or, clr */
    j_u32 u32Request;  /* requested events */
    struct _j_porting_waittask_s * pstruPrev;
    struct _j_porting_waittask_s * pstruNext;
}j_porting_waittask_s;

typedef struct _j_porting_eventObject_s
{
    j_u32  u32EventId;
    j_u32 u32Flag;
    j_mutexID mutexID;
    j_bool bDestory;
    j_porting_waittask_s struHead;
}j_porting_eventObject_s;

static j_porting_eventObject_s   gEventFlag[J_EVENT_NUM_MAX];
static j_mutexID  gEventMutex;

#define EVENT_INVALID_HANDLE 0xffffffff 

#define CHECK_EVENT_FAIL_AND_RETURN(bFlag,mutexId)  \
    if(bFlag == J_TRUE)   \
    {   \
        j_porting_OsMutexUnlock(mutexId );    \
        return J_ERR_SRCNOTEXIST; \
        }    


ret_code_e j_poriting_OsEventInit()
{
    j_u32  i;
    ret_code_e eRet =J_FAILURE;
    
    for ( i = 0; i < J_EVENT_NUM_MAX; i++ )
    {
        gEventFlag[i].u32EventId =EVENT_INVALID_HANDLE ;
    }

    eRet=j_porting_OsCreateMutex(&gEventMutex);
    if(eRet!=J_SUCCESS)
    {
        J_ERROR_PRINT("j_porting_OsCreateMutex  gEventMutex error \n");
        return J_FAILURE;
    }
    return J_SUCCESS;
}
ret_code_e j_porting_OsEventCreate(j_u32 *EventId)
{
    j_u32 i;
    ret_code_e eRet =J_FAILURE;
    
    if(EventId ==J_NULL)
    {
        J_ERROR_PRINT("invalid param  EventId null \n");
        return J_FAILURE;
    }
    j_porting_OsMutexLock( gEventMutex );
    for ( i = 0; i < J_EVENT_NUM_MAX; i++ )
    {
        /* found an unused element */
        if ( EVENT_INVALID_HANDLE == gEventFlag[i].u32EventId )
        {
            gEventFlag[i].u32EventId = i;
            gEventFlag[i].u32Flag = 0;
            gEventFlag[i].bDestory = J_FALSE;
            gEventFlag[i].struHead.pstruNext = &gEventFlag[i].struHead;
            gEventFlag[i].struHead.pstruPrev = &gEventFlag[i].struHead;
            eRet=j_porting_OsCreateMutex( &gEventFlag[i].mutexID);
            if(eRet!=J_SUCCESS)
            {
                J_ERROR_PRINT("create mutex error \n");
                j_porting_OsMutexUnlock( gEventMutex);
                return J_FAILURE;
            }
            *EventId = i;
            j_porting_OsMutexUnlock(gEventMutex );
            return J_SUCCESS;
        }
    }
    j_porting_OsMutexUnlock( gEventMutex );
    J_ERROR_PRINT("not left event to create \n");
    return J_FAILURE;
}


static void  OSFlagAddToTail( j_porting_eventObject_s *psEventObj, j_porting_waittask_s *pstruNode )
{
    if ( ( J_NULL == psEventObj) || ( J_NULL == pstruNode) )
    {
        J_ERROR_PRINT("OSFlagAddToTail invalid arguments\n");
        return;
    }
    pstruNode->pstruPrev = psEventObj->struHead.pstruPrev;
    pstruNode->pstruNext = &psEventObj->struHead;
    pstruNode->pstruPrev->pstruNext = pstruNode;
    pstruNode->pstruNext->pstruPrev = pstruNode;
    return ;
}

static void  OSFlagDelNode( j_porting_eventObject_s *psEventObj, j_porting_waittask_s *pstruNode )
{
    j_porting_waittask_s *pstruCurrTask;
    
    if ( ( J_NULL == psEventObj) || ( J_NULL == pstruNode) )
    {
        J_ERROR_PRINT("OSFlagAddToTail invalid arguments\n");
        return;
    }

    pstruCurrTask = psEventObj->struHead.pstruNext;

    while ( ( pstruCurrTask != pstruNode ) && ( pstruCurrTask != &psEventObj->struHead ) )
    {
        pstruCurrTask = pstruCurrTask->pstruNext;
    }

    if ( &psEventObj->struHead == pstruCurrTask )
    {
        J_ERROR_PRINT("OSFlagDelNode failed, not found specified node\n");
        return;
    }
    
    if(pstruCurrTask && pstruCurrTask->pstruNext)
    {
        pstruCurrTask->pstruNext->pstruPrev = pstruCurrTask->pstruPrev;
    }
    
    if(pstruCurrTask && pstruCurrTask->pstruPrev)
    {
        pstruCurrTask->pstruPrev->pstruNext = pstruCurrTask->pstruNext;
    }

    return;
}

ret_code_e j_porting_OsEventWait(j_u32 EventId , j_u32 Pattern , j_porting_eventType_e eEventType,j_u32 *Value,j_u32 Timeout)
{
    j_u32 i;
    ret_code_e eRet = 0;
    j_u32 compare;
    j_porting_waittask_s *pstruCurrTask;

    for ( i = 0; i < J_EVENT_NUM_MAX; i++ )
    {
        /* found corresponding element */
        if ( EventId == gEventFlag[i].u32EventId )
        {
            eRet = j_porting_OsMutexTryLock( gEventFlag[i].mutexID);
            if(eRet == 22)//锁已经销毁
            {
                eRet = J_ERR_IDNOTEXIST;
                break;
            }
            CHECK_EVENT_FAIL_AND_RETURN(gEventFlag[i].bDestory, gEventFlag[i].mutexID);
            /* Isolate common event flags.  */
            compare =  gEventFlag[i].u32Flag & Pattern;

            /* Determine if all of the events must be present.  */
            if (eEventType & J_PORTING_EVENTTYPE_ANDSET)
            {
                /* Yes, all events must be present.  See if the compare value is
                   the same as the requested value.  */
                compare =  (compare == Pattern);
            }

            /* Determine if the requested combination of event flags are present.  */
            if (compare)
            {

                /* Yes, necessary event flags are present.  */

                /* Copy the current event flags into the appropriate destination.  */
                *Value =  gEventFlag[i].u32Flag;

                /* Determine if consumption is required.  If so, consume the event
                   flags present in the group.  */
                if (eEventType & J_PORTING_EVENTTYPE_CLEARSET)
                {
                    gEventFlag[i].u32Flag = gEventFlag[i].u32Flag & ( ~Pattern );
                }      

                
                j_porting_OsMutexUnlock( gEventFlag[i].mutexID);
                
                return eRet;

            }
            else
            {

                /* Determine if the task requested suspension.  */
                if (Timeout)
                {
                    /* Suspension is selected.  */

                    /* allocate an element */
                    pstruCurrTask = ( j_porting_waittask_s * )malloc( sizeof( j_porting_waittask_s ) );
                    if ( J_NULL == pstruCurrTask )
                    {
                        J_ERROR_PRINT("out of memory\n");
                        return -1;
                    }

                    pstruCurrTask->u32Opt = eEventType;
                    pstruCurrTask->u32Request = Pattern;
                    j_porting_OsCreateSem( &pstruCurrTask->semID,0);

                    OSFlagAddToTail( &gEventFlag[i], pstruCurrTask );
                    /* Isolate common event flags.  */
                    compare =  gEventFlag[i].u32Flag & Pattern;

                    /* Determine if all of the events must be present.  */
                    if (eEventType & J_PORTING_EVENTTYPE_ANDSET)
                    {
                        /* Yes, all events must be present.  See if the compare value is
                           the same as the requested value.  */
                        compare =  (compare == Pattern);
                    }

                    /* Determine if the requested combination of event flags are present.  */
                    if ( compare)
                    {
                        
                    }
                    else    
                    {
                        
                        j_porting_OsMutexUnlock( gEventFlag[i].mutexID);
                        
                        if ( J_PORTING_WAITFOREVER == Timeout )
                        {
                            j_porting_OsSemWait( pstruCurrTask->semID,J_PORTING_WAITFOREVER);
                        }
                        else
                        {
                            while ( Timeout > 0 )
                            {
                                eRet = j_porting_OsSemWait( pstruCurrTask->semID,0);
                                if ( 0 == eRet )
                                {
                                    break;
                                }
                                j_porting_OsDelay(1);
                                Timeout--;
                            }

                        }
                        
                       j_porting_OsMutexLock( gEventFlag[i].mutexID);
                        
                    }

                    OSFlagDelNode( &gEventFlag[i], pstruCurrTask );

                    /* free the delteted node */
                    j_porting_OsDestroySem( pstruCurrTask->semID);
                    free( pstruCurrTask );


                    /* Copy the current event flags into the appropriate destination.  */
                    *Value =  gEventFlag[i].u32Flag;
                    /* Determine if consumption is required.  If so, consume the event
                       flags present in the group.  */
                    if (eEventType & J_PORTING_EVENTTYPE_CLEARSET)
                    {
                        gEventFlag[i].u32Flag = gEventFlag[i].u32Flag & ( ~Pattern );
                    }      
                }
                else
                {

                    /* No suspension requested.  Simply return an error status
                       and zero the retrieved events variable.  */
                    eRet = -1;
                    *Value = 0;
                }
                
                j_porting_OsMutexUnlock( gEventFlag[i].mutexID );
                
                return eRet;
            }
            
            j_porting_OsMutexUnlock( gEventFlag[i].mutexID);
            
        }
    }

    return J_FAILURE;
}

ret_code_e j_porting_OsEventSet(j_u32 EventId , j_u32 Mask)
{
    j_u32 i;
    j_porting_waittask_s *pstruCurrTask;
    j_u32 compare;
    ret_code_e eRet = J_SUCCESS ;

    for ( i = 0; i < J_EVENT_NUM_MAX; i++ )
    {
        /* found corresponding element */
        if ( EventId == gEventFlag[i].u32EventId )
        {
            /* set the flag */

            eRet = j_porting_OsMutexTryLock( gEventFlag[i].mutexID);
            if(eRet == 22)//锁已经销毁
            {
                eRet = J_ERR_IDNOTEXIST;
                break;
            }
            CHECK_EVENT_FAIL_AND_RETURN(gEventFlag[i].bDestory, gEventFlag[i].mutexID);
            gEventFlag[i].u32Flag = gEventFlag[i].u32Flag | Mask;
            pstruCurrTask = gEventFlag[i].struHead.pstruNext;
            while ( pstruCurrTask !=  &gEventFlag[i].struHead )
            {

                /* First, find the event flags in common.  */
                compare = gEventFlag[i].u32Flag & pstruCurrTask->u32Request;

                /* Second, determine if all the event flags must match.  */
                if ( pstruCurrTask->u32Opt & J_PORTING_EVENTTYPE_ANDSET )
                {
                    /* Yes, an AND condition is present.  All requested events
                       must be present.  */
                    compare = ( compare == pstruCurrTask->u32Request );
                }

                /* If compare is non-zero, the suspended task's event request is
                   satisfied.  */
                if (compare)
                {
                    /* Resume the suspended task.  */
                    j_porting_OsSemPost( pstruCurrTask->semID);
                }  

                pstruCurrTask = pstruCurrTask->pstruNext;

            }
            j_porting_OsMutexUnlock( gEventFlag[i].mutexID);
            return J_SUCCESS;
        }
    }
    return J_FAILURE;
}

ret_code_e j_porting_OsEventDestroy(j_u32 EventId)
{
    j_u32 i;
    j_porting_waittask_s *pstruCurrTask;

    for ( i = 0; i < J_EVENT_NUM_MAX; i++ )
    {
        /* found corresponding element */
        if ( EventId == gEventFlag[i].u32EventId)
        {
        
            j_porting_OsMutexLock( gEventFlag[i].mutexID);
            
            gEventFlag[i].bDestory = J_TRUE;
            pstruCurrTask = gEventFlag[i].struHead.pstruNext;
            /* Walk the chain task(s) currently suspended on the event_group.  */
            while ( pstruCurrTask != & gEventFlag[i].struHead )
            {
                /* Resume the suspended task.  */
                j_porting_OsSemPost( pstruCurrTask->semID );
                pstruCurrTask = pstruCurrTask->pstruNext;

            }
            j_porting_OsMutexUnlock( gEventFlag[i].mutexID);
            /* check if the list is empty */
            while( gEventFlag[i].struHead.pstruNext != &gEventFlag[i].struHead )
            {
                j_porting_OsDelay( 100);
            }
            j_porting_OsMutexLock( gEventFlag[i].mutexID);
            gEventFlag[i].u32Flag = 0;
            gEventFlag[i].u32EventId = EVENT_INVALID_HANDLE;
            j_porting_OsMutexUnlock( gEventFlag[i].mutexID);
            j_porting_OsDestroyMutex( gEventFlag[i].mutexID);
            return J_SUCCESS;
        }
    }
    J_ERROR_PRINT(" error  \n");
    return J_FAILURE;
}

ret_code_e j_porting_OsEventClear(j_u32 EventId, j_u32 Mask )
{
    j_u32 i;

    for ( i = 0; i < J_EVENT_NUM_MAX; i++ )
    {
        /* found corresponding element */
        if ( EventId == gEventFlag[i].u32EventId)
        {
            /* set the flag */
            j_porting_OsMutexLock( gEventFlag[i].mutexID);
            gEventFlag[i].u32Flag = gEventFlag[i].u32Flag & ( ~Mask);
            j_porting_OsMutexUnlock( gEventFlag[i].mutexID );
            return J_SUCCESS;
        }
    }

    return J_FAILURE;
}

ret_code_e j_porting_OsGetCwd(j_s8 *Buff, j_s32 MaxLen)
{
    j_s32 s32Ret = J_FAILURE;

    if(J_NULL == getcwd(Buff, MaxLen))
    {
        s32Ret = J_ERR_SRCNOTEXIST;
        goto end;
    }

    s32Ret = J_SUCCESS;
end:
    return s32Ret;
}

ret_code_e j_porting_OsTimerCreat(j_u32 *TimerId, j_porting_os_timer_cb pcbCallback, void *Param, j_u32 u32Delay, j_u32 u32Interval, j_bool bRepeat)
{
    return j_porting_OsTimerCreate(TimerId, pcbCallback, Param, u32Delay, u32Interval, bRepeat);
}

////////////////////////////network///////////////////////////////////////////

ret_code_e j_porting_OsGetNetDeviceInfo(j_u32 *pu32EthernetNum, j_s8 **ps8EthernetNameList)
{
    j_s32 s32Ret = J_FAILURE;
    j_u32 u32NetDevNum = 0;
    j_u32 u32NameLen;
    j_s8 s8arrLine[512];
    j_s8 s8arrName[128];
    FILE *hFile = J_NULL;

    if(!pu32EthernetNum || !ps8EthernetNameList)
    {
        j_porting_OsPrintf("Open /proc/net/dev failed! errno:pu32EthernetNum\n");
        goto exit;
    }

    hFile = fopen("/proc/net/dev", "r");
    if(!hFile)
    {
        j_porting_OsPrintf("Open /proc/net/dev failed! errno:%d\n", errno);
        goto exit;
    }

    fgets(s8arrLine, sizeof(s8arrLine), hFile);
    fgets(s8arrLine, sizeof(s8arrLine), hFile);

    while(fgets(s8arrLine, sizeof(s8arrLine), hFile))
    {
        memset(s8arrName, 0, sizeof(s8arrName));
        sscanf(s8arrLine, "%s", s8arrName);
        u32NameLen = strlen(s8arrName);
        if(u32NameLen <= 0)
        {
            continue;
        }
        if(':' == s8arrName[u32NameLen-1])
        {
            s8arrName[u32NameLen-1] = 0;
        }
        if(0 == strcmp(s8arrName, "lo"))
        {
            continue;
        }
        if(u32NetDevNum < *pu32EthernetNum)
        {
            j_porting_OsPrintf("\nDevice[%d]:%s\n", u32NetDevNum, s8arrName);
            //ps8EthernetNameList[u32NetDevNum] = j_porting_OsMalloc(32*sizeof(j_s8));
            u32NameLen = (u32NameLen < 31)?u32NameLen:31;
            j_porting_OsMemCpy(ps8EthernetNameList[u32NetDevNum], s8arrName, u32NameLen);
            ps8EthernetNameList[u32NetDevNum][u32NameLen] = '\0'; 
            u32NetDevNum++;
        }
        else
        {
            break;
        }
    }

    fclose(hFile);
    hFile = J_NULL;
    s32Ret = J_SUCCESS;
    j_porting_OsPrintf("\nDeviceTotalNum:%d\n", u32NetDevNum);
    *pu32EthernetNum = u32NetDevNum;
    
exit:
    return s32Ret;
}


j_s32 j_porting_OsGetMac(j_s8 *ps8Eth, j_s8 *out_ps8Mac, j_s8 s8Dot)
{
    j_s32 s32Ret = J_FAILURE;
    j_handle hSocket = -1;
    struct ifreq stIfReq;

    if (!ps8Eth || !out_ps8Mac)
    {
        printf("param is NULL\n");
        goto end;
    }

    hSocket = socket(AF_INET,SOCK_STREAM,0);
    if (hSocket < 0)
    {
        printf("get socket failed\n");
        goto end;
    }

    bzero((char *)&stIfReq, sizeof(stIfReq));
    strcpy(stIfReq.ifr_name, ps8Eth);

    s32Ret = ioctl(hSocket, SIOCGIFHWADDR, &stIfReq);
    if (J_SUCCESS != s32Ret)
    {
        printf("ioctl failed\n");
        goto end;
    }

    if (s8Dot)
    {
        sprintf(out_ps8Mac, "%02X%c%02X%c%02X%c%02X%c%02X%c%02X", stIfReq.ifr_ifru.ifru_hwaddr.sa_data[0], s8Dot, \
                stIfReq.ifr_ifru.ifru_hwaddr.sa_data[1], s8Dot, stIfReq.ifr_ifru.ifru_hwaddr.sa_data[2], s8Dot, stIfReq.ifr_ifru.ifru_hwaddr.sa_data[3], s8Dot, \
                stIfReq.ifr_ifru.ifru_hwaddr.sa_data[4], s8Dot, stIfReq.ifr_ifru.ifru_hwaddr.sa_data[5]);
    }
    else
    {
        sprintf(out_ps8Mac, "%02X%02X%02X%02X%02X%02X", stIfReq.ifr_ifru.ifru_hwaddr.sa_data[0], \
                stIfReq.ifr_ifru.ifru_hwaddr.sa_data[1],  stIfReq.ifr_ifru.ifru_hwaddr.sa_data[2],  stIfReq.ifr_ifru.ifru_hwaddr.sa_data[3], \
                stIfReq.ifr_ifru.ifru_hwaddr.sa_data[4],  stIfReq.ifr_ifru.ifru_hwaddr.sa_data[5]);
    }


    s32Ret = J_SUCCESS;

end:
    if (hSocket >= 0)
        close(hSocket);

    return s32Ret;
}

#define RESOLV_FILE     "/var/resolv.conf"
#define INET_NTOA(buffer, ip) sprintf(buffer, "%03d.%03d.%03d.%03d", ip & 0xff, (ip >> 8) & 0xff, (ip >> 16) & 0xff, (ip >> 24) & 0xff)
#define SET_SA_FAMILY(addr, family) \
    memset ((char *) &(addr), '\0', sizeof(addr));\
    addr.sa_family = (family);

#define SIN_ADDR(x) (((struct sockaddr_in *) (&(x)))->sin_addr.s_addr)

#define DEL_LEADING_ZERO(dst, src) do \
    { \
        int a, b, c, d; \
        a = b = c = d = 0; \
        sscanf(src, "%03d.%03d.%03d.%03d", &a, &b, &c, &d); \
        sprintf(dst, "%d.%d.%d.%d", a, b, c, d); \
    } \
    while (0);

ret_code_e j_porting_OsSetDns(j_s8 *ps8Domain, j_s8 **pps8NameServer, j_u32 u32Num)
{
    FILE *fp = J_NULL;
    j_s32 i;
    char s8arrIp[32];

    if (J_NULL == pps8NameServer)
    {
        return J_FAILURE;
    }

    fp = fopen(RESOLV_FILE, "w");
    if (NULL == fp)
    {
        return J_FAILURE;
    }

    if (NULL != ps8Domain)
    {
        fwrite("search ", 1, strlen("search "), fp);
        fwrite(ps8Domain, 1, strlen(ps8Domain), fp);
        fwrite("\n", 1, 1, fp);
    }

    for (i = 0; i < u32Num; i++)
    {
        DEL_LEADING_ZERO(s8arrIp, pps8NameServer[i]);
        fwrite("nameserver ", 1, strlen("nameserver "), fp);
        fwrite(s8arrIp, 1, strlen(s8arrIp), fp);
        fwrite("\n", 1, 1, fp);
    }

    fflush(fp);
    fclose(fp);

    return J_SUCCESS;
}

ret_code_e j_porting_OsGetDns(j_s8 **out_pps8NameServer, j_u32 *out_pu32Num)
{
    FILE *fp = NULL;
    char buf[128] = {0}; // 128 is enough for linux
    char tmpBuff[16] = {0};
    char *pbuf = buf;
    char *ptr = NULL, *ptr2 = NULL;

    fp = fopen(RESOLV_FILE, "r");

    if (fp == NULL)
    {
        printf("can't open RESOLV_FILE \n");
        return -1;
    }

    int i = 0;
    while (fgets(buf, sizeof(buf), fp) && i < *out_pu32Num)
    {
        if ((ptr=strstr(pbuf, "nameserver")) != NULL)
        {
            ptr += strlen("nameserver");

            while (*ptr < '0' || *ptr > '9') ptr++;
                ptr2 = ptr;

            while (((*ptr2 >= '0' && *ptr2 <= '9') || (*ptr2==0x2e)) && ((ptr2-ptr) < 16))
                ptr2++;

            if (out_pps8NameServer[i])
            {
                struct sockaddr_in addr;
                memset(tmpBuff, 0, sizeof(tmpBuff));

                memset(out_pps8NameServer[i], 0, sizeof("255.255.255.255"));
                memcpy(out_pps8NameServer[i], ptr, ptr2-ptr);

                bzero(&addr, sizeof(struct sockaddr_in));
                addr.sin_family = AF_INET;

                DEL_LEADING_ZERO(tmpBuff, out_pps8NameServer[i]);

                inet_aton(tmpBuff, &addr.sin_addr);

                INET_NTOA(out_pps8NameServer[i], addr.sin_addr.s_addr);
            }
            i++;
        }
        memset(buf, 0, sizeof(buf));

    }

    *out_pu32Num = i;

    fclose(fp);

    if (0 != i)
    {
        return J_SUCCESS;
    }
    else
    {
        return J_FAILURE;
    }
}

#define UDP_BUFFER_SIZE 8192
struct route_info
{
    struct in_addr dstAddr;
    struct in_addr srcAddr;
    struct in_addr gateWay;
    char ifName[IFNAMSIZ];
};

static ret_code_e __parseRoutes(struct nlmsghdr *nlHdr, struct route_info *rtInfo, struct in_addr *pstRoute, j_s8 *ifName)
{
    struct rtmsg *rtMsg;
    struct rtattr *rtAttr;
    j_s32 rtLen = 0;

    rtMsg = (struct rtmsg *)NLMSG_DATA(nlHdr);

    /* If the route is not for AF_INET or does not belong to main routing table
    then return. */
    if ((rtMsg->rtm_family != AF_INET) || (rtMsg->rtm_table != RT_TABLE_MAIN))
        return J_FAILURE;

    /* get the rtattr field */
    rtAttr = (struct rtattr *)RTM_RTA(rtMsg);
    rtLen = RTM_PAYLOAD(nlHdr);

    for (; RTA_OK(rtAttr,rtLen); rtAttr = RTA_NEXT(rtAttr,rtLen))
    {
        switch (rtAttr->rta_type)
        {
        case RTA_OIF:
            if_indextoname(*(unsigned *)RTA_DATA(rtAttr), rtInfo->ifName);
            break;
        case RTA_GATEWAY:
            rtInfo->gateWay.s_addr = *(u_int *)RTA_DATA(rtAttr);
            break;
        case RTA_PREFSRC:
            rtInfo->srcAddr.s_addr = *(u_int *)RTA_DATA(rtAttr);
            break;
        case RTA_DST:
            rtInfo->dstAddr.s_addr = *(u_int *)RTA_DATA(rtAttr);
            break;
        }
    }

    if (strstr((j_s8 *)inet_ntoa(rtInfo->dstAddr), "0.0.0.0"))
    {
        if(strcmp(rtInfo->ifName, ifName) == 0)
        {
            memcpy(pstRoute, &rtInfo->gateWay, sizeof(struct in_addr));

            return J_SUCCESS;
        }
    }

    return J_FAILURE;
}

static j_s32 __readNlSock(j_s32 sockFd, j_s8 *bufPtr, j_s32 seqNum, j_s32 pId)
{
    struct nlmsghdr *nlHdr;
    int readLen = 0, msgLen = 0;

    do
    {
        /* Recieve response from the kernel */
        if ((readLen = recv(sockFd, bufPtr, UDP_BUFFER_SIZE - msgLen, 0)) < 0)
        {
            perror("SOCK READ: ");
            return J_FAILURE;
        }

        nlHdr = (struct nlmsghdr *)bufPtr;

        /* Check if the header is valid */
        if ((NLMSG_OK(nlHdr, readLen) == 0) || (nlHdr->nlmsg_type == NLMSG_ERROR))
        {
            perror("Error in recieved packet");
            return J_FAILURE;
        }

        /* Check if the its the last message */
        if (nlHdr->nlmsg_type == NLMSG_DONE)
        {
            break;
        }
        else
        {
            /* Else move the pointer to buffer appropriately */
            bufPtr += readLen;
            msgLen += readLen;
        }

        /* Check if its a multi part message */
        if((nlHdr->nlmsg_flags & NLM_F_MULTI) == 0)
        {
            /* return if its not */
            break;
        }
    } while ((nlHdr->nlmsg_seq != seqNum) || (nlHdr->nlmsg_pid != pId));

    return msgLen;
}

ret_code_e j_porting_OsGetGateway(j_s8 *ps8Eth, j_s8 **out_pps8Getway, j_u32 *pu32Num)
{
    struct nlmsghdr *nlMsg;
    struct rtmsg *rtMsg;
    struct route_info rtInfo;
    char msgBuf[UDP_BUFFER_SIZE];
    struct in_addr gw;
    int gw_cnt = 0;
    int sock = 0;
    int len = 0;
    int msgSeq = 0;

    /* Create Socket */
    if ((sock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE)) < 0)
    {
        printf("[%s] socket error\n", __func__);
        return J_FAILURE;
    }
    /* Initialize the buffer */
    memset(msgBuf, 0, UDP_BUFFER_SIZE);

    /* point the header and the msg structure pointers into the buffer */
    nlMsg = (struct nlmsghdr *)msgBuf;
    rtMsg = (struct rtmsg *)NLMSG_DATA(nlMsg);

    /* Fill in the nlmsg header*/
    nlMsg->nlmsg_len  = NLMSG_LENGTH(sizeof(struct rtmsg)); // Length of message.
    nlMsg->nlmsg_type = RTM_GETROUTE; // Get the routes from kernel routing table .

    nlMsg->nlmsg_flags = NLM_F_DUMP | NLM_F_REQUEST; // The message is a request for dump.
    nlMsg->nlmsg_seq   = msgSeq++; // Sequence of the message packet.
    nlMsg->nlmsg_pid   = getpid(); // PID of process sending the request.

    /* Send the request */
    if (send(sock, nlMsg, nlMsg->nlmsg_len, 0) < 0)
    {
        printf("Write To Socket Failed...\n");
        close(sock);
        return J_FAILURE;
    }

    /* Read the response */
    if ((len = __readNlSock(sock, msgBuf, msgSeq, getpid())) < 0)
    {
        printf("Read From Socket Failed...\n");
        close(sock);
        return J_FAILURE;
    }

    // ADDED BY BOB
    /* THIS IS THE NETTSTAT -RL code I commented out the printing here and in parse routes */
    for(; NLMSG_OK(nlMsg, len); nlMsg = NLMSG_NEXT(nlMsg,len))
    {
        memset(&rtInfo, 0, sizeof(struct route_info));
        if (J_SUCCESS == __parseRoutes(nlMsg, &rtInfo, &gw, ps8Eth))
        {
            if (NULL != out_pps8Getway)
            {
                INET_NTOA(out_pps8Getway[gw_cnt], gw.s_addr);
                gw_cnt++;
                if(gw_cnt >= *pu32Num)
                {
                    break;
                }
            }
            else
                gw_cnt++;
        }
    }
    close(sock);

    *pu32Num = gw_cnt;

    return J_SUCCESS;
}

ret_code_e j_porting_OsSetGateway(j_s8 *ps8Eth, j_s8 **pps8Getaway, j_u32 u32Num)
{
    j_s32 s32Ret = J_FAILURE, i;
    j_u32 u32CurNum = 0, u32AllocateNum = 0;
    j_s8 **pps8CurGateway = J_NULL;
    int sock = 0;
    struct rtentry rt;
    char s8arrIp[32];

    s32Ret = j_porting_OsGetGateway(ps8Eth, J_NULL, &u32CurNum);
    if (J_SUCCESS != s32Ret)
    {
        printf("get gata way failed\n");
        goto end;
    }

    /* Get an internet socket for doing socket ioctls. */
    sock = socket(AF_INET, SOCK_DGRAM, 0);

    memset(&rt, 0, sizeof (rt));

    /*set Destination addr*/
    SET_SA_FAMILY (rt.rt_dst, AF_INET);
    SIN_ADDR(rt.rt_gateway) = inet_addr("0.0.0.0");

    /*set gw addr*/
    SET_SA_FAMILY (rt.rt_gateway, AF_INET);

    rt.rt_dev = ps8Eth;
    rt.rt_flags = RTF_GATEWAY;

    if (u32CurNum > 0)
    {
        pps8CurGateway = calloc(u32CurNum, sizeof(j_s8*));
        if (!pps8CurGateway)
        {
            printf("allocate memory failed\n");
            goto end;
        }

        u32AllocateNum = u32CurNum;

        for (i = 0; i < u32CurNum; i++)
        {
            pps8CurGateway[i] = calloc(32, sizeof(j_s8));
            if (!pps8CurGateway[i])
            {
                printf("allocate memory failed\n");
                goto end;
            }
        }

        s32Ret = j_porting_OsGetGateway(ps8Eth, pps8CurGateway, &u32CurNum);
        if (J_SUCCESS != s32Ret)
        {
            printf("get gata way failed\n");
            goto end;
        }

        for (i = 0; i < u32CurNum; i++)
        {
            DEL_LEADING_ZERO(s8arrIp, pps8CurGateway[i]);
            SIN_ADDR(rt.rt_gateway) = inet_addr(s8arrIp);
            s32Ret = ioctl(sock, SIOCDELRT, &rt);
            if (s32Ret < 0)
            {
                printf("ioctl failed error = %d\n", errno);
                goto end;
            }
        }
    }

    for (i = 0 ; i < u32Num; i++)
    {
        DEL_LEADING_ZERO(s8arrIp, pps8Getaway[i]);
        SIN_ADDR(rt.rt_gateway) = inet_addr(s8arrIp);
        if (ioctl(sock, SIOCADDRT, &rt) < 0)
        {
            printf("ioctl2 failed error = %d\n", errno);
            goto end;
        }
    }

    s32Ret = J_SUCCESS;

end:
    if (sock)
    {
        close(sock);
    }

    if (pps8CurGateway)
    {
        for (i = 0; i < u32AllocateNum; i++)
        {
            if (pps8CurGateway[i])
                free(pps8CurGateway[i]);
        }
        free(pps8CurGateway);
    }
    return s32Ret;
}

ret_code_e j_porting_OsSetIp(j_s8 *ps8Eth, j_s8 *ps8Ip)
{
    int s;
    struct ifreq ifr;
    struct sockaddr_in addr;
    char s8arrIp[32];

    if (NULL == ps8Eth || NULL == ps8Ip)
    {
        return J_FAILURE;
    }

    if ((s = socket(AF_INET,SOCK_STREAM,0)) < 0)
    {
        return J_FAILURE;
    }

    DEL_LEADING_ZERO(s8arrIp, ps8Ip);

    strcpy(ifr.ifr_name, ps8Eth);
    bzero(&addr, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    inet_aton(s8arrIp, &addr.sin_addr);

    memcpy((char*)&ifr.ifr_ifru.ifru_addr,(char*)&addr,sizeof(struct sockaddr_in));

    if (ioctl(s, SIOCSIFADDR, &ifr) < 0)
    {
        close(s);
        return J_FAILURE;
    }

    close(s);

    return J_SUCCESS;
}

ret_code_e j_porting_OsGetIp(j_s8 *ps8Eth, j_s8 *ps8Ip)
{
    int s;
    struct ifreq ifr;
    struct sockaddr_in * ptr;

    if (NULL == ps8Eth || NULL == ps8Ip)
    {
        return J_FAILURE;
    }

    if ((s = socket(AF_INET,SOCK_STREAM,0)) < 0)
    {
        return J_FAILURE;
    }

    bzero((char *)&ifr, sizeof(ifr));
    strcpy(ifr.ifr_name, ps8Eth);

    if(ioctl(s,SIOCGIFADDR,&ifr) < 0)
    {
        close(s);
        return J_FAILURE;
    }

    ptr = (struct sockaddr_in *)&ifr.ifr_ifru.ifru_addr;

    INET_NTOA(ps8Ip, ptr->sin_addr.s_addr);

    close(s);
    return J_SUCCESS;
}

ret_code_e j_porting_OsSetNetMask(j_s8 *ps8Eth, j_s8 *ps8Mask)
{
    int s = 0;
    struct ifreq ifr;
    struct sockaddr_in netmask_addr;
    char s8arrIp[32];

    if (NULL == ps8Eth || NULL == ps8Mask)
    {
        return J_FAILURE;
    }

    if ((s = socket(AF_INET,SOCK_STREAM,0)) < 0)
    {
        return J_FAILURE;
    }

    DEL_LEADING_ZERO(s8arrIp, ps8Mask);

    bzero((char *)&ifr, sizeof(ifr));
    strcpy(ifr.ifr_name, ps8Eth);

    bzero(&netmask_addr,sizeof(struct sockaddr_in));
    netmask_addr.sin_family = AF_INET;
    inet_aton(s8arrIp, &netmask_addr.sin_addr);

    memcpy((j_s8*)&ifr.ifr_ifru.ifru_netmask, (j_s8*)&netmask_addr, sizeof(struct sockaddr_in));

    if(ioctl(s, SIOCSIFNETMASK, &ifr) < 0)
    {
        close(s);
        return J_FAILURE;
    }

    close(s);

    return J_SUCCESS;
}


ret_code_e j_porting_OsGetNetMask(j_s8 *ps8Eth, j_s8 *ps8Mask)
{
    int s = 0;
    struct ifreq ifr;
    struct sockaddr_in *ptr;

    if (NULL == ps8Eth || NULL == ps8Mask)
    {
        return J_FAILURE;
    }

    if ((s = socket(AF_INET,SOCK_STREAM,0)) < 0)
    {
        return J_FAILURE;
    }

    bzero((char *)&ifr, sizeof(ifr));
    strcpy(ifr.ifr_name, ps8Eth);

    if (ioctl(s, SIOCGIFNETMASK, &ifr) < 0)
    {
        close(s);
        return J_FAILURE;
    }

    ptr = (struct sockaddr_in *)&ifr.ifr_ifru.ifru_netmask;

    INET_NTOA(ps8Mask, ptr->sin_addr.s_addr);

    close(s);

    return J_SUCCESS;
}

ret_code_e j_porting_getNetLinkstatus(j_s8 *interface_name, j_s32 *pStatus)
{
    if(NULL == interface_name || NULL == pStatus)
    {
        return J_FAILURE;
    }

    int sfd = -1;
    struct ifreq ifr;
    bzero((j_s8 *)&ifr, sizeof(ifr));

    sfd = socket (AF_INET,SOCK_STREAM, 0);
    if (sfd < 0)
    {
        return J_FAILURE;
    }

    strcpy(ifr.ifr_name, interface_name);
    if (ioctl (sfd, SIOCGIFFLAGS, &ifr) < 0)
    {
        close(sfd);
        return J_FAILURE;
    }

    if (((ifr.ifr_flags) & IFF_UP) && ((ifr.ifr_flags) & IFF_RUNNING))
    {
        *pStatus = 1;
    }
    else
    {
        *pStatus = 0;
    }

    close(sfd);

    return J_SUCCESS;
}

ret_code_e j_porting_getIpAddrByName(const void *src, char* dst, unsigned int size)
{
    struct addrinfo *result;
    struct addrinfo *res;
    int error;
    const char *addrString;

    if (src == J_NULL || dst == J_NULL || size == 0)
    {
        return J_FAILURE;
    }

    error = getaddrinfo(src, J_NULL, J_NULL, &result);
    if (error != 0)
    {
        printf("error in getaddrinfo: %s\n", gai_strerror(error));
        return J_FAILURE;
    }
    for (res = result; res != J_NULL; res = res->ai_next)
    {
        char buf[INET6_ADDRSTRLEN];
        addrString = inet_ntop (res->ai_family,
                                 res->ai_family == AF_INET
                                 ? (void *) &((struct sockaddr_in *) res->ai_addr)->sin_addr
                                 : (void *) &((struct sockaddr_in6 *) res->ai_addr)->sin6_addr,
                                 buf, sizeof(buf));
        if (addrString != NULL)
        {
            unsigned int copySize = size < INET6_ADDRSTRLEN? size: INET6_ADDRSTRLEN;
            strncpy(dst, addrString, copySize);
            dst[copySize] = '\0';
            break;
        }
    }

    freeaddrinfo(result);

    return J_SUCCESS;
}

ret_code_e j_porting_OsDhcp(j_s8 *ps8Eth, j_s8 *ps8Cmd, j_s32 s32Timeout)
{
    j_s32 s32Ret = J_FAILURE;
    j_s8 s8arrIpaddr[64];
    j_s8 s8Cmd[128];

    if (!ps8Eth || !ps8Cmd)
        return s32Ret;
        
    sprintf(s8Cmd, "killall -9 udhcpc");

 // printf("[%s][%d] s8Cmd : %s\n",__FUNCTION__,__LINE__,s8Cmd);
    
    system(s8Cmd);

    sprintf(s8Cmd, "udhcpc -i %s %s &", ps8Eth, ps8Cmd);

 // printf("[%s][%d] s8Cmd : %s\n",__FUNCTION__,__LINE__,s8Cmd);
    
    system(s8Cmd);

    if (s32Timeout > 0)
    {
        while (1)
        {
            usleep(500 * 1000);
            s32Ret = j_porting_OsGetIp(ps8Eth, s8arrIpaddr);
            if (J_SUCCESS == s32Ret)
            {
                s32Ret = J_SUCCESS;
                break;
            }
            else
            {
                s32Timeout -= 500;
                if (s32Timeout < 0)
                {
                    sprintf(s8Cmd, "killall -9 udhcpc");
                    system(s8Cmd);

                    break;
                }
            }
        }
    }
    else
    {
        s32Ret = J_SUCCESS;
    }

    j_porting_OsPrintf("[%s][%d] s8arrIpaddr : %s\n",__FUNCTION__,__LINE__,s8arrIpaddr);

    return s32Ret;
}


ret_code_e j_porting_OsLancableDhcp(j_s8 *ps8Eth, j_u8 *mac,j_s32 s32Timeout)
{
    j_s8 ps8Ip[12] = {0};
    j_s8 strip[64];
    j_s8 s8Cmd[128];
   
    j_s32 s32Ret = J_FAILURE;

    sprintf(s8Cmd,"ifconfig %s down",ps8Eth);

    system(s8Cmd);
    
    sprintf(s8Cmd,"ifconfig %s hw ether %02x%02x%02x%02x%02x%02x",ps8Eth,mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
        
    system(s8Cmd);
        
    sprintf(s8Cmd,"ifconfig %s up",ps8Eth);

    system(s8Cmd);
        
    sprintf(s8Cmd,"udhcpc -i %s &",ps8Eth);
    
    system(s8Cmd);
    
    if (s32Timeout > 0)
    {
        while (1)
        {
            usleep(500 * 1000);
            s32Ret = j_porting_OsGetIp(ps8Eth, ps8Ip);
            if (J_SUCCESS == s32Ret)
            {
                if (ps8Ip == strstr(ps8Ip, "192.168.100."))  //192.168.100.***的IP地址认为是不合法的
                {
                    s32Ret = J_FAILURE;
                    s32Timeout -= 500;

                    if (s32Timeout < 0)
                        break;
                }
                else
                {
                    s32Ret = J_SUCCESS;
                    break;
                }
            }
            else
            {
                s32Timeout -= 500;
                if (s32Timeout < 0)
                    break;
            }
        }
    }
    else
    {
        s32Ret = J_SUCCESS;
    }

    if(J_SUCCESS == s32Ret)
    {
        //sprintf(strip,"ifconfig %s %s netmask 255.255.255.0",ps8Eth,ps8Ip); 
       // j_porting_OsPrintf("[%s][%d] strip : %s - %s\n",__FUNCTION__,__LINE__,&strip[0],strip);
       // __j_Os_system(strip);

    }

    return s32Ret;

}
    

ret_code_e j_porting_OsExitDhcp()
{
    j_s8 s8Cmd[128];

    sprintf(s8Cmd, "killall -9 udhcpc");
    system(s8Cmd);

    return J_SUCCESS;
}
ret_code_e j_porting_OsRunningDevice(j_u32 *pu32RunningNum, j_s8 **ps8RunninDeviceNameList)
{
    j_s32 s32Ret = J_FAILURE;
    j_s32 u32Index;
    j_u32 Count = 0;
    j_s32 linkStatus = 0;
    //static j_s32 sLastLinkStatus = 1;
    j_s8 *ps8arrEthernetName[3];
    j_u32 u32EthernetNum = 3;
    j_s8 s8arrName[3][32];

    for(u32Index = 0; u32Index < 3; u32Index++)
    {
        ps8arrEthernetName[u32Index] = s8arrName[u32Index];
    }

    if(!ps8RunninDeviceNameList || !pu32RunningNum)
    {
        printf("parameter error!\n");
        goto exit;
    }

    if(J_SUCCESS != j_porting_OsGetNetDeviceInfo(&u32EthernetNum, ps8arrEthernetName))
    {
        printf("j_porting_OsGetNetDeviceInfo failure!\n");
    	goto exit;
    }

    for(u32Index = 0; u32Index < u32EthernetNum; u32Index++)
    {
    	linkStatus = 0;
        if(J_SUCCESS != j_porting_getNetLinkstatus(ps8arrEthernetName[u32Index], &linkStatus))
    	{
            printf("j_porting_getNetLinkstatus failure!\n");
    		goto exit;
    	}
    	else
    	{
    		if(1 == linkStatus)
    		{
    		memset(ps8RunninDeviceNameList[Count], '\0', sizeof(ps8RunninDeviceNameList[Count]));
    		memcpy(ps8RunninDeviceNameList[Count], ps8arrEthernetName[u32Index], sizeof(ps8arrEthernetName[u32Index]));
    		Count++;
    		}
    	}

    }

    *pu32RunningNum = Count;
    s32Ret = J_SUCCESS;

    exit:
        return s32Ret;
}
