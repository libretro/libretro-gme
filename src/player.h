#ifndef GME_LIBRETRO_PLAYER_H__
#define GME_LIBRETRO_PLAYER_H__
#include <stdio.h>
#include <stdlib.h>
#include <boolean.h>
#include "gme.h"

void open_file(const char *path, long sample_rate);
void close_file(void);
void start_track(int track);
short *play(void);
void next_track(void);
void prev_track(void);
char *get_game_name(char *buf);
char *get_track_count(char *buf);
char *get_song_name(char *buf);
char *get_track_position(char *buf);
int get_track_elapsed_frames(void);
void play_pause(void);
#endif