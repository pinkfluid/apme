
# Extract the top directory 
TOP_DIR_SHORT:=$(dir $(lastword $(MAKEFILE_LIST)))
TOP_DIR:=$(abspath $(TOP_DIR_SHORT))

# Allow the flags to be overwritten by config.mk
CFLAGS:=
LDFLAGS:=
STRIP:=strip

EXTERN_DIR:=$(TOP_DIR)/extern
EXTERN_DIR_SHORT:=$(TOP_DIR_SHORT)/extern
EXTERN_BUILD:=$(EXTERN_DIR)/build

include $(TOP_DIR)/config.mk

# Get the OS version
UNAME:=$(shell uname -s)


ifneq ($(findstring CYGWIN, $(UNAME)),)
    SYS_CFLAGS      :=  -DSYS_WINDOWS -DOS_CYGWIN
    BUILTIN_PCRE    :=  true
    USE_MANIFEST    :=  true
    WINDRES         :=  windres

    XBUILD          :=  $(XBUILD_CYGWIN)
    XBUILD_TARGET   ?=  i686-w64-mingw32
    XBUILD_ERROR    :=  Unable to find the 32-bit MinGW compiler. Please install the mingw64-i686-gcc-core package
endif

ifneq ($(findstring MINGW, $(UNAME)),)
# MinGW doesn't have a default compiler so we have to force it to GCC
    SYS_CFLAGS      :=  -DSYS_WINDOWS -DOS_MINGW 
    CC              :=  gcc
    LD              :=  gcc
    BUILTIN_PCRE:=true
endif

ifneq ($(findstring DragonFly, $(UNAME)),)
    SYS_CFLAGS      :=  -DSYS_UNIX -DOS_DRAGONFLY
    BUILTIN_PCRE    :=  true

    XBUILD          :=  $(XBUILD_DRAGONFLY)
    XBUILD_TARGET   ?=  i686-w64-mingw32
    XBUILD_ERROR    :=  Unable to find the 32-bit MinGW compiler. 
endif

ifneq ($(findstring Linux, $(UNAME)),)
    SYS_CFLAGS      :=  -DSYS_UNIX -DOS_LINUX

# Linux actually has these available
    XBUILD          :=  $(XBUILD_LINUX)
    XBUILD_TARGET   ?=  i686-w64-mingw32
    XBUILD_ERROR    :=  Unable to find the 32-bit MinGW compiler. Please install the gcc-mingw-w64 package
endif

# Check if we should do a MinGW cross compile
ifdef XBUILD
    # Override the system CFLAGS 
    SYS_CFLAGS          :=  -DSYS_WINDOWS -DOS_MINGW
    USE_MANIFEST        :=  true
    BUILTIN_PCRE        :=  true

    XBUILD_CC           ?=  $(XBUILD_TARGET)-gcc
    XBUILD_LD           ?=  $(XBUILD_TARGET)-gcc
    XBUILD_STRIP        ?=  $(XBUILD_TARGET)-strip
    XBUILD_WINDRES      ?=  $(XBUILD_TARGET)-windres
    # This will be used for the --build flag to configure; auto-guess it using the -dumpmachine option of GCC
    XBUILD_GUEST        ?=  $(shell gcc -dumpmachine)

    ifneq ($(findstring GCC,$(shell $(XBUILD_CC) --version)),GCC)
        $(error $(XBUILD_ERROR))
    endif

    CC                  :=  $(XBUILD_CC)
    LD                  :=  $(XBUILD_LD)
    STRIP               :=  $(XBUILD_STRIP)
    WINDRES             :=  $(XBUILD_WINDRES)
    PCRE_EXTRA_CONFIG   := --host=$(XBUILD_TARGET) --build=$(XBUILD_GUEST)
    EXE                 := .exe
endif

include $(EXTERN_DIR)/pcre/sys_pcre.mk

CFLAGS+=-I$(EXTERN_DIR) $(SYS_CFLAGS)

