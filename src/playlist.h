#ifndef GME_LIBRETRO_PLAYLIST_H__
#define GME_LIBRETRO_PLAYLIST_H__

#include "gme.h"

typedef struct {
	char* name;
	char* data;
	int length;
} file_data;

typedef struct {
	char* name;
	char* data;
	int length;
	gme_type_t file_type;
	int num_tracks;
} gme_file_data;

typedef struct {
	int file_id;
	int track_id;
	int track_length;
	char* game_name;
	char* track_name;	
} gme_track_data;
		
typedef struct {
	int num_files;
	gme_file_data** files;
	int num_tracks;
	gme_track_data** tracks;
} playlist;

playlist* get_playlist(const char *path);

gme_file_data** get_playlist_gme_files(const char *path);
gme_file_data* get_gme_file_data(file_data *fd);
file_data* get_file_data(const char *path);
file_data** get_files_from_zip(const char *path);

gme_track_data* get_track_data(Music_Emu* emu, int fileid, int trackid, const char *path);

void cleanup_playlist(playlist* pl);

#endif
