#ifndef GME_LIBRETRO_PLAYLIST_H__
#define GME_LIBRETRO_PLAYLIST_H__

#include "gme.h"
#include "fileformat.h"

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

bool get_playlist(const char *path,playlist **pl);

bool get_playlist_gme_files(const char *path,gme_file_data*** gme_files,int *num_file, int *num_tracks);
bool get_gme_file_data(file_data *fd,gme_file_data **gfd);
bool get_track_data(Music_Emu* emu, int fileid, int trackid, const char *path,gme_track_data **gtd);

bool cleanup_playlist(playlist* pl);

#endif
