CC = gcc
CXX = g++
INCFLAGS= -I. -Ideps/libretro-common/include -Ideps/game-music-emu/gme
CFLAGS=-c -Wall $(INCFLAGS) -DHAVE_RPNG
CXXFLAGS=-c -Wall $(INCFLAGS)
LDFLAGS= -shared -L /mingw64/lib -lz

EXECUTABLE= gme_libretro.dll
INFO= gme_libretro.info

SOURCES_CXX := deps/game-music-emu/gme/Ay_Apu.cpp \
			deps/game-music-emu/gme/Ay_Cpu.cpp \
			deps/game-music-emu/gme/Ay_Emu.cpp \
			deps/game-music-emu/gme/Blip_Buffer.cpp \
			deps/game-music-emu/gme/Classic_Emu.cpp \
			deps/game-music-emu/gme/Data_Reader.cpp \
			deps/game-music-emu/gme/Dual_Resampler.cpp \
			deps/game-music-emu/gme/Effects_Buffer.cpp \
			deps/game-music-emu/gme/Fir_Resampler.cpp \
			deps/game-music-emu/gme/Gbs_Emu.cpp \
			deps/game-music-emu/gme/Gb_Apu.cpp \
			deps/game-music-emu/gme/Gb_Cpu.cpp \
			deps/game-music-emu/gme/Gb_Oscs.cpp \
			deps/game-music-emu/gme/gme.cpp \
			deps/game-music-emu/gme/Gme_File.cpp \
			deps/game-music-emu/gme/Gym_Emu.cpp \
			deps/game-music-emu/gme/Hes_Apu.cpp \
			deps/game-music-emu/gme/Hes_Cpu.cpp \
			deps/game-music-emu/gme/Hes_Emu.cpp \
			deps/game-music-emu/gme/Kss_Cpu.cpp \
			deps/game-music-emu/gme/Kss_Emu.cpp \
			deps/game-music-emu/gme/Kss_Scc_Apu.cpp \
			deps/game-music-emu/gme/M3u_Playlist.cpp \
			deps/game-music-emu/gme/Multi_Buffer.cpp \
			deps/game-music-emu/gme/Music_Emu.cpp \
			deps/game-music-emu/gme/Nes_Apu.cpp \
			deps/game-music-emu/gme/Nes_Cpu.cpp \
			deps/game-music-emu/gme/Nes_Fme7_Apu.cpp \
			deps/game-music-emu/gme/Nes_Namco_Apu.cpp \
			deps/game-music-emu/gme/Nes_Oscs.cpp \
			deps/game-music-emu/gme/Nes_Vrc6_Apu.cpp \
			deps/game-music-emu/gme/Nsfe_Emu.cpp \
			deps/game-music-emu/gme/Nsf_Emu.cpp \
			deps/game-music-emu/gme/Sap_Apu.cpp \
			deps/game-music-emu/gme/Sap_Cpu.cpp \
			deps/game-music-emu/gme/Sap_Emu.cpp \
			deps/game-music-emu/gme/Sms_Apu.cpp \
			deps/game-music-emu/gme/Snes_Spc.cpp \
			deps/game-music-emu/gme/Spc_Cpu.cpp \
			deps/game-music-emu/gme/Spc_Dsp.cpp \
			deps/game-music-emu/gme/Spc_Emu.cpp \
			deps/game-music-emu/gme/Spc_Filter.cpp \
			deps/game-music-emu/gme/Vgm_Emu.cpp \
			deps/game-music-emu/gme/Vgm_Emu_Impl.cpp \
			deps/game-music-emu/gme/Ym2413_Emu.cpp \
			deps/game-music-emu/gme/Ym2612_Emu.cpp 

SOURCES_C    := src/libretro.c \
				src/graphics.c \
				src/player.c \
				deps/libretro-common/compat/compat_fnmatch.c \
				deps/libretro-common/compat/compat_posix_string.c \
				deps/libretro-common/compat/compat_strcasestr.c \
				deps/libretro-common/compat/compat_strl.c \
				deps/libretro-common/file/archive_file_zlib.c \
				deps/libretro-common/file/archive_file.c \
				deps/libretro-common/file/retro_dirent.c \
				deps/libretro-common/file/retro_stat.c \
				deps/libretro-common/file/file_path.c \
				deps/libretro-common/file/nbio/nbio_stdio.c \
				deps/libretro-common/lists/dir_list.c \
				deps/libretro-common/lists/file_list.c \
				deps/libretro-common/lists/string_list.c \
				deps/libretro-common/streams/file_stream.c

OBJECTS := $(SOURCES_CXX:.cpp=.o) $(SOURCES_C:.c=.o)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) -o $@ $<
    
$(EXECUTABLE): $(OBJECTS)
	$(CXX) -o $@ $(OBJECTS) $(LDFLAGS)

all: $(EXECUTABLE)

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)

install:
	cp $(EXECUTABLE) ../libretro-super/dist/cores
	cp $(INFO) ../libretro-super/dist/info

.PHONY: clean install
