CXX = arm-linux-gnueabihf-gcc
CXXFLAGS = -g -Wall -Werror -std=c99 -D _POSIX_C_SOURCE=200809L -I. -pthread -lm

DEPS = sort.h display.h pot.h udp.h
OBJ = sort.o display.o pot.o udp.o\

all: sorter install noworky

noworky:
	$(CXX) $(CXXFLAGS) noworky.c -o noworky
	cp noworky ~/cmpt433/public/myApps
	@echo noworky >> .gitignore

default: sorter install
	@echo sorter > .gitignore

%.o: %.c $(DEPS)
	$(CXX) -c -o $@ $< $(CFLAGS)

sorter: main.c $(OBJ)
	$(CXX) $(CXXFLAGS) main.c $(OBJ) -o sorter

install: sorter
	cp sorter ~/cmpt433/public/myApps

.PHONY: clean

clean:
	@rm -f sorter *.o noworky
