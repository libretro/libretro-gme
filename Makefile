STATIC_LINKING := 0
AR             := ar

CORE_DIR       := .

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

TARGET_NAME := gme

include Makefile.common

CFLAGS   += -Wall -std=c99 $(INCFLAGS)
CXXFLAGS += -Wall $(INCFLAGS)
LDFLAGS  += -shared

EXECUTABLE = $(TARGET_NAME)_libretro$(EXT)
INFO = $(TARGET_NAME).info

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

test:
	retroarch --verbose -L ~/.config/retroarch/cores/$(EXECUTABLE) "./test/Final Fantasy 6 [ff6].zip"

.PHONY: clean install test
