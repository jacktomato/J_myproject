/****************************
*********Written by Jack********
****************************/


#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include "j_porting_os.h"
#include "j_porting_osd.h"

#define HD_OSD_MAX_LAYER 1
static j_bool g_osd_init = J_FALSE;


static struct fb_var_screeninfo fb_vinfo;
static struct fb_fix_screeninfo fb_finfo;
static char *frameBuffer_adr = 0;
static int fb_handle = 0;

struct fb_var_screeninfo *getfbvinfo()
{
    return &fb_vinfo;
}
static void printFixedInfo ()
{
	J_DEBUG_PRINT_BBBB ("Fixed screen info:\n"
			"\tid: %s\n"
			"\tsmem_start: 0x%lx\n"
			"\tsmem_len: %d\n"
			"\ttype: %d\n"
			"\ttype_aux: %d\n"
			"\tvisual: %d\n"
			"\txpanstep: %d\n"
			"\typanstep: %d\n"
			"\tywrapstep: %d\n"
			"\tline_length: %d\n"
			"\tmmio_start: 0x%lx\n"
			"\tmmio_len: %d\n"
			"\taccel: %d\n"
			"\n",
			fb_finfo.id, fb_finfo.smem_start, fb_finfo.smem_len, fb_finfo.type,
			fb_finfo.type_aux, fb_finfo.visual, fb_finfo.xpanstep, fb_finfo.ypanstep,
			fb_finfo.ywrapstep, fb_finfo.line_length, fb_finfo.mmio_start,
			fb_finfo.mmio_len, fb_finfo.accel);
}

static void printVariableInfo ()
{
	J_DEBUG_PRINT_BBBB ("Variable screen info:\n"
			"\txres: %d\n"
			"\tyres: %d\n"
			"\txres_virtual: %d\n"
			"\tyres_virtual: %d\n"
			"\tyoffset: %d\n"
			"\txoffset: %d\n"
			"\tbits_per_pixel: %d\n"
			"\tgrayscale: %d\n"
			"\tred: offset: %2d, length: %2d, msb_right: %2d\n"
			"\tgreen: offset: %2d, length: %2d, msb_right: %2d\n"
			"\tblue: offset: %2d, length: %2d, msb_right: %2d\n"
			"\ttransp: offset: %2d, length: %2d, msb_right: %2d\n"
			"\tnonstd: %d\n"
			"\tactivate: %d\n"
			"\theight: %d\n"
			"\twidth: %d\n"
			"\taccel_flags: 0x%x\n"
			"\tpixclock: %d\n"
			"\tleft_margin: %d\n"
			"\tright_margin: %d\n"
			"\tupper_margin: %d\n"
			"\tlower_margin: %d\n"
			"\thsync_len: %d\n"
			"\tvsync_len: %d\n"
			"\tsync: %d\n"
			"\tvmode: %d\n"
			"\n",
			fb_vinfo.xres, fb_vinfo.yres, fb_vinfo.xres_virtual, fb_vinfo.yres_virtual,
			fb_vinfo.xoffset, fb_vinfo.yoffset, fb_vinfo.bits_per_pixel,
			fb_vinfo.grayscale, fb_vinfo.red.offset, fb_vinfo.red.length,
			fb_vinfo.red.msb_right, fb_vinfo.green.offset, fb_vinfo.green.length,
			fb_vinfo.green.msb_right, fb_vinfo.blue.offset, fb_vinfo.blue.length,
			fb_vinfo.blue.msb_right, fb_vinfo.transp.offset, fb_vinfo.transp.length,
			fb_vinfo.transp.msb_right, fb_vinfo.nonstd, fb_vinfo.activate,
			fb_vinfo.height, fb_vinfo.width, fb_vinfo.accel_flags, fb_vinfo.pixclock,
			fb_vinfo.left_margin, fb_vinfo.right_margin, fb_vinfo.upper_margin,
			fb_vinfo.lower_margin, fb_vinfo.hsync_len, fb_vinfo.vsync_len,
			fb_vinfo.sync, fb_vinfo.vmode);
}
 
int j_porting_OSD_Init(void)
{
	const char *devfile = "/dev/fb0";

	fb_handle = open (devfile, O_RDWR);
	if (fb_handle == -1)
	{
		perror ("Error: cannot open framebuffer device");
		return  -1;
	}

	if (ioctl (fb_handle, FBIOGET_FSCREENINFO, &fb_finfo) == -1)
	{
		perror ("Error reading fixed information");
		return -2;
	}
	printFixedInfo ();
	if (ioctl (fb_handle, FBIOGET_VSCREENINFO, &fb_vinfo) == -1)
	{
		perror ("Error reading variable information");
		return -3;
	}
	//why this HD_SYS_Printf can case crash
	printVariableInfo ();
	g_osd_init = J_TRUE;

	return 0;
}

