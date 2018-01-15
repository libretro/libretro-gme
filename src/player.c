#include "player.h"
#include <string.h>
#include <boolean.h>
#include "playlist.h"
#include "fileformat.h"
#include "log.h"

static playlist *plist = NULL;
static gme_track_data *track = NULL;
static gme_file_data *file = NULL;
static int prev_fileid;
Music_Emu* emu = NULL;
static short audio_buffer[8192];
static bool is_playing_;
static long sample_rate_;
static int current_track;

bool is_emu_loaded(void)
{
	return (emu != NULL);
}

bool open_file(const char *path, long sample_rate)
{
	sample_rate_ = sample_rate;
	current_track = 0;
	prev_fileid = -1;
	if(get_playlist(path,&plist))
	{
		start_track(current_track);
		return true;
	}
	else
		return false;
}

void close_file(void)
{
	gme_delete( emu );
	if(plist!=NULL)
		cleanup_playlist(plist);
}

void start_track(int tracknr)
{
	char message[256];
	memset(audio_buffer,0,8192 * sizeof(short));
	current_track = tracknr;
	track = plist->tracks[tracknr];
	if(track)
	{
		//change emu if file changes
		if(track->file_id != prev_fileid)
		{
			is_playing_ = false;
			file = plist->files[track->file_id];
			prev_fileid = track->file_id;
			if(emu !=NULL)
				gme_delete (emu);		
			emu = gme_new_emu(file->file_type,sample_rate_);
			gme_load_data(emu,file->data,file->length);
			is_playing_ = true;
		}
	}
	else
	{
		strcpy(message,"Unknown track type");
		handle_error(message);
		is_playing_ = false;				
	}
	if(is_playing_)
		gme_start_track(emu, track->track_id);
}

short *play(void)
{
	if(is_playing_)
	{
		if(gme_track_ended(emu))
		{
			if(current_track< (plist->num_tracks-1))
			{
				start_track(++current_track);
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
		memset(audio_buffer,0,8192 * sizeof(short));
	}
	return audio_buffer;
}

void next_track(void)
{
	if(current_track< (plist->num_tracks-1))
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

char *get_game_name(char *buf)
{
	sprintf(buf, "%s",track->game_name);
	return buf;
}

char *get_track_count(char *buf)
{
	sprintf(buf, "%d/%d",current_track+1,plist->num_tracks);
	return buf;
}

char *get_song_name(char *buf)
{
	sprintf(buf, "%s",track->track_name);
	return buf;
}

char *get_track_position(char *buf)
{
	long seconds = track->track_length / 1000;
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
