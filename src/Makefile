include ../sys.mk

SRC:=main.c help.c chatlog.c regeng.c util.c items.c aion.c cmd.c txtbuf.c console.c event.c

all: APme 

$(eval $(PCRE_INCLUDE))

OBJ:=$(patsubst %.c,%.o,$(SRC))

APme: $(PCRE_DEP) $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(OBJ) APme APme.exe
