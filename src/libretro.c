#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include <libretro.h>
#include <retro_miscellaneous.h>
#include <streams/file_stream.h>

#include "graphics.h"
#include "player.h"

// Static globals
static surface *framebuffer = NULL;
static uint16_t previnput = 0;
static float last_aspect = 0.0f;
static float last_scale = 0.0f;
static bool display_rainbow;
// Callbacks

retro_log_printf_t log_cb;

static retro_video_refresh_t video_cb;
static retro_input_poll_t input_poll_cb;
static retro_input_state_t input_state_cb;
static retro_environment_t environ_cb;
static retro_audio_sample_t audio_cb;
static retro_audio_sample_batch_t audio_batch_cb;
// libretro global setters
void retro_set_video_refresh(retro_video_refresh_t cb) { video_cb = cb; }
void retro_set_input_poll(retro_input_poll_t cb) { input_poll_cb = cb; }
void retro_set_input_state(retro_input_state_t cb) { input_state_cb = cb; }


void retro_set_environment(retro_environment_t cb)
{
   struct retro_vfs_interface_info vfs_iface_info;
   environ_cb = cb;

   vfs_iface_info.required_interface_version = 1;
   vfs_iface_info.iface                      = NULL;
   if (cb(RETRO_ENVIRONMENT_GET_VFS_INTERFACE, &vfs_iface_info))
      filestream_vfs_init(&vfs_iface_info);
   static const struct retro_variable vars[] = {
      { "gme_aspect", "Aspect Ratio; 16:9|4:3"},
      { "gme_scale", "Scale; 1x|2x"},
      { "display_rainbow", "Display Rainbow Animation; false|true"},
      { NULL, NULL},
   };
   cb(RETRO_ENVIRONMENT_SET_VARIABLES, (void*)vars);
}

void retro_set_audio_sample(retro_audio_sample_t cb) { audio_cb = cb; }
void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb) { audio_batch_cb = cb; }

unsigned retro_api_version(void) { return RETRO_API_VERSION; }
unsigned retro_get_region(void) { return RETRO_REGION_PAL; }

// Serialisation methods
size_t retro_serialize_size(void) { return 0; }
bool retro_serialize(void *data, size_t size) { return false; }
bool retro_unserialize(const void *data, size_t size) { return false; }

// libretro unused api functions
void retro_set_controller_port_device(unsigned port, unsigned device) {}
void *retro_get_memory_data(unsigned id) { return NULL; }
size_t retro_get_memory_size(unsigned id){ return 0; }

// Cheats
void retro_cheat_reset(void) {}
void retro_cheat_set(unsigned index, bool enabled, const char *code) {}


static int draw_text_centered(char* text,unsigned short color, int y, int maxlen)
{
   int centerx = framebuffer->width/2;
   int msglen = get_string_length(text);
   draw_string(framebuffer,color,text,MAX(centerx-(msglen/2),21), y,get_track_elapsed_frames());
   return MAX(msglen,maxlen);
}

// Custom functions
static void draw_ui(void)
{
   int offset;
   int maxlen    = 0;
   int centerx = framebuffer->width/2;
   int centery = framebuffer->height/2;
   box ob = {5,5,framebuffer->width-5,framebuffer->height-5};
   box ib = {20,20,framebuffer->width-20,framebuffer->height-20};
   char colorstart = 0;
   unsigned short lc = 0;
   char *message = malloc(100);
   //lines

   draw_box(framebuffer,gme_white,ob);
   draw_line(framebuffer,gme_gray,ob.x0,ob.y0,ib.x0,ib.y0); //top-left corner
   draw_line(framebuffer,gme_gray,ob.x1,ob.y0,ib.x1,ib.y0); //top-right corner
   draw_line(framebuffer,gme_gray,ob.x0,ob.y1,ib.x0,ib.y1); //bottom-left corner
   draw_line(framebuffer,gme_gray,ob.x1,ob.y1,ib.x1,ib.y1);
   draw_box(framebuffer,gme_gray,ib);
   //display rainbow if variable is set
   if(display_rainbow)
   {
      colorstart = (get_track_elapsed_frames() % 30) >> 2;
      for(offset=1;offset<15;offset++)
      {
            lc = gme_rainbow7[(colorstart + (offset >> 1)) % 7];
            draw_line(framebuffer,lc,ob.x0+offset,ob.y0+1+offset,ob.x0+offset,ob.y1-1-offset); //left
            draw_line(framebuffer,lc,ob.x0+1+offset,ob.y0+offset,ob.x1-1-offset,ob.y0+offset); //top
            draw_line(framebuffer,lc,ob.x1-offset,ob.y0+1+offset,ob.x1-offset,ob.y1-1-offset); //right
            draw_line(framebuffer,lc,ob.x0+1+offset,ob.y1-offset,ob.x1-1-offset,ob.y1-offset); //top
      }
   }
   //text

   maxlen = draw_text_centered(get_game_name(message),gme_red,centery-20,maxlen);
   maxlen = draw_text_centered(get_track_count(message),gme_yellow,centery-10,maxlen);
   maxlen = draw_text_centered(get_song_name(message),gme_blue,centery,maxlen);
   maxlen = draw_text_centered(get_track_position(message),gme_white,centery+10,maxlen);
   maxlen = MIN(maxlen,framebuffer->width-40);
   box tb = {centerx-(maxlen/2),centery-22,centerx+(maxlen/2),centery+22};
   draw_box(framebuffer,gme_violet,tb);
   free(message);
}

