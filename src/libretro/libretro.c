#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "libretro.h"
#include "gme/gme.h"

Music_Emu* emu;

void handle_error( const char* error );

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
void retro_set_environment(retro_environment_t cb) { environ_cb = cb; }
void retro_set_audio_sample(retro_audio_sample_t cb) { audio_cb = cb; }
void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb) { audio_batch_cb = cb; }

unsigned retro_api_version(void) { return RETRO_API_VERSION; }
unsigned retro_get_region(void) { return RETRO_REGION_PAL; }

short audio_buffer[4096];

unsigned short framebuffer[330*410];

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
    info->valid_extensions = "nsf|spc";
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
    info->geometry.base_width   = 330;
    info->geometry.base_height  = 410;
    info->geometry.max_width    = 330;
    info->geometry.max_height   = 410;
    info->geometry.aspect_ratio = 330.0f / 410.0f;

    environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &pixel_format);
}

void retro_init(void)
{
    /* set up some logging */
    struct retro_log_callback log;
    unsigned level = 4;

    if (environ_cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &log))
        log_cb = log.log;
    else
        log_cb = NULL;

    // the performance level is guide to frontend to give an idea of how intensive this core is to run
    environ_cb(RETRO_ENVIRONMENT_SET_PERFORMANCE_LEVEL, &level);

    //vectrex->Reset();
}

// End of retrolib
void retro_deinit(void) {}

// Reset the Vectrex
void retro_reset(void) 
{ 
	
}

// Run a single frames with out Vectrex emulation.
void retro_run(void)
{
	audio_batch_cb(audio_buffer,1960);
	handle_error( gme_play( emu, 2048, audio_buffer ) );
    video_cb(framebuffer, 330, 410, sizeof(unsigned short) * 330);
}

// File Loading
bool retro_load_game(const struct retro_game_info *info)
{
	long sample_rate = 44100;
	int track = 0;
	
    if (info && info->data) { // ensure there is ROM data
		handle_error( gme_open_file( info->path, &emu, sample_rate ) );
		handle_error( gme_start_track( emu, track ) );
		handle_error( gme_play( emu, 2048, audio_buffer ) );		
    }
    return true;
}

bool retro_load_game_special(unsigned game_type, const struct retro_game_info *info, size_t num_info) 
{ 
   return false;
}

void retro_unload_game(void) 
{
	gme_delete( emu );
}

void handle_error( const char* error )
{
	char str [256];
	if(error) {
		sprintf( str, "Error: %s", error );
		log_cb(RETRO_LOG_ERROR, str ); 		
	}
}