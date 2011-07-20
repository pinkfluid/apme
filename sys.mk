
# Extract the top directory 
TOP_DIR_SHORT:=$(dir $(lastword $(MAKEFILE_LIST)))
TOP_DIR:=$(abspath $(TOP_DIR_SHORT))

# Allow the flags to be overwritten by config.mk
CFLAGS:=
LDFLAGS:=

include $(TOP_DIR)/config.mk
export TOP_DIR

EXTERN_DIR:=$(TOP_DIR)/extern
EXTERN_DIR_SHORT:=$(TOP_DIR_SHORT)/extern
EXTERN_BUILD:=$(EXTERN_DIR)/build

# Get the OS version
UNAME:=$(shell uname -s)

ifneq ($(findstring CYGWIN, $(UNAME)),)
    CFLAGS+=-DSYS_WINDOWS -DOS_CYGWIN
    BUILTIN_PCRE:=true
endif

ifneq ($(findstring MINGW, $(UNAME)),)
    CFLAGS+=-DSYS_WINDOWS -DOS_MINGW 
# MinDW doesn't have a default compiler so we have to force it to GCC
    CC:=gcc
    LD:=gcc
    BUILTIN_PCRE:=true
endif

ifneq ($(findstring DragonFly, $(UNAME)),)
    CFLAGS+=-DSYS_UNIX -DOS_DRAGONFLY
endif

ifneq ($(findstring Linux, $(UNAME)),)
    CFLAGS+=-DSYS_UNIX -DOS_LINUX
endif

ifneq ($(BUILTIN_PCRE),)
    PCRE_CONFIG:=$(EXTERN_DIR)/pcre/pcre-config
    # If pcre-config exists, use that to extract flags
    ifneq ($(wildcard $PCRE_CONFIG), )
        PCRE_CFLAGS:=$(shell $(PCRE_CONFIG) --cflags-posix)
        PCRE_LDFLAGS:=$(shell $(PCRE_CONFIG) --libs-posix)
    else
        PCRE_CFLAGS:=-DPCRE_STATIC -I$(EXTERN_DIR)/pcre
        PCRE_LDFLAGS:=-L$(EXTERN_DIR)/pcre -lpcreposix -lpcre
    endif
else
    PCRE_CFLAGS:=
    PCRE_LDFLAGS:=-lpcreposix -lpcre
endif 

CFLAGS+=-I$(EXTERN_DIR)