/*
 * Tell libretro about this core, it's name, version and which rom files it supports.
 */
void retro_get_system_info(struct retro_system_info *info)
{
   memset(info, 0, sizeof(*info));
   info->library_name = "Game Music Emulator";
   info->library_version = "v0.6.1";
   info->need_fullpath = true;
   info->valid_extensions = "ay|gbs|gym|hes|kss|nsf|nsfe|sap|spc|vgm|vgz|zip";
   info->block_extract = true;
}

/*
 * Tell libretro about the AV system; the fps, sound sample rate and the
 * resolution of the display.
 */
void retro_get_system_av_info(struct retro_system_av_info *info)
{
   float aspect = 4.0f / 3.0f;
   int scale = 2;
   int width = 640;
   int height = 480;
   struct retro_variable var = {0};
   if(framebuffer)
      free_surface(framebuffer);
   var.key = "gme_scale";
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      if (!strcmp(var.value, "1x"))
         scale = 1;
      else if (!strcmp(var.value, "2x"))
         scale = 2;
   }
   var.key = "gme_aspect";
   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      if (!strcmp(var.value, "4:3"))
      {
         aspect = 4.0f / 3.0f;
         width = 320 * scale;
         height = 240 * scale;
      }
      else if (!strcmp(var.value, "16:9"))
      {
         aspect = 16.0f / 9.0f;
         width = 320 * scale;
         height = 180 * scale;
      }
   }
   var.key = "display_rainbow";
   if(environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      if(!strcmp(var.value,"true"))
         display_rainbow = true;
      else
         display_rainbow = false;
   }
   int pixel_format = RETRO_PIXEL_FORMAT_RGB565;
   memset(info, 0, sizeof(*info));
   info->timing.fps            = 60.0f;
   info->timing.sample_rate    = 44100;
   info->geometry.base_width   = width;
   info->geometry.base_height  = height;
   info->geometry.max_width    = 640;
   info->geometry.max_height   = 480;
   info->geometry.aspect_ratio = aspect;
   environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &pixel_format);
   framebuffer = create_surface(width,height,2);
   last_aspect = aspect;
   last_scale = scale;
}

void retro_init(void)
{
   unsigned level = 0;
   /* set up some logging */
   struct retro_log_callback log;
   if (environ_cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &log))
      log_cb = log.log;
   else
      log_cb = NULL;

   // the performance level is guide to frontend to give an idea of how intensive this core is to run
   environ_cb(RETRO_ENVIRONMENT_SET_PERFORMANCE_LEVEL, &level);
   framebuffer = create_surface(320,240,2);
}

// End of retrolib
void retro_deinit(void)
{
   if (framebuffer)
      free_surface(framebuffer);
   framebuffer = NULL;
}

// Reset gme
void retro_reset(void)
{
    start_track(0);
}

// Run a single frame
void retro_run(void)
{
   uint16_t input = 0;
   uint16_t realinput = 0;
   int i;
   bool updated = false;
   // input handling
   input_poll_cb();
   for(i=0;i<16;i++)
   {
      if(input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, i))
         realinput |= 1<<i;
   }
   input = realinput & ~previnput;
   previnput = realinput;

   if(input & (1<<RETRO_DEVICE_ID_JOYPAD_L))
      prev_track();

   if(input & (1<<RETRO_DEVICE_ID_JOYPAD_R))
      next_track();

   if(input & (1<<RETRO_DEVICE_ID_JOYPAD_START))
      play_pause();

   //graphic handling
   memset(framebuffer->pixel_data,0,framebuffer->bytes_per_pixel * framebuffer->width * framebuffer->height);
   draw_ui();
   video_cb(framebuffer->pixel_data, framebuffer->width, framebuffer->height, framebuffer->bytes_per_pixel * framebuffer->width);
   //audio handling
   audio_batch_cb(play(),1470);
   if(environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE,&updated) && updated)
   {
      float aspect = last_aspect;
      int scale = last_scale;
      struct retro_system_av_info info;
      retro_get_system_av_info(&info);
      if((aspect != last_aspect && aspect != 0.0f) || (scale != last_scale && scale != 0.0f))
      {
		environ_cb(RETRO_ENVIRONMENT_SET_GEOMETRY, &info.geometry);
      }
   }
}

// File Loading
bool retro_load_game(const struct retro_game_info *info)
{
   if (info && open_file(info->path, 44100))
      return true;
   return false;
}

bool retro_load_game_special(unsigned game_type, const struct retro_game_info *info, size_t num_info)
{
   return false;
}

void retro_unload_game(void)
{
   close_file();
}
