#ifndef GME_LIBRETRO_GRAPHICS_H__
#define GME_LIBRETRO_GRAPHICS_H__

#include <stdint.h>
#include <boolean.h>

#define set_pixel(surf,x,y,color) ((unsigned short *)surf->pixel_data)[(x)+((y)*surf->width)] = color
#define get_pixel(surf,x,y) ((unsigned short *)surf->pixel_data)[(x)+((y)*surf->width)]

typedef struct
{
   unsigned int 	 width;
   unsigned int 	 height;
   unsigned int 	 bytes_per_pixel; /* 2:RGB16, 3:RGB, 4:RGBA */ 
   char  		   *pixel_data;
} surface;

typedef struct {
	int x0;
	int y0;
	int x1;
	int y1;
} box;

#define gme_black   0x0000
#define gme_white   0xFFFF
#define gme_gray    0x7BEF
#define gme_red     0xF800
#define gme_orange  0xFBE0
#define gme_yellow  0xFFE0
#define gme_green   0x07E0
#define gme_blue    0x001F
#define gme_indigo  0x4810
#define gme_violet  0x901A

extern const short gme_rainbow7[7];

bool is_font_pixel(unsigned char letter, int x, int y);
unsigned short get_color(char r, char g, char b);
surface *create_surface(unsigned int width, unsigned int height, unsigned int bpp);
void free_surface(surface *surf);
surface *clip_surface(surface *src_surf, int x_src, int y_src, box clip_box);
void copy_surface(surface *src_surf, surface *dst_surf, int x_src, int y_src, int x_dst, int y_dst, int w, int h);
void draw_line(surface *surf, unsigned short color, int start_x, int start_y, int end_x, int end_y);
void draw_box(surface *surf, unsigned short color, box b);
void draw_shape(surface *surf, unsigned short color, int pos_x, int pos_y, int w, int h);
void draw_letter(surface *surf, unsigned short color, char letter, int pos_x, int pos_y);
void draw_string(surface *surf, unsigned short color, char* text, int pos_x, int pos_y, unsigned int framecounter, box clip_box);
int get_string_length(char* text);

#endif
