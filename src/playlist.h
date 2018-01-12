#ifndef GME_LIBRETRO_PLAYLIST_H__
#define GME_LIBRETRO_PLAYLIST_H__

#include "gme.h"

typedef enum {GME_ZIP,GME_FILE} playlist_type;

typedef struct {
	gme_type_t track_type;
	int track_number;
	char *track_data;
	int track_data_length;
	char *track_name;
	long track_length;
} playlist_entry;

typedef struct {
	playlist_type type;
	char *filename;
	char *game_name;
	char *playlist_data;
	int num_tracks;
	int current_track;
	int playlist_data_length;
	playlist_entry *entries[255];
} playlist;

playlist *load_playlist(const char *path,long sample_rate);
bool load_gme_file(playlist *playlist,long sample_rate);
bool load_zip(playlist *playlist,long sample_rate);
void unload_playlist(playlist *playlist);

#endif
