
# Extract the top directory 
TOP_DIR_SHORT:=$(dir $(lastword $(MAKEFILE_LIST)))
TOP_DIR:=$(abspath $(TOP_DIR_SHORT))

# Allow the flags to be overwritten by config.mk
CFLAGS:=
LDFLAGS:=

EXTERN_DIR:=$(TOP_DIR)/extern
EXTERN_DIR_SHORT:=$(TOP_DIR_SHORT)/extern
EXTERN_BUILD:=$(EXTERN_DIR)/build

include $(TOP_DIR)/config.mk

# Get the OS version
UNAME:=$(shell uname -s)

ifneq ($(findstring CYGWIN, $(UNAME)),)
    CFLAGS+=-DSYS_WINDOWS -DOS_CYGWIN
    BUILTIN_PCRE:=true
endif

ifneq ($(findstring MINGW, $(UNAME)),)
# MinGW doesn't have a default compiler so we have to force it to GCC
    CC:=gcc
    LD:=gcc
    CFLAGS+=-DSYS_WINDOWS -DOS_MINGW 
    BUILTIN_PCRE:=true
endif

ifneq ($(findstring DragonFly, $(UNAME)),)
    CFLAGS+=-DSYS_UNIX -DOS_DRAGONFLY
endif

ifneq ($(findstring Linux, $(UNAME)),)
    CFLAGS+=-DSYS_UNIX -DOS_LINUX
endif

include $(EXTERN_DIR)/pcre/sys_pcre.mk

CFLAGS+=-I$(EXTERN_DIR)

