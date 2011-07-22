include ../sys.mk

SRC:=main.c chatlog.c regeng.c util.c items.c aion.c cmd.c txtbuf.c console.c

all: aptrack

$(eval $(PCRE_INCLUDE))

OBJ:=$(patsubst %.c,%.o,$(SRC))

aptrack: $(PCRE_DEP) $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(OBJ) aptrack aptrack.exe
