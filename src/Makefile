include ../sys.mk

all: APme$(EXE)

SRC := main.c \
       help.c \
       chatlog.c \
       config.c \
       regeng.c \
       util.c \
       items.c \
       cmd.c \
       txtbuf.c \
       console.c \
       aion.c \
       aion_trans.c \
       aion_sys.c \
       event.c \
       term.c

OBJ := $(patsubst %.c,%.o,$(SRC))

ifdef USE_MANIFEST
# Include the manifest file as a resource in apme.rc
apme.res : apme.rc apme.manifest
	$(WINDRES) --input $< --output $@ --output-format=coff

# Add the resource file to the objects
OBJ += apme.res
endif

# Add external dependencies
include $(EXTERN_DIR)/bsdqueue/module.mk
include $(EXTERN_DIR)/pcre/module.mk
include $(EXTERN_DIR)/iniparser/module.mk
include $(EXTERN_DIR)/wxwidgets/module.mk

APme$(EXE): $(OBJ) $(DEPS)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)
	$(STRIP) $@

.PHONY: clean
clean:
	rm -f $(OBJ) APme$(EXE)
