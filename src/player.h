#ifndef GME_LIBRETRO_PLAYER_H__
#define GME_LIBRETRO_PLAYER_H__
#include <stdio.h>
#include <stdlib.h>
#include <boolean.h>
#include "gme.h"

void open_file(const char *path, long sample_rate);
void close_file(void);
void start_track(int track);
short *play(bool is_playing);
void next_track(void);
void prev_track(void);
void get_track_title(char *title);
#endif