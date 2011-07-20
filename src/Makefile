include ../sys.mk

CFLAGS+=$(PCRE_CFLAGS)
LDFLAGS+=$(PCRE_LDFLAGS)

SRC:=parse.c util.c items.c aion.c cmd.c txtbuf.c console.c

OBJ:=$(patsubst %.c,%.o,$(SRC))

all: aptrack

aptrack: $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(OBJ) aptrack aptrack.exe
