SRC:=parse.c util.c items.c aion.c cmd.c txtbuf.c

CFLAGS:=-Wall -O2 
LDFLAGS:=

#####################

UNAME:=$(shell uname -s)

ifneq ($(findstring CYGWIN, $(UNAME)),)
    CFLAGS+=-DSYS_WINDOWS
endif

ifneq ($(findstring DragonFly, $(UNAME)),)
    CFLAGS+=-DSYS_UNIX -DOS_DRAGONFLY
endif

ifneq ($(findstring Linux, $(UNAME)),)
    CFLAGS+=-DSYS_UNIX -DOS_LINUX
endif

OBJ:=$(patsubst %.c,%.o,$(SRC))

all: aptrack 

aptrack: $(OBJ)
	$(CC) $(LDFLAGS) $(OBJ) -o $@

.PHONY: clean
clean:
	rm -f $(OBJ) parse parse.exe
