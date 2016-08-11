#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "libretro.h"
#include "player.h"
#include "graphics.h"

static unsigned int framecounter;
static uint16_t x=0;
static uint16_t y=0;
static uint16_t previnput;
static bool is_playing;

void handle_error( const char* error );
void handle_info( const char* info);

// Callbacks
static retro_log_printf_t log_cb;
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
void retro_set_environment(retro_environment_t cb) { environ_cb = cb; }
void retro_set_audio_sample(retro_audio_sample_t cb) { audio_cb = cb; }
void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb) { audio_batch_cb = cb; }

unsigned retro_api_version(void) { return RETRO_API_VERSION; }
unsigned retro_get_region(void) { return RETRO_REGION_PAL; }

unsigned short framebuffer[320*240];

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

/*
 * Tell libretro about this core, it's name, version and which rom files it supports.
 */
void retro_get_system_info(struct retro_system_info *info)
{
    memset(info, 0, sizeof(*info));
    info->library_name = "Game Music Emulator";
    info->library_version = "1.0";
    info->need_fullpath = false;
    info->valid_extensions = "ay|gbs|gym|hes|kss|nsf|nsfe|sap|spc|vgm|vgz";
}

/*
 * Tell libretro about the AV system; the fps, sound sample rate and the
 * resolution of the display.
 */
void retro_get_system_av_info(struct retro_system_av_info *info) {
    int pixel_format = RETRO_PIXEL_FORMAT_RGB565;
    memset(info, 0, sizeof(*info));
    info->timing.fps            = 60.0f;
    info->timing.sample_rate    = 44100;
    info->geometry.base_width   = 320;
    info->geometry.base_height  = 240;
    info->geometry.max_width    = 320;
    info->geometry.max_height   = 240;
    info->geometry.aspect_ratio = 320.0f / 240.0f;
    environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &pixel_format);
}

void retro_init(void)
{
    /* set up some logging */
    struct retro_log_callback log;
    unsigned level = 0;
    if (environ_cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &log))
        log_cb = log.log;
    else
        log_cb = NULL;
    // the performance level is guide to frontend to give an idea of how intensive this core is to run
    environ_cb(RETRO_ENVIRONMENT_SET_PERFORMANCE_LEVEL, &level);
	framecounter = 0;
}

// End of retrolib
void retro_deinit(void) {}

// Reset gme
void retro_reset(void) 
{ 
	
}

// Run a single frame
void retro_run(void)
{
	uint16_t input = 0;
	uint16_t realinput = 0;
	int i;
	char *message = NULL;
	// input handling
	input_poll_cb();
	for(i=0;i<16;i++)
	{
		if(input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, i))
		{
			realinput |= 1<<i;
		}
	}
	input = realinput & ~previnput;
	previnput = realinput;	
	if(input & (1<<RETRO_DEVICE_ID_JOYPAD_L))
	{
		prev_track();
	}
	
	if(input & (1<<RETRO_DEVICE_ID_JOYPAD_R))
	{
		next_track();
	}
	
	if(input & (1<<RETRO_DEVICE_ID_JOYPAD_START))
	{
		is_playing = !is_playing;
	}
	//graphic handling
	memset(framebuffer,0,sizeof(unsigned short) * 320 * 240);
	message = malloc(100);
	//lines
	draw_line(framebuffer,get_color(31,63,31),5,5,315,5);
	draw_line(framebuffer,get_color(31,63,31),5,235,315,235);
	draw_line(framebuffer,get_color(31,63,31),5,5,5,235);
	draw_line(framebuffer,get_color(31,63,31),315,5,315,235);
	draw_line(framebuffer,get_color(31,63,31),5,5,20,20);
	draw_line(framebuffer,get_color(31,63,31),315,5,300,20);
	draw_line(framebuffer,get_color(31,63,31),5,235,20,220);
	draw_line(framebuffer,get_color(31,63,31),315,235,300,220);
	//text
	get_game_name(message);
	draw_string(framebuffer,get_color(31,0,0),message,6,100);
	get_track_count(message);
	draw_string(framebuffer,get_color(0,63,0),message,6,110);
	get_song_name(message);
	draw_string(framebuffer,get_color(0,0,31),message,6,120);
	get_track_position(message);
	draw_string(framebuffer,get_color(31,63,31),message,6,130);
    video_cb(framebuffer, 320, 240, sizeof(unsigned short) * 320);
	//audio handling
	audio_batch_cb(play(is_playing),1470);
	framecounter++;
	free(message);
}

// File Loading
bool retro_load_game(const struct retro_game_info *info)
{
	long sample_rate = 44100;
	x = (320/2)-(32/2);
	y = (240/2)-(32/2);
    if (info && info->data) { // ensure there is ROM data
		open_file( info->path, sample_rate);
		is_playing = true;
    }
	return true;		
}

bool retro_load_game_special(unsigned game_type, const struct retro_game_info *info, size_t num_info) 
{ 
   return false;
}

void retro_unload_game(void) 
{
	close_file();
}

void handle_error( const char* error )
{
	char str [256];
	if(error) {
		sprintf( str, "Error: %s", error );
		log_cb(RETRO_LOG_ERROR, str ); 		
	}
}

void handle_info( const char* info )
{
	char str [256];
	if(info) {
		sprintf( str, "Info: %s", info );
		log_cb(RETRO_LOG_INFO, str ); 		
	}
}