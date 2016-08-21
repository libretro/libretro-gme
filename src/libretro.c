#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "libretro.h"
#include "graphics.h"
#include "player.h"
#include "log.h"

// Static globals
static surface *framebuffer = NULL;;
static uint16_t previnput = 0;

// Callbacks

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

// Custom functions
void draw_ui(void);
int draw_text_centered(char* text, char r, char g, char b, int y, int maxlen);
void handle_error( const char* error );
void handle_info( const char* info);

/*
 * Tell libretro about this core, it's name, version and which rom files it supports.
 */
void retro_get_system_info(struct retro_system_info *info)
{
    memset(info, 0, sizeof(*info));
    info->library_name = "Game Music Emulator";
    info->library_version = "v0.6.1";
    info->need_fullpath = false;
    info->valid_extensions = "ay|gbs|gym|hes|kss|nsf|nsfe|sap|spc|vgm|vgz|zip";
	info->block_extract = true;
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
  unsigned level = 0;
  /* set up some logging */
  init_log(environ_cb);
  // the performance level is guide to frontend to give an idea of how intensive this core is to run
  environ_cb(RETRO_ENVIRONMENT_SET_PERFORMANCE_LEVEL, &level);
	framebuffer = create_surface(320,240,2);
}

// End of retrolib
void retro_deinit(void) {
	free(framebuffer);
}

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
		play_pause();
	}
	//graphic handling
	memset(framebuffer->pixel_data,0,framebuffer->bytes_per_pixel * framebuffer->width * framebuffer->height);
	draw_ui();
  video_cb(framebuffer->pixel_data, framebuffer->width, framebuffer->height, framebuffer->bytes_per_pixel * framebuffer->width);
  //audio handling
	audio_batch_cb(play(),1470);
}

// File Loading
bool retro_load_game(const struct retro_game_info *info)
{
	long sample_rate = 44100;
	char message[256];
	int i;
	if (info && info->data) { // ensure there is ROM data
		//open_file( info->path, sample_rate);
    open_file(info->path,sample_rate);
		return true;
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

void draw_ui(void)
{
	char *message = NULL;
	int maxlen = 0;
	message = malloc(100);
	//lines
	draw_box(framebuffer,get_color(31,63,31),5,5,315,235);
	draw_line(framebuffer,get_color(15,31,15),5,5,20,20);
	draw_line(framebuffer,get_color(15,31,15),315,5,300,20);
	draw_line(framebuffer,get_color(15,31,15),5,235,20,220);
	draw_line(framebuffer,get_color(15,31,15),315,235,300,220);
	draw_box(framebuffer,get_color(15,31,15),20,20,300,220);
	//text
	maxlen = draw_text_centered(get_game_name(message),31,0,0,100,maxlen);
	maxlen = draw_text_centered(get_track_count(message),0,63,0,110,maxlen);
	maxlen = draw_text_centered(get_song_name(message),0,0,31,120,maxlen);
	maxlen = draw_text_centered(get_track_position(message),31,63,31,130,maxlen);
	maxlen = min(maxlen,280);
	draw_box(framebuffer,get_color(15,0,15),160-(maxlen/2),98,160+(maxlen/2),140);
	free(message);
}

int draw_text_centered(char* text,char r, char g, char b, int y, int maxlen)
{
	int msglen;
	msglen = get_string_length(text);
	draw_string(framebuffer,get_color(r,g,b),text,max(160-(msglen/2),21), y,get_track_elapsed_frames());
	return max(msglen,maxlen);
}
