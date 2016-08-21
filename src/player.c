#include "player.h"
#include <string.h>
#include <boolean.h>
#include "playlist.h"

static playlist *plist = NULL;
static playlist_entry *entry = NULL;
Music_Emu* emu = NULL;
static short audio_buffer[8192];
static bool is_playing_;
static long sample_rate_;

bool is_emu_loaded(void)
{
	return (emu != NULL);
}

void open_file(const char *path, long sample_rate)
{
	sample_rate_ = sample_rate;
	plist = load_playlist(path,sample_rate_);
	start_track(plist->current_track);
}

void close_file(void)
{
	gme_delete( emu );
	if(plist!=NULL)
		unload_playlist(plist);
}

void start_track(int track)
{
	memset(audio_buffer,0,8192);
	plist->current_track = track;
	entry = plist->entries[track];
	if(plist->type == GME_ZIP)
	{
		if(emu !=NULL)
			gme_delete (emu);
		emu = gme_new_emu(entry->track_type,sample_rate_);
		gme_load_data(emu,entry->track_data,entry->track_data_length);
		gme_start_track(emu, 0);
		is_playing_ = true;
	}
	else if(plist->type == GME_FILE)
	{
		if(emu == NULL)
		{
			emu = gme_new_emu(entry->track_type,sample_rate_);
			gme_load_data(emu,plist->playlist_data,plist->playlist_data_length);
		}
		gme_start_track(emu, track);
		is_playing_ = true;
	}
	else
	{
		emu = NULL;
		is_playing_ = false;
	}

}

short *play(void)
{
	if(is_playing_)
	{
		if(gme_track_ended(emu))
		{
			if(plist->current_track< (plist->num_tracks-1))
			{
				start_track(++plist->current_track);
			}
			else
			{
				is_playing_ = false;
			}
		}
		else
		{
			gme_play( emu, 2048, audio_buffer );
		}
	}
	else
	{
		memset(audio_buffer,0,8192);
	}
	return audio_buffer;
}

void next_track(void)
{
	if(plist->current_track< (plist->num_tracks-1))
	{
		start_track(++plist->current_track);
	}
}

void prev_track(void)
{
	if(plist->current_track > 0)
	{
		start_track(--plist->current_track);
	}
}

char *get_game_name(char *buf)
{
	sprintf(buf, "%s",plist->game_name);
	return buf;
}

char *get_track_count(char *buf)
{
	sprintf(buf, "%d/%d",entry->track_number,plist->num_tracks);
	return buf;
}

char *get_song_name(char *buf)
{
	sprintf(buf, "%s",entry->track_name);
	return buf;
}

char *get_track_position(char *buf)
{
	long seconds = entry->track_length / 1000;
	long elapsed_seconds = gme_tell(emu) / 1000;
	sprintf(buf, "(%ld:%02ld / %ld:%02ld)",elapsed_seconds/60,elapsed_seconds%60,seconds/60,seconds % 60);
	return buf;
}

int get_track_elapsed_frames(void)
{
	return gme_tell_samples(emu)/1470;
}

void play_pause(void)
{
	is_playing_ = !is_playing_;
}
