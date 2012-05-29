include ../../sys.mk
include ../../package.mk

all: PCRE

PCRE_PREFIX     := $(PKG_DIR)/pcre
PCRE_VER        := 8.30
PCRE_URL        := ftp://ftp.csx.cam.ac.uk/pub/software/programming/pcre/pcre-$(PCRE_VER).tar.bz2

PCRE_CONFIG     := --disable-shared
PCRE_CONFIG     += --disable-cpp
PCRE_CONFIG     += --enable-utf8
PCRE_CONFIG     += --enable-unicode-properties
ifndef NATIVE_BUILD
PCRE_CONFIG     += --host=$(XBUILD_TARGET)
PCRE_CONFIG     += --build=$(XBUILD_MACH)
endif

$(eval $(call MAKE_PACKAGE,PCRE))
    
$(PCRE_TARGET_DONE): $(PCRE_PREFIX)/build.mk

$(PCRE_PREFIX)/build.mk: $(PCRE_TARGET_INSTALL)
	echo CFLAGS += $(shell "$(PCRE_PREFIX)/pcre-config" --cflags-posix) > "$@"
	echo LDFLAGS += $(shell "$(PCRE_PREFIX)/pcre-config" --libs-posix) >> "$@"


clean: PCRE_clean 
	rm -rf "$(PCRE_PREFIX)"
