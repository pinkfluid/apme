SRC:=parse.c util.c items.c group.c

CFLAGS:=-Wall -O2 -mwindows

#####################

SYS:=$(shell uname -s)

OBJ:=$(patsubst %.c,%.o,$(SRC))

all: parse

parse: $(OBJ)

.PHONY: clean
clean:
	rm -f $(OBJ) parse parse.exe
