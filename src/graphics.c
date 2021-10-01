#include "graphics.h"
#include <stdlib.h>
#include <string.h>

#include <retro_miscellaneous.h>

unsigned short get_color(char r, char g, char b)
{
   unsigned short color= (r << 11) | (g << 5) | b;
   return color;
}

surface *create_surface(unsigned int width, unsigned int height, unsigned int bpp)
{
   surface *newsurf = malloc(sizeof(surface));
   if (!newsurf)
      return NULL;
   newsurf->pixel_data = malloc(width*height*bpp);

   if (!newsurf->pixel_data)
   {
      free(newsurf);
      return NULL;
   }

   memset(newsurf->pixel_data,0,width*height*bpp);
   newsurf->width           = width;
   newsurf->height          = height;
   newsurf->bytes_per_pixel = bpp;	
   return newsurf;
}

void free_surface(surface *surf)
{
   if (surf)
   {
      free(surf->pixel_data);
      free(surf);		
   }
}

surface *clip_surface(surface *src_surf, int x_src, int y_src, int x0, int y0, int x1, int y1)
{
	int wx0,wy0,wx1,wy1; //world coord
	int lx,ly,lw,lh; //local coord
	surface *clipped_surf = NULL;
	//check if completely out of bounds
	if(      (x_src+src_surf->width) <x0 
         || (x_src > x1) 
         || (y_src+src_surf->height) < y0 
         || (y_src > y1))
		return clipped_surf;
	wx0          = MAX(x_src,x0);
	wy0          = MAX(y_src,y0);
	wx1          = MIN(x_src+src_surf->width,x1);
	wy1          = MIN(y_src+src_surf->height,y1);
	lx           = wx0 - x_src;
	ly           = wy0 - y_src;
	lw           = wx1 - wx0;
	lh           = wy1 - wy0;
	clipped_surf = create_surface(lw, lh,2);
	copy_surface(src_surf,clipped_surf,lx,ly,0,0,lw,lh);
	return clipped_surf;
}

void copy_surface(surface *src_surf, surface *dst_surf, int x_src, int y_src, int x_dst, int y_dst, int w, int h)
{
	int x,y;
	unsigned short pixel;
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
      int y;
      for(y=start_y;y<=end_y;y++)
         set_pixel(surf,start_x,y,color);
   }
   else if(start_y==end_y) // horizontal line
   {
      int x;
      for(x=start_x;x<=end_x;x++)
         set_pixel(surf,x,start_y,color) = color;
   }
   else 
   {
      // Bresenham line algorithm, copied from https://rosettacode.org/wiki/Bitmap/Bresenham%27s_line_algorithm#C
      int x = start_x, y = start_y;
      int dx = abs(end_x-start_x), sx = start_x<end_x ? 1 : -1;
      int dy = abs(end_y-start_y), sy = start_y<end_y ? 1 : -1; 
      int err = (dx>dy ? dx : -dy)/2, e2;
      for(;;)
      {
         set_pixel(surf,x,y,color);
         if (x==end_x && y==end_y)
            break;
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
   int y, x;
	for(y=pos_y;y<(pos_y+h);y++)
	{
		for(x=pos_x;x<(pos_x+w);x++)
			set_pixel(surf,x,y,color);
	}	
}

void draw_letter(surface *surf, unsigned short color, char letter, int pos_x, int pos_y)
{
   int y, x;
	//calculate letter offset
	int charx = (letter % 16) * 8;
	int chary = (letter >> 4) * 8;

	for(y=0;y<8;y++)
	{
		for(x=0;x<8;x++)
		{
			if(is_font_pixel(charx+x,chary+y))
				set_pixel(surf,pos_x+x,pos_y+y,color);
		}
	}		
}

void draw_string(surface *surf, unsigned short color, char* text, int pos_x, int pos_y, unsigned int framecounter)
{
   int x;
	int x_offset = 0;
	int delta = 0;
	int delay = 30;
	int modulo = 0;
	int frame_delay = 2;
	int msglen = strlen(text);
   surface *clipped_surface = NULL;
	surface *temp_surface    = create_surface((msglen*8),8,2);	

	for(x=0;x<msglen;x++)
		draw_letter(temp_surface,color,text[x],(x*8),0);

	if((msglen*8)>280)
	{
		delta    = (msglen*8) -280;
		modulo   = delta + (delay * 2);
		x_offset = (modulo - abs(framecounter/frame_delay % (2*modulo) - modulo)) - delay; // triangle function
		x_offset = MAX(x_offset,0); //clamp left to add delay
		x_offset = MIN(x_offset,delta); //clamp right to add delay
	}

	clipped_surface = clip_surface(temp_surface,pos_x-x_offset,pos_y,21,21,299,219);

	if (clipped_surface)
	{
		copy_surface(clipped_surface,surf,0,0,pos_x,pos_y,clipped_surface->width,clipped_surface->height);
		free_surface(clipped_surface);		
	}
	free_surface(temp_surface);
}

int get_string_length(char* text)
{
	int ln = strlen(text);
	return ln * 8;
}