j_u32 j_porting_OSD_Open(J_OSD_SCREEN_RESOLUTION screen_reso,J_OSD_BIT_RESOLUTION bit_reso,j_u8 layer_no, j_u32* pOsd,j_u8 debug_level)
{
	J_OSD_INST_T* inst;
	j_u16 width,height;

	if(layer_no > HD_OSD_MAX_LAYER)return 0;
	if(g_osd_init == J_FALSE)
	{
		J_ERROR_PRINT("OSD driver is not init\n");
		return 0;
	}

	if(screen_reso==R_720_576)
	{
		width=720;
		height=480;//not 576
	}
	else if(screen_reso==R_1280_720)
	{
		width=1280;
		height=720;
	}
	else if(screen_reso==R_1920_1080)
	{
		width=1920;
		height=1080;
	}
	else return 0;

	inst=(J_OSD_INST_T*)j_porting_OsMalloc(sizeof(J_OSD_INST_T));
	if(inst == NULL)
	{
		J_ERROR_PRINT("[HD:HD_OSD_Open]not enough memory\n");
		return 0;
	}
	
	fb_vinfo.xres = width;
	fb_vinfo.yres = height;
	fb_vinfo.xres_virtual = 720;
	fb_vinfo.yres_virtual = 480;
	if (ioctl(fb_handle, FBIOPUT_VSCREENINFO, &fb_vinfo) == -1) {
		perror("Error setting variable information");
		return 0;
	}
  	if(ioctl(fb_handle, FBIOPAN_DISPLAY, &fb_vinfo) < 0) {                              
         	J_ERROR_PRINT("Panning display failed!\n");                                     
         	return 0;                                                                
     }
	/* Figure out the size of the screen in bytes */
	// fb_finfo.smem_len;

	/* Map the device to memory */
	frameBuffer_adr =(char *) mmap (0, fb_finfo.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED,fb_handle, 0);
	if (frameBuffer_adr == MAP_FAILED)
	{
		perror ("Error: Failed to map framebuffer device to memory");
		return 0;
	}


	inst->deviceAddr = (j_u32*)frameBuffer_adr;
	int screensize = fb_vinfo.xres * fb_vinfo.yres *4;
	memset(frameBuffer_adr,0xffff0000,screensize);
	J_DEBUG_PRINT("[HD:HD_OSD_Open]base address of OSD %#x, size\n", (unsigned int)inst->deviceAddr);
	j_porting_OsDelay(200);
	inst->screen=screen_reso;

	inst->layerNo=layer_no;
	inst->exportAddr=pOsd;
	inst->width=width;
	inst->height=height;
	inst->debug_level=debug_level;
	*pOsd=(j_u32)inst->deviceAddr;
	
	return (j_u32)inst;

}


int  j_porting_OSD_Close(j_u32 osd_handler)
{
	J_OSD_INST_T *inst;

	if(osd_handler == 0) return -1;
	inst=(J_OSD_INST_T *)osd_handler;

	*(inst->exportAddr)=(j_u32)NULL;
	j_porting_OsFree(inst);
        munmap(frameBuffer_adr, fb_finfo.smem_len);
        close(fb_handle);
	return 0;
}

j_u8* j_porting_OSD_GetOsdBaseAddr(j_u32 osd_handler)
{
	J_OSD_INST_T *inst;

	if(osd_handler == 0) return NULL;
	inst=(J_OSD_INST_T *)osd_handler;

	return (j_u8*)(inst->deviceAddr);
}


/***************************************************

color_type:
#define ARGB8888BMP    0x1
#define RGB565BMP    0x2
#define ARGB4444BMP    0x4
#define I8BMP    0x8
***************************************************/

//#include "gb_osd.h"
extern j_u32 osd_handler;

void j_porting_OSD_DrawBMP(J_OSD_RECT rec, j_u32 color_type, void * bmp_data)
{
	J_OSD_INST_T *inst;
	int stride = 720;

	if(osd_handler == 0) return ;
	inst=(J_OSD_INST_T *)osd_handler;

	//TODO:: osd screan only two 1280*720 or 720*576(480)
	if(inst->screen != R_720_576){
		stride= 1280;
	}

	unsigned int *dest = (unsigned int *) (inst->deviceAddr)
		+ (rec.m_startY + fb_vinfo.yoffset) * stride + (rec.m_startX + fb_vinfo.xoffset);
	j_u8* pSource= bmp_data;

	int x, y;
	for (y = 0; y < rec.m_sizeY; ++y){
		for (x = 0; x < rec.m_sizeX; ++x){
			dest[x] = (pSource[3]<<24)|(pSource[2]<<16)|(pSource[1]<<8)|pSource[0];
			pSource+=4;
		}
		dest += stride;
	}
		
}

void j_porting_OSD_DrawBMPBlt(J_OSD_RECT rec, j_u32 color_type, void * bmp_data)
{
	J_OSD_INST_T *inst;
	int stride = 720;

	if(osd_handler == 0) return ;
	inst=(J_OSD_INST_T *)osd_handler;

	//TODO:: osd screan only two 1280*720 or 720*576(480)
	if(inst->screen != R_720_576){
		stride= 1280;
	}

	unsigned int *dest = (unsigned int *) (inst->deviceAddr)
		+ (rec.m_startY + fb_vinfo.yoffset) * stride + (rec.m_startX + fb_vinfo.xoffset);
	j_u8* pSource= bmp_data;

	int x, y;
	for (y = 0; y < rec.m_sizeY; ++y){
		for (x = 0; x < rec.m_sizeX; ++x){
			//dest[x] = (pSource[0]<<24)|(pSource[1]<<16)|(pSource[2]<<8)|pSource[3];
			dest[x] = (pSource[3]<<24)|(pSource[2]<<16)|(pSource[1]<<8)|pSource[0];
			pSource+=4;
		}
		dest += stride;
	}
		
}


#ifdef SUPPORT_VIRTUAL_OSD
j_u32 j_porting_OSD_VirtualScreenCreate(int w, int h)
{
	J_OSD_INST_T* inst;
	inst=(J_OSD_INST_T*)j_porting_OsCalloc(sizeof(J_OSD_INST_T));
	inst->width = w;
	inst->height = h;
	inst->deviceAddr = j_porting_OsCalloc(w*h*4);
	memset(inst->deviceAddr, 0, w*h*4);
	return (j_u32) inst;
}

int j_porting_OSD_VirtualScreenDelete(j_u32 handle)
{
	J_OSD_INST_T *inst = (J_OSD_INST_T *) handle;
	if(handle == 0) return -1;
	j_porting_OsFree(inst);
	return 0;
}

/*
max screen number support: 2
every screen should be 720*480
it is tmp way for increasing speed
*/
static inline j_u32 calc_color(j_u32 c0, j_u32 c1)
{
	j_u32 dst;
	j_u8 r0,g0,b0,r1,g1,b1,r2,g2,b2;
	j_u8 a0;
	int t;
	#define calc_one(a, b,c) t = (255-a0)*b +a0*a; c=t/255;
	#define rgb(c,r,g,b) r = (c>>16)&0xff; g = (c>>8)&0xff; b=c&0xff;
	a0 = c0>>24;
	if(a0 == 0) return c1;
	if(a0 == 0xff) return c0;
	rgb(c0,r0,g0,b0);
	rgb(c1,r1,g1,b1);
	calc_one(r0, r1, r2);
	calc_one(g0, g1, g2);
	calc_one(b0, b1, b2);
	dst = 0xff000000 | (r2 << 16) | (g2 << 8) | b2;
	return dst;
}

	#define COLOR(addr, off)(addr[off])
int j_porting_OSD_VirtualScreenSync(j_u32 *virtual_handles, int v_number, j_u32 device_handle)
{
	int i, k;
	j_u32 *addr0, *addr1;
	j_u32 *device_addr; 

	if(virtual_handles == NULL || v_number == 0 || device_handle == 0) return -1;
	for(k = 0; k < v_number; k ++)
		if(virtual_handles[k] == 0) return -1;
	device_addr = ((J_OSD_INST_T*)device_handle)->deviceAddr;
	addr0 = ((J_OSD_INST_T *)virtual_handles[0])->deviceAddr;
	if(v_number == 1)
		memcpy(device_addr, addr0, 720*480*4);
	else
	{
		addr1 = ((J_OSD_INST_T *)virtual_handles[1])->deviceAddr;
		for(i = 0; i < 720*480; i ++)
			COLOR(device_addr, i) = calc_color(COLOR(addr0, i),COLOR(addr1, i)); 
	}
	return 0;
}


//#define DATA_DUMP
static void data_dump(j_s8 *data,j_u32 size)
{
	printf("jack:the data dump:\n");
	j_s8 *ptr = data;
	j_u32 index;
	for(index = 0;index < size;index++)
	{
		printf("0x%02x  ",*(ptr+index));
		if(!(index+1)%10)
			printf("\n");
	}

}
int j_porting_OSD_VirtualScreenSyncArea(j_u32 *virtual_handles, int v_number, j_u32 device_handle, J_OSD_RECT * rlist, int cnt)
{
	int i, j, k,m,n;
	int x, y;
	int off,off1;
	
	if(virtual_handles == NULL || v_number == 0 || device_handle == 0 || rlist == NULL || cnt == 0) return -1;
	if(v_number != 2) return -2;
	if(virtual_handles[0] == 0 || virtual_handles[1] == 0) return -3;

	j_u32 *device_addr; 
	j_u32 *addr0, *addr1;
	device_addr = ((J_OSD_INST_T*)device_handle)->deviceAddr;
	addr0 = ((J_OSD_INST_T *)virtual_handles[0])->deviceAddr;//graph_plane
	addr1 = ((J_OSD_INST_T *)virtual_handles[1])->deviceAddr;//still_plane
			
	for(k = 0; k < cnt; k ++){
		x = rlist[k].m_startX;
		y = rlist[k].m_startY;
		off = y*720+x;
		for(i = 0; i < rlist[k].m_sizeY; i ++, off+=720)
			for(j = 0, off1 = off; j < rlist[k].m_sizeX; j ++, off1 ++){
				COLOR(device_addr, off1) = calc_color(COLOR(addr0, off1),COLOR(addr1, off1)); 
			}
	}
	return 0;
}

#endif
