#include <stdio.h>
#include <linux/fb.h>
#include "jtypedef.h"
#include "j_porting_os.h"
#include "j_porting_osd.h"
#include "png.h"




 j_u32 osd_handler;

#define PNG_BYTES_TO_CHECK 4  
extern struct fb_var_screeninfo *getfbvinfo();

static int load_png_image( const char *filepath, j_u32 _handler)  
{  
        FILE *fp;  
        png_structp png_ptr;  
        png_infop info_ptr;  
        png_bytep* row_pointers;  
        char buf[PNG_BYTES_TO_CHECK];  
        int w, h, x, y, temp, color_type; 
        
        J_OSD_INST_T *h_ptr = (J_OSD_INST_T*)_handler;
        j_u32 * addr = h_ptr->deviceAddr;
        
        fp = fopen( filepath, "rb" );  
        if( fp == NULL ) {
           J_ERROR_PRINT("open source file failed\n");
            return /* ����ֵ */;  
        }  
        struct fb_var_screeninfo * fb_vinfo = getfbvinfo();
        
        png_ptr = png_create_read_struct( PNG_LIBPNG_VER_STRING, 0, 0, 0 );  
        info_ptr = png_create_info_struct( png_ptr );  
  
        setjmp( png_jmpbuf(png_ptr) );   
        /* ��ȡPNG_BYTES_TO_CHECK���ֽڵ����� */  
        temp = fread( buf, 1, PNG_BYTES_TO_CHECK, fp );  
        /* �����������ݲ�û��PNG_BYTES_TO_CHECK���ֽ� */  
        if( temp < PNG_BYTES_TO_CHECK ) {  
                fclose(fp);  
                png_destroy_read_struct( &png_ptr, &info_ptr, 0);  
                return /* ����ֵ */;  
        }  
        /* ��������Ƿ�ΪPNG��ǩ�� */  
        temp = png_sig_cmp( (png_bytep)buf, (png_size_t)0, PNG_BYTES_TO_CHECK );  
        /* �������PNG��ǩ������˵�����ļ�����PNG�ļ� */  
        if( temp != 0 ) {  
                fclose(fp);  
                png_destroy_read_struct( &png_ptr, &info_ptr, 0);  
                return /* ����ֵ */;  
        }  
          
        /* ��λ�ļ�ָ�� */  
        rewind( fp );  
        /* ��ʼ���ļ� */  
        png_init_io( png_ptr, fp );   
        /* ��ȡPNGͼƬ��Ϣ */  
        png_read_png( png_ptr, info_ptr, PNG_TRANSFORM_EXPAND, 0 );  
        /* ��ȡͼ���ɫ������ */  
        color_type = png_get_color_type( png_ptr, info_ptr ); 
        J_ERROR_PRINT("color type is %d\n",color_type);
        /* ��ȡͼ��Ŀ�� */  
        w = png_get_image_width( png_ptr, info_ptr );  
        h = png_get_image_height( png_ptr, info_ptr );
        J_ERROR_PRINT("w is %d h is %d\n",w,h);
        /* ��ȡͼ����������������ݣ�row_pointers��߾���rgba���� */  
        row_pointers = png_get_rows( png_ptr, info_ptr );  
        /* ���ݲ�ͬ��ɫ�����ͽ�����Ӧ���� */  
        switch( color_type ) {  
        case PNG_COLOR_TYPE_RGB_ALPHA: 
                for( y=0; y<h; ++y ) {  
                        for( x=0; x<w*4; ) {  
                                /* ������RGBA���ݣ���Ҫ�Լ�������룬����RGBA���� */  
                                j_u8 red = row_pointers[y][x++]; // red  
                                j_u8 green  = row_pointers[y][x++]; // green  
                                j_u8 blue  = row_pointers[y][x++]; // blue  
                                j_u8 alpha  = row_pointers[y][x++]; // alpha  
                                j_u32 color = alpha<<24|red<<16|green<<8|blue;
                                *(addr+(y+50)*w+x/4) = color;

                        }  
                }  
                break;  
  
        case PNG_COLOR_TYPE_RGB:  
                for( y=0; y<h; ++y ) {  
                        for( x=0; x<w*3; ) {  
                                j_u8 red = row_pointers[y][x++]; // red  
                                j_u8 green= row_pointers[y][x++]; // green  
                                j_u8 blue= row_pointers[y][x++]; // blue
                                j_u32 color = 0;
                               color = 0xff000000|red<<16|green<<8|blue;
                                *(addr+y*w+x/3) = color;
  
                        }  
                }  
                break;  
        /* ����ɫ�����͵�ͼ��Ͳ����� */  
        default:  
                fclose(fp);  
                png_destroy_read_struct( &png_ptr, &info_ptr, 0);  
                return /* ����ֵ */;  
        }  
        fclose(fp);
        png_destroy_read_struct( &png_ptr, &info_ptr, 0);  
        return 0;  
}  

int main(int argc,char **argv)
{
    j_s32 s32Ret = -1;
    j_u32 pOsd = 0;
    j_porting_os_Init();
    
    j_porting_OSD_Init();
    osd_handler=j_porting_OSD_Open(R_720_576, GB_ARGB1555, 1, &pOsd, 0);
    if(osd_handler == 0)
        J_ERROR_PRINT("OSD_Open error \n");

#if 1
        J_OSD_INST_T *h_ptr = (J_OSD_INST_T*)osd_handler;
        j_u8 * addr = ( j_u8 * )h_ptr->deviceAddr;        
        memset(addr,0xffff0000,720*480*4);
#endif
    load_png_image("./abs_images/LOGO4.png",osd_handler);
    j_porting_OSD_Close(osd_handler);
    while(1){
        j_porting_OsDelay(2000);
    }
    return 0;
}
