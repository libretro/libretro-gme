STATIC_LINKING := 0
AR             := ar

#detect platform
ifeq ($(platform),)
platform = unix
ifeq ($(shell uname -a),)
   platform = win
else ifneq ($(findstring win,$(shell uname -a)),)
   platform = win
else ifneq ($(findstring MINGW,$(shell uname -a)),)
   platform = mingw
else ifneq ($(findstring Darwin,$(shell uname -a)),)
   platform = osx
endif
endif

#detect architecture
ifeq ($(shell uname -m),)
   arch = i686
else
   arch = ($(shell uname -m)
endif

#set extension and lib path

ifneq ($(filter $(platform),win mingw),)
EXT = .dll
else ifeq ($(platform), unix)
EXT = .so
fpic = -fPIC
endif

CC = gcc
CXX = g++
INCFLAGS= -I. -Ideps/libretro-common/include -Ideps/game-music-emu/gme  -Ideps/zlib-1.2.8 -Ideps/zlib-1.2.8/contrib/minizip

CFLAGS += -Wall -std=c99 $(INCFLAGS)
CXXFLAGS += -Wall $(INCFLAGS)
LDFLAGS += -shared

TARGET_NAME := gme_libretro
EXECUTABLE = $(TARGET_NAME)$(EXT)
INFO = $(TARGET_NAME).info

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
				src/playlist.c \
				deps/zlib-1.2.8/adler32.c \
				deps/zlib-1.2.8/crc32.c \
				deps/zlib-1.2.8/inflate.c \
				deps/zlib-1.2.8/inftrees.c \
				deps/zlib-1.2.8/inffast.c \
				deps/zlib-1.2.8/zutil.c \
				deps/zlib-1.2.8/contrib/minizip/ioapi.c \
				deps/zlib-1.2.8/contrib/minizip/unzip.c

OBJECTS := $(SOURCES_CXX:.cpp=.o) $(SOURCES_C:.c=.o)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(fpic) -c -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) $(fpic) -c -o $@ $<

$(EXECUTABLE): $(OBJECTS)
	$(CXX) -o $@ $(OBJECTS) $(LDFLAGS)

all: $(EXECUTABLE)

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)

install:
	cp $(EXECUTABLE) ~/.config/retroarch/cores
	cp $(INFO) ~/.config/retroarch/cores
	
.PHONY: clean install
