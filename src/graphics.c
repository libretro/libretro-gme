#include "graphics.h"
#include <stdlib.h>
#include <string.h>

unsigned short get_color(char r, char g, char b)
{
	unsigned short color;
	color= (r << 11) | (g << 5) | b;
	return color;
}

surface *create_surface(unsigned int width, unsigned int height, unsigned int bpp)
{
	surface *newsurf = malloc(sizeof(surface));
	if(newsurf==NULL)
		return NULL;
	newsurf->pixel_data = malloc(width*height*bpp);
	if(newsurf->pixel_data==NULL) {
		free(newsurf);
		return NULL;
	}
	memset(newsurf->pixel_data,0,width*height*bpp);
	newsurf->width = width;
	newsurf->height = height;
	newsurf->bytes_per_pixel = bpp;	
	return newsurf;
}

void free_surface(surface *surf)
{
	if(surf != NULL) {
		free(surf->pixel_data);
		free(surf);		
	}
}

void copy_surface(surface *src_surf, surface *dst_surf, int x_src, int y_src, int x_dst, int y_dst, int w, int h)
{
	int x,y;
	unsigned short pixel;
	//clamp width and height
	w = min(w,dst_surf->width-x_dst-20);
	h = min(h,dst_surf->height-y_dst-15);
	for(y=0;y<h;y++)
	{
		for(x=0;x<w;x++)
		{
			pixel = get_pixel(src_surf,x_src+x,y_src+y);
			set_pixel(dst_surf,x_dst+x,y_dst+y,pixel);
		}
	}
}

void draw_line(surface *surf, unsigned short color, int start_x, int start_y, int end_x, int end_y)
{
	if(start_x==end_x) //vertical line
	{
		for(int y=start_y;y<=end_y;y++)
		{
			set_pixel(surf,start_x,y,color);
		}
	}
	else if(start_y==end_y) // horizontal line
	{
		for(int x=start_x;x<=end_x;x++)
		{
			set_pixel(surf,x,start_y,color) = color;
		}
	}
	else 
	{
		// Bresenham line algorithm, copied from https://rosettacode.org/wiki/Bitmap/Bresenham%27s_line_algorithm#C
		int x = start_x, y = start_y;
		int dx = abs(end_x-start_x), sx = start_x<end_x ? 1 : -1;
		int dy = abs(end_y-start_y), sy = start_y<end_y ? 1 : -1; 
		int err = (dx>dy ? dx : -dy)/2, e2;
		for(;;){
			set_pixel(surf,x,y,color);
			if (x==end_x && y==end_y) break;
			e2 = err;
			if (e2 >-dx) { err -= dy; x += sx; }
			if (e2 < dy) { err += dx; y += sy; }
		}
	}
}

void draw_box(surface *surf, unsigned short color, int x0, int y0, int x1, int y1)
{
	draw_line(surf,color,x0,y0,x1,y0); //top line
	draw_line(surf,color,x0,y1,x1,y1); //bottom line	
	draw_line(surf,color,x0,y0,x0,y1); //left line
	draw_line(surf,color,x1,y0,x1,y1); //right line
}

void draw_shape(surface *surf, unsigned short color, int pos_x, int pos_y, int w, int h)
{
	for(int y=pos_y;y<(pos_y+h);y++)
	{
		for(int x=pos_x;x<(pos_x+w);x++)
		{
			set_pixel(surf,x,y,color);
		}
	}	
}

void draw_letter(surface *surf, unsigned short color, char letter, int pos_x, int pos_y)
{
	int charx = 0;
	int chary = 0;
	//calculate letter offset
	charx = (letter % 16);
	chary = (letter >> 4);
	charx *= 8;
	chary *= 8;
	for(int y=0;y<8;y++)
	{
		for(int x=0;x<8;x++)
		{
			if(is_font_pixel(charx+x,chary+y))
			{
				set_pixel(surf,pos_x+x,pos_y+y,color);
			}
		}
	}		
}

void draw_string(surface *surf, unsigned short color, char* text, int pos_x, int pos_y)
{
	int msglen = strlen(text);
	surface *temp_surface = create_surface((msglen*8),8,2);	
	for(int x=0;x<msglen;x++)
	{
		draw_letter(temp_surface,color,text[x],(x*8),0);
	}
	copy_surface(temp_surface,surf,0,0,pos_x,pos_y,(msglen*8),8);

	free_surface(temp_surface);
}

int get_string_length(char* text)
{
	int ln = strlen(text);
	return ln * 8;
	
}