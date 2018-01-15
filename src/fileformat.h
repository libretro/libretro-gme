#ifndef GME_LIBRETRO_FILEFORMAT_H__
#define GME_LIBRETRO_FILEFORMAT_H__

typedef struct {
	char* name;
	char* data;
	int length;
} file_data;

bool get_file_data(const char *path,file_data ***files);

#endif