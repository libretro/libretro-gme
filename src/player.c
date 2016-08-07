#include "player.h"
#include <string.h>

static int current_track;
Music_Emu* emu;
static short audio_buffer[8192];
gme_info_t* track_info_;

void open_file(const char *path, long sample_rate)
{
	gme_open_file(path, &emu, sample_rate );
	start_track(0);
}

void close_file(void)
{
	gme_delete( emu );
}

void start_track(int track)
{
	current_track = track;
	gme_track_info( emu, &track_info_, track );
	if ( track_info_->length <= 0 )
		track_info_->length = track_info_->intro_length +
					track_info_->loop_length * 2;	
	if ( track_info_->length <= 0 )
		track_info_->length = (long) (2.5 * 60 * 1000);
	gme_set_fade(emu, track_info_->length );
	gme_start_track(emu, current_track);
}

short *play(bool is_playing)
{
	if(is_playing)
	{
		if(gme_track_ended(emu))
		{
			next_track();
		}
		gme_play( emu, 2048, audio_buffer );
	}
	else
	{
		memset(audio_buffer,0,sizeof(short)*8192);
	}
	return audio_buffer;
}

void next_track(void)
{
	if(current_track< (gme_track_count(emu)-1))
	{
		start_track(++current_track);
	}
}

void prev_track(void)
{
	if(current_track > 0)
	{
		start_track(--current_track);
	}
}

void get_game_name(char *buf)
{
	sprintf(buf, "%s",track_info_->game);
}

void get_track_count(char *buf)
{
	sprintf(buf, "%d/%d",current_track+1,gme_track_count(emu));
}

void get_song_name(char *buf)
{
	sprintf(buf, "%s",strcmp(track_info_->song,"") ? track_info_->song : "-");
}

void get_track_position(char *buf)
{
	long seconds = track_info_->length / 1000;
	long elapsed_seconds = gme_tell(emu) / 1000;
	sprintf(buf, "(%ld:%02ld / %ld:%02ld)",elapsed_seconds/60,elapsed_seconds%60,seconds/60,seconds % 60);
}
