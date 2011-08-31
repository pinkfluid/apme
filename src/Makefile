include ../sys.mk

SRC:=main.c help.c chatlog.c regeng.c util.c items.c cmd.c txtbuf.c console.c aion.c aion_trans.c aion_sys.c

all: aptrack

$(eval $(PCRE_INCLUDE))

OBJ:=$(patsubst %.c,%.o,$(SRC))

aptrack: $(PCRE_DEP) $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(OBJ) aptrack aptrack.exe
