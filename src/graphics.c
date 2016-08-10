#include "graphics.h"
#include <stdlib.h>
#include <string.h>

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
			fb[start_x+(y*320)] = color;
		}
	}
	else if(start_y==end_y) // horizontal line
	{
		for(int x=start_x;x<=end_x;x++)
		{
			fb[x+(start_y*320)] = color;
		}
	}
	else // Brenenham line
	{
		int x, y, end, dx, dy, p;
		dx = abs(start_x - end_x);
		dy = abs(start_y - end_y);
		p = 2 * dy - dx;
		if(start_x > end_x)
		{
			x = end_x;
			y = end_y;
			end = start_x;
		}
		else
		{
			x = start_x;
			y = start_y;
			end = end_x;
		}
		fb[x+(y*320)] = color;
		while(x<end)
		{
			x++;
			if(p <0)
			{
				p += 2 * dy; 
			}
			else
			{
				y++;
				p += 2 * dy - dx;
			}
			fb[x+(y*320)] = color;
		}
	}
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