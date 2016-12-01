/****************************
*********Written by Jack********
****************************/

#ifndef _J_PORTING_OSD_H_
#define _J_PORTING_OSD_H_
#include "jtypedef.h"

typedef struct 
{
     j_u16      m_startX;      
     j_u16      m_startY;      
     j_u16      m_sizeX;       
     j_u16      m_sizeY;       
}J_OSD_RECT;


typedef enum{  
	GB_8BPP=0,
	GB_RGB565,
	GB_ARGB1555,
	GB_ARGB0555,
	GB_ARGB8888
}J_OSD_BIT_RESOLUTION;

typedef enum{  
	GB_ONE_COLOR=0,
	GB_MULTI_COLOR,
}J_OSD_TOTAL_COLOR;

typedef	struct{
	j_u32 osd_handler;
	J_OSD_RECT draw_rect;
	j_u8* pOsd;
	j_u32 pOsd_width_len;
	J_OSD_TOTAL_COLOR  total_color;
	j_u32 color;
} J_OSD_DRAW_INFO_T;

typedef enum{  
	R_720_576=0,
	R_1280_720,
	R_1920_1080
}J_OSD_SCREEN_RESOLUTION;

typedef struct
{
	J_OSD_SCREEN_RESOLUTION screen;
	j_u8 layerNo;
	j_u32 *deviceAddr;
	j_u32 *exportAddr;
	j_u16 width;
	j_u16 height;
	j_u8 debug_level;
}J_OSD_INST_T;

int j_porting_OSD_Init(void);

j_u32 j_porting_OSD_Open(J_OSD_SCREEN_RESOLUTION screen_reso,J_OSD_BIT_RESOLUTION bit_reso,j_u8 layer_no, j_u32* pOsd,j_u8 debug_level);

int   j_porting_OSD_Close(j_u32 osd_handler);

int  j_porting_OSD_Enable(j_u32 osd_handler, j_bool enable);

int   j_porting_OSD_SetAlpha(j_u32 osd_handler,j_u32 alpha);

void  j_porting_OSD_DrawImage(J_OSD_DRAW_INFO_T osd_draw_info);

j_u8*  j_porting_OSD_GetOsdBaseAddr(j_u32 osd_handler);

void  j_porting_OSD_FlushMemory(void);

void  j_porting_OSD_FlushQueue(void);

void  j_porting_OSD_DrawRec(J_OSD_RECT rec, j_u32 color);

void  j_porting_OSD_DrawLine(int x0, int y0, int x1, int y1, j_u32 color);

void  j_porting_OSD_DrawFontBmp(unsigned char * buf, j_u32 x, j_u32 y, j_u32 width, j_u32 height, j_u32 color);

void  j_porting_OSD_DrawBMP(J_OSD_RECT rec, j_u32 color_type, void * bmp_data);

void  j_porting_OSD_DrawBMPBlt(J_OSD_RECT rec, j_u32 color_type, void * bmp_data);

j_u32  j_porting_OSD_VirtualScreenCreate(int w, int h);

int  j_porting_OSD_VirtualScreenDelete(j_u32 handle);

int  j_porting_OSD_VirtualScreenSync(j_u32 *virtual_handles, int v_number, j_u32 device_handle);

int  j_porting_OSD_VirtualScreenSyncArea(j_u32 *virtual_handles, int v_number, j_u32 device_handle, J_OSD_RECT * rlist, int cnt);

int  j_porting_OSD_Scale(unsigned long nec_w, unsigned long nec_h);

#endif

