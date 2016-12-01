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
            return /* 返回值 */;  
        }  
        struct fb_var_screeninfo * fb_vinfo = getfbvinfo();
        
        png_ptr = png_create_read_struct( PNG_LIBPNG_VER_STRING, 0, 0, 0 );  
        info_ptr = png_create_info_struct( png_ptr );  
  
        setjmp( png_jmpbuf(png_ptr) );   
        /* 读取PNG_BYTES_TO_CHECK个字节的数据 */  
        temp = fread( buf, 1, PNG_BYTES_TO_CHECK, fp );  
        /* 若读到的数据并没有PNG_BYTES_TO_CHECK个字节 */  
        if( temp < PNG_BYTES_TO_CHECK ) {  
                fclose(fp);  
                png_destroy_read_struct( &png_ptr, &info_ptr, 0);  
                return /* 返回值 */;  
        }  
        /* 检测数据是否为PNG的签名 */  
        temp = png_sig_cmp( (png_bytep)buf, (png_size_t)0, PNG_BYTES_TO_CHECK );  
        /* 如果不是PNG的签名，则说明该文件不是PNG文件 */  
        if( temp != 0 ) {  
                fclose(fp);  
                png_destroy_read_struct( &png_ptr, &info_ptr, 0);  
                return /* 返回值 */;  
        }  
          
        /* 复位文件指针 */  
        rewind( fp );  
        /* 开始读文件 */  
        png_init_io( png_ptr, fp );   
        /* 读取PNG图片信息 */  
        png_read_png( png_ptr, info_ptr, PNG_TRANSFORM_EXPAND, 0 );  
        /* 获取图像的色彩类型 */  
        color_type = png_get_color_type( png_ptr, info_ptr ); 
        J_ERROR_PRINT("color type is %d\n",color_type);
        /* 获取图像的宽高 */  
        w = png_get_image_width( png_ptr, info_ptr );  
        h = png_get_image_height( png_ptr, info_ptr );
        J_ERROR_PRINT("w is %d h is %d\n",w,h);
        /* 获取图像的所有行像素数据，row_pointers里边就是rgba数据 */  
        row_pointers = png_get_rows( png_ptr, info_ptr );  
        /* 根据不同的色彩类型进行相应处理 */  
        switch( color_type ) {  
        case PNG_COLOR_TYPE_RGB_ALPHA: 
                for( y=0; y<h; ++y ) {  
                        for( x=0; x<w*4; ) {  
                                /* 以下是RGBA数据，需要自己补充代码，保存RGBA数据 */  
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
        /* 其它色彩类型的图像就不读了 */  
        default:  
                fclose(fp);  
                png_destroy_read_struct( &png_ptr, &info_ptr, 0);  
                return /* 返回值 */;  
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
