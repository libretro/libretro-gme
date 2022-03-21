#include "graphics.h"
#include <stdlib.h>
#include <string.h>

#include <retro_miscellaneous.h>
#include "font8x8.h"

const short gme_rainbow7[7] = {gme_red,gme_orange,gme_yellow,gme_green,gme_blue,gme_indigo,gme_violet};

bool is_font_pixel(unsigned char letter, int x, int y)
{
   if(letter >=0x00 && letter <0x7F)
      return ((font8x8_basic[letter][y] & (1 << x)) > 0);
   else if(letter >=0x80 && letter <0xA0)
      return ((font8x8_control[letter - 0x80][y] & (1 << x)) > 0);
   else if(letter >=0xA0 && letter <0x100)
      return ((font8x8_ext_latin[letter - 0xA0][y] & (1 << x)) > 0);
   else
      return false;
}


unsigned short get_color(char r, char g, char b)
{
   return (r << 11) | (g << 5) | b;
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

surface *clip_surface(surface *src_surf, int x_src, int y_src, box clip_box)
{
   int wx0,wy0,wx1,wy1; //world coord
   int lx,ly,lw,lh; //local coord
   surface *clipped_surf = NULL;
   //check if completely out of bounds
   if(      (x_src+src_surf->width) <clip_box.x0 
         || (x_src > clip_box.x1) 
         || (y_src+src_surf->height) < clip_box.y0 
         || (y_src > clip_box.y1))
      return clipped_surf;

   wx0          = MAX(x_src,clip_box.x0);
   wy0          = MAX(y_src,clip_box.y0);
   wx1          = MIN(x_src+src_surf->width,clip_box.x1);
   wy1          = MIN(y_src+src_surf->height,clip_box.y1);
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

void draw_box(surface *surf, unsigned short color, box b)
{
   draw_line(surf,color,b.x0,b.y0,b.x1,b.y0); //top line
   draw_line(surf,color,b.x0,b.y1,b.x1,b.y1); //bottom line	
   draw_line(surf,color,b.x0,b.y0,b.x0,b.y1); //left line
   draw_line(surf,color,b.x1,b.y0,b.x1,b.y1); //right line
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

   for(y=0;y<8;y++)
   {
      for(x=0;x<8;x++)
      {
         if(is_font_pixel((unsigned char)letter, x, y))
            set_pixel(surf,pos_x+x,pos_y+y,color);
      }
   }		
}

void draw_string(surface *surf, unsigned short color, char* text, int pos_x, int pos_y, unsigned int framecounter, box clip_box)
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
   
   if(temp_surface)
   {
      for(x=0;x<msglen;x++)
         draw_letter(temp_surface,color,text[x],(x*8),0);
      if((msglen*8)>(surf->width-40))
      {
         delta    = (msglen*8) -(surf->width-40);
         modulo   = delta + (delay * 2);
         x_offset = (modulo - abs(framecounter/frame_delay % (2*modulo) - modulo)) - delay; // triangle function
         x_offset = MAX(x_offset,0); //clamp left to add delay
         x_offset = MIN(x_offset,delta); //clamp right to add delay
      }

      clipped_surface = clip_surface(temp_surface,pos_x-x_offset,pos_y, clip_box);
      if(clipped_surface)
      {
         copy_surface(clipped_surface,surf,0,0,pos_x,pos_y,clipped_surface->width,clipped_surface->height);
         free_surface(clipped_surface);		
      }
      free_surface(temp_surface);
   }
}

int get_string_length(char* text)
{

   return strlen(text) * 8;
}
