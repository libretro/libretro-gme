CC = gcc
CXX = g++
INCFLAGS= -I. -I.src/libretro -Ideps/game-music-emu/gme
CFLAGS=-c -Wall $(INCFLAGS)
CXXFLAGS=-c -Wall $(INCFLAGS)
LDFLAGS= -shared

EXECUTABLE= gme_libretro.dll

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

SOURCES_C    := src/libretro/libretro.c

OBJECTS := $(SOURCES_CXX:.cpp=.o) $(SOURCES_C:.c=.o)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

all: $(EXECUTABLE)
    
$(EXECUTABLE): $(OBJECTS)
	$(CXX) -o $@ $(OBJECTS) $(LDFLAGS)

install:
	cp $(EXECUTABLE) ../libretro-super/dist/cores
	
clean:
	rm -f $(OBJECTS) $(EXECUTABLE)

.PHONY: clean install
