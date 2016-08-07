#include "graphics.h"
#include <string.h>

unsigned short get_color(char r, char g, char b)
{
	unsigned short color;
	color= (r << 16) | (g << 8) | b;
	return color;
}

void draw_shape(unsigned short *fb, unsigned short color, int pos_x, int pos_y, int w, int h)
{
	for(int y=pos_y;y<(pos_y+h);y++)
	{
		for(int x=pos_x;x<(pos_x+w);x++)
		{
			fb[x+(y*320)] = color;
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
				fb[(pos_x+x)+((pos_y+y)*320)] = color;		
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