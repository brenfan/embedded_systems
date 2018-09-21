CXX = arm-linux-gnueabihf-gcc
CXXFLAGS = -g -Wall -Werror -std=c99 -D _POSIX_C_SOURCE=200809L -I.

DEPS = io.h
OBJ = io.o

default: hello install
	@echo hello > .gitignore

%.o: %.c $(DEPS)
	$(CXX) -c -o $@ $< $(CFLAGS)

hello: hello.c $(OBJ)
	$(CXX) $(CXXFLAGS) hello.c $(OBJ) -o hello

install: hello
	cp hello ~/cmpt433/public/myApps

.PHONY: clean

clean:
	@rm -f hello *.o
