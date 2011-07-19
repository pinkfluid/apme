
# Extract the top directory 
TOP_DIR:=$(abspath $(dir $(lastword $(MAKEFILE_LIST))))

# Allow the flags to be overwritten by config.mk
CFLAGS:=
LDFLAGS:=

include $(TOP_DIR)/config.mk
export TOP_DIR

# Get the OS version
UNAME:=$(shell uname -s)

ifneq ($(findstring CYGWIN, $(UNAME)),)
    CFLAGS+=-DSYS_WINDOWS -DOS_CYGWIN
endif

ifneq ($(findstring MINGW, $(UNAME)),)
    CFLAGS+=-DSYS_WINDOWS -DOS_MINGW
endif

ifneq ($(findstring DragonFly, $(UNAME)),)
    CFLAGS+=-DSYS_UNIX -DOS_DRAGONFLY
endif

ifneq ($(findstring Linux, $(UNAME)),)
    CFLAGS+=-DSYS_UNIX -DOS_LINUX
endif


EXTERN_DIR:=$(TOP_DIR)/extern
EXTERN_BUILD:=$(EXTERN_DIR)/build
