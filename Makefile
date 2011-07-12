SRC:=parse.c util.c items.c aion.c

CFLAGS:=-Wall -O2 

#####################

UNAME:=$(shell uname -s)

ifneq ($(findstring CYGWIN, $(UNAME)),)
    CFLAGS+=-DSYS_WINDOWS -mwindows
endif

ifneq ($(findstring DragonFly, $(UNAME)),)
    CFLAGS+=-DSYS_UNIX -DOS_DRAGONFLY
endif

ifneq ($(findstring Linux, $(UNAME)),)
    CFLAGS+=-DSYS_UNIX -DOS_LINUX
endif

OBJ:=$(patsubst %.c,%.o,$(SRC))

all: parse

parse: $(OBJ)

.PHONY: clean
clean:
	rm -f $(OBJ) parse parse.exe
