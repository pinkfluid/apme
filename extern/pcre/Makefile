include ../../sys.mk
include ../../package.mk

ifdef BUILTIN_PCRE
all: PCRE
else
all: 
	@echo Will use system PCRE.
endif
	
PCRE_PREFIX     := $(PCRE_PKGDIR)
PCRE_VER        := 8.30
PCRE_URL        := ftp://ftp.csx.cam.ac.uk/pub/software/programming/pcre/pcre-$(PCRE_VER).tar.bz2

PCRE_CONFIG     := --disable-shared
PCRE_CONFIG     += --disable-cpp
PCRE_CONFIG     += --enable-utf8
PCRE_CONFIG     += --enable-unicode-properties
ifdef XBUILD
PCRE_CONFIG     += --host=$(XBUILD_TARGET)
PCRE_CONFIG     += --build=$(XBUILD_MACH)
endif

$(eval $(call MAKE_PACKAGE,PCRE))

clean: PCRE_clean 
	rm -rf "$(PCRE_PKGDIR)"
