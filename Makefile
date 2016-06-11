CC = gcc
CXX = g++
INCFLAGS= -I. -I.src/libretro -Igame-music-emu/gme
CFLAGS=-c -Wall $(INCFLAGS) -fpic
CXXFLAGS=-c -Wall $(INCFLAGS) -fpic
LDFLAGS= -shared

EXECUTABLE= gme_libretro.dll

SOURCES_CXX := game-music-emu/gme/Ay_Apu.cpp \
			game-music-emu/gme/Ay_Cpu.cpp \
			game-music-emu/gme/Ay_Emu.cpp \
			game-music-emu/gme/Blip_Buffer.cpp \
			game-music-emu/gme/Classic_Emu.cpp \
			game-music-emu/gme/Data_Reader.cpp \
			game-music-emu/gme/Dual_Resampler.cpp \
			game-music-emu/gme/Effects_Buffer.cpp \
			game-music-emu/gme/Fir_Resampler.cpp \
			game-music-emu/gme/Gbs_Emu.cpp \
			game-music-emu/gme/Gb_Apu.cpp \
			game-music-emu/gme/Gb_Cpu.cpp \
			game-music-emu/gme/Gb_Oscs.cpp \
			game-music-emu/gme/gme.cpp \
			game-music-emu/gme/Gme_File.cpp \
			game-music-emu/gme/Gym_Emu.cpp \
			game-music-emu/gme/Hes_Apu.cpp \
			game-music-emu/gme/Hes_Cpu.cpp \
			game-music-emu/gme/Hes_Emu.cpp \
			game-music-emu/gme/Kss_Cpu.cpp \
			game-music-emu/gme/Kss_Emu.cpp \
			game-music-emu/gme/Kss_Scc_Apu.cpp \
			game-music-emu/gme/M3u_Playlist.cpp \
			game-music-emu/gme/Multi_Buffer.cpp \
			game-music-emu/gme/Music_Emu.cpp \
			game-music-emu/gme/Nes_Apu.cpp \
			game-music-emu/gme/Nes_Cpu.cpp \
			game-music-emu/gme/Nes_Fme7_Apu.cpp \
			game-music-emu/gme/Nes_Namco_Apu.cpp \
			game-music-emu/gme/Nes_Oscs.cpp \
			game-music-emu/gme/Nes_Vrc6_Apu.cpp \
			game-music-emu/gme/Nsfe_Emu.cpp \
			game-music-emu/gme/Nsf_Emu.cpp \
			game-music-emu/gme/Sap_Apu.cpp \
			game-music-emu/gme/Sap_Cpu.cpp \
			game-music-emu/gme/Sap_Emu.cpp \
			game-music-emu/gme/Sms_Apu.cpp \
			game-music-emu/gme/Snes_Spc.cpp \
			game-music-emu/gme/Spc_Cpu.cpp \
			game-music-emu/gme/Spc_Dsp.cpp \
			game-music-emu/gme/Spc_Emu.cpp \
			game-music-emu/gme/Spc_Filter.cpp \
			game-music-emu/gme/Vgm_Emu.cpp \
			game-music-emu/gme/Vgm_Emu_Impl.cpp \
			game-music-emu/gme/Ym2413_Emu.cpp \
			game-music-emu/gme/Ym2612_Emu.cpp 

SOURCES_C    := src/libretro/libretro.c

OBJECTS := $(SOURCES_CXX:.cpp=.o) $(SOURCES_C:.c=.o)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

all: $(EXECUTABLE)
    
$(EXECUTABLE): $(OBJECTS)
	$(CXX) -o $@ $(OBJECTS) $(LDFLAGS)
	
clean:
	rm -f $(OBJECTS) $(EXECUTABLE)

.PHONY: clean
