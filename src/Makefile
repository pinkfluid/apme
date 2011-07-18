include ../sys.mk

CFLAGS+=-I${TOP_DIR}/extern

SRC:=parse.c util.c items.c aion.c cmd.c txtbuf.c console.c

OBJ:=$(patsubst %.c,%.o,$(SRC))

all: aptrack

aptrack: $(OBJ)
	$(CC) $(LDFLAGS) $(OBJ) -o $@

.PHONY: clean
clean:
	rm -f $(OBJ) aptrack aptrack.exe
