/****************************
*********Written by Jack********
****************************/

#ifndef J_TYPEDEF_H_
#define J_TYPEDEF_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <unistd.h>
//#include "j_porting_os.h"



typedef char j_s8;
typedef unsigned char j_u8;
typedef short j_s16;
typedef unsigned short j_u16;
typedef int j_s32;
typedef unsigned int j_u32;
typedef long long j_s64;
typedef unsigned long long j_u64;
typedef float j_f32;
typedef double j_f64;
typedef int j_bool;

typedef j_u32 j_handle;

#ifndef NULL
#define J_NULL (0)
#else
#define J_NULL NULL
#endif /* NULL */

#ifndef __LINE__
#define __LINE__ (-1)
#endif /* __LINE__*/

#define J_TRUE (0==0)
#define J_FALSE (!J_TRUE)

#define J_MIN(x, y) ((x>y)?y:x)
#define J_MAX(x, y) ((x>y)?x:y)

#define J_INVALID_ID (0xffffffff)
#define J_INVALID_PID (0x1fff)

#define INLINE inline
#define REGISTER register

typedef enum ret_code_e {
	J_SUCCESS = 0,
	J_FAILURE = -1,

	J_ERR_OUTOFMEMORY = -2, /* 内存泄漏 */
	J_ERR_INVALIDPOINTER = -3, /* 无效指针 */
	J_ERR_INVALIDID = -4,

	J_ERR_TIMEOUT = -5, /* 超时 */

	J_ERR_IOERROR = -6,

	J_ERR_NOTINITED = -7, /* 未初始化 */
	J_ERR_HASINITED = -8, /* 已初始化过 */
	J_ERR_CANNTREENTRANT = -9, /* 不可重入 */

	J_ERR_IDNOTEXIST = -10,  /* 无效的ID */

	J_ERR_SRCNOTEXIST = -11,  /* 源不存在 */
	J_ERR_NOENOUGHSPACE = -12,  /* 没有足够空间 */
}ret_code_e;

typedef struct J_rect_t
{
	j_s32 x;
	j_s32 y;
	j_s32 w;
	j_s32 h;
} J_rect_t;

#ifndef __LINE__
#define __J_LINE__ (-1)
#else
#define __J_LINE__ __LINE__
#endif

#ifndef __FILE__
#define __J_FILE__ ("null")
#else
#define __J_FILE__ __FILE__
#endif

#define __J_FUNCTION__ __FUNCTION__


#define RED          "\033[0;32;31m"
#define NONE         "\033[m"
#define BLUE         "\033[0;32;34m"
#define YELLOW       "\033[01;33m"
#define GREEN        "\033[0;32;32m"
#define WHITE        "\e[0;37m"
#define PURPLE       "\e[0;35m"

typedef enum
{
    J_PRINT_LEVEL_NONE_E = -1,
    J_PRINT_LEVEL_ERROR_E,
    J_PRINT_LEVEL_WARN_E,
    J_PRINT_LEVEL_DEBUG_E,
    J_PRINT_LEVEL_INFO_E,
    J_PRINT_LEVEL_DATA_DUMP_PARAM_E, 

    J_PRINT_LEVEL_DATA_DUMP_ALL_E = 0x10,  
    J_PRINT_LEVEL_FUNCTION_ENTRANCE_E = 0x20 
} J_printLevel_e;

#if 1

#define J_ERROR_PRINT(fmt, ...)  j_porting_OsPrintf(RED "[ERROR][%s][%s][%d] [time:%u] "fmt NONE, __J_FILE__, __J_FUNCTION__, __J_LINE__,j_porting_OsTickTime(), ##__VA_ARGS__)

#define J_INFO_PRINT(fmt, ...)  do{} while(0)//evm_porting_OsPrintf(NONE "[INFO][%s][%s][%d]"fmt, __J_FILE__, __J_FUNCTION__, __J_LINE__, ##__VA_ARGS__)

#define J_DEBUG_PRINT(fmt, ...) do{} while(0)//evm_porting_OsPrintf(NONE "[DEBUG][%s][%s][%d]"fmt, __J_FILE__, __J_FUNCTION__, __J_LINE__, ##__VA_ARGS__)

//#define J_MEMORY_DEBUG    printf

#define J_DEBUG_PRINT_BBBB(fmt, ...) j_porting_OsPrintf(GREEN "[DEBUG][%s][%s][%d]"fmt NONE, __J_FILE__, __J_FUNCTION__, __J_LINE__, ##__VA_ARGS__)


#else

#define J_ERROR_PRINT(fmt, ...)  printf(RED "[ERROR][%s][%s][%d] [time:%u] "fmt NONE, __J_FILE__, __J_FUNCTION__, __J_LINE__,evm_porting_OsTickTime(), ##__VA_ARGS__)

#define J_INFO_PRINT(fmt, ...)  //j_porting_OsPrintf(NONE "[INFO][%s][%s][%d]"fmt, __J_FILE__, __J_FUNCTION__, __J_LINE__, ##__VA_ARGS__)

#define J_DEBUG_PRINT(fmt, ...) printf(NONE "[DEBUG][%s][%s][%d]"fmt, __J_FILE__, __J_FUNCTION__, __J_LINE__, ##__VA_ARGS__)

#define J_DEBUG_PRINT_BBBB(fmt, ...) printf(GREEN "[DEBUG][%s][%s][%d]"fmt, __J_FILE__, __J_FUNCTION__, __J_LINE__, ##__VA_ARGS__)


#endif


#endif /* J_TYPEDEF_H_ */

