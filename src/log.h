#ifndef GME_LIBRETRO_LOG_H__
#define GME_LIBRETRO_LOG_H__

#include "libretro.h"

void init_log(retro_environment_t environ_cb);
void handle_error( const char* error );
void handle_info( const char* info );

#endif
