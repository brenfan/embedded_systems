CXX = arm-linux-gnueabihf-gcc
CXXFLAGS = -g -Wall -Werror -std=c99 -D _POSIX_C_SOURCE=200809L -I.

DEPS = audio_mixer.h beat.h io.h udp.h
OBJ = audio_mixer.o beat.o io.o udp.o

LFLAGS = -L$(HOME)/cmpt433/public/asound_lib_BBB -lpthread -lasound

all: beatbox install sounds web

default: all
	@echo beatbox > .gitignore

web:
	cp -r node/* ~/cmpt433/public/myApps/beatbox-server-copy/

sounds:
	cp wav/* ~/cmpt433/public/myApps/beatbox-wav-files/ 

%.o: %.c $(DEPS)
	$(CXX) -c -o $@ $< $(CFLAGS) $(LFLAGS)

beatbox: main.c $(OBJ)
	$(CXX) $(CXXFLAGS) main.c $(OBJ) -o beatbox $(LFLAGS)

install: beatbox
	cp beatbox ~/cmpt433/public/myApps

.PHONY: clean

clean:
	@rm -f beatbox *.o noworky
