#include "graphics.h"
#include <stdlib.h>
#include <string.h>

#define set_pixel(fb,x,y,color) fb[x+(y*320)] = color

unsigned short get_color(char r, char g, char b)
{
	unsigned short color;
	color= (r << 11) | (g << 5) | b;
	return color;
}

void draw_line(unsigned short *fb, unsigned short color, int start_x, int start_y, int end_x, int end_y)
{
	if(start_x==end_x) //vertical line
	{
		for(int y=start_y;y<=end_y;y++)
		{
			set_pixel(fb,start_x,y,color);
		}
	}
	else if(start_y==end_y) // horizontal line
	{
		for(int x=start_x;x<=end_x;x++)
		{
			set_pixel(fb,x,start_y,color) = color;
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
			set_pixel(fb,x,y,color);
			if (x==end_x && y==end_y) break;
			e2 = err;
			if (e2 >-dx) { err -= dy; x += sx; }
			if (e2 < dy) { err += dx; y += sy; }
		}
	}
}

void draw_box(unsigned short *fb, unsigned short color, int x0, int y0, int x1, int y1)
{
	draw_line(fb,color,x0,y0,x1,y0); //top line
	draw_line(fb,color,x0,y1,x1,y1); //bottom line	
	draw_line(fb,color,x0,y0,x0,y1); //left line
	draw_line(fb,color,x1,y0,x1,y1); //right line
}

void draw_shape(unsigned short *fb, unsigned short color, int pos_x, int pos_y, int w, int h)
{
	for(int y=pos_y;y<(pos_y+h);y++)
	{
		for(int x=pos_x;x<(pos_x+w);x++)
		{
			set_pixel(fb,x,y,color);
		}
	}	
}

void draw_letter(unsigned short *fb, unsigned short color, char letter, int pos_x, int pos_y)
{
	int charx = 0;
	int chary = 0;
	//calculate letter offset
	charx = letter % 16;
	chary = letter >> 4;
	charx *= 8;
	chary *= 8;
	unsigned short *font_data = (unsigned short *)font.pixel_data;
	for(int y=0;y<8;y++)
	{
		for(int x=0;x<8;x++)
		{
			if(font_data[((charx+x)+((chary+y)*128))]==0)
			{
				set_pixel(fb,(pos_x+x),(pos_y+y),color);
			}
		}
	}		
}

void draw_string(unsigned short *fb, unsigned short color, char* text, int pos_x, int pos_y)
{
	for(int x=0;x<strlen(text);x++)
	{
		if((pos_x+(x*8)+8)<320)
		{
			draw_letter(fb,color,text[x],pos_x+(x*8),pos_y);			
		}
	}
}

int get_string_length(char* text)
{
	int ln = strlen(text);
	return ln * 8;
	
}