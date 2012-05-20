include ../../sys.mk
include ../../package.mk

all: INIPARSER

INIPARSER_PREFIX    := $(INIPARSER_PKGDIR)
INIPARSER_VER       := 3.1
INIPARSER_URL       := http://ndevilla.free.fr/iniparser/iniparser-$(INIPARSER_VER).tar.gz
INIPARSER_DIR       := $(CURDIR)/iniparser
INIPARSER_CONFIGURE := true
INIPARSER_MAKE_INSTALL := true

INIPARSER_CONFIG    := --disable-shared
ifdef XBUILD
INIPARSER_CONFIG    += --host=$(XBUILD_TARGET)
INIPARSER_CONFIG    += --build=$(XBUILD_MACH)
endif

$(eval $(call MAKE_PACKAGE,INIPARSER))

INIPARSER_FILES:=$(INIPARSER_DIR)/libiniparser.a
INIPARSER_FILES+=$(INIPARSER_DIR)/src/iniparser.h
INIPARSER_FILES+=$(INIPARSER_DIR)/src/dictionary.h

# Iniparser doesn't have a proper make install target
$(INIPARSER_TARGET_INSTALL): $(INIPARSER_PKGDIR)/libiniparser.a

$(INIPARSER_PKGDIR)/libiniparser.a:
	@mkdir -p "$(INIPARSER_PKGDIR)"
	for file in $(INIPARSER_FILES); do cp -v "$${file}" "$(INIPARSER_PKGDIR)"; done
    

clean: INIPARSER_clean 
	rm -rf "$(INIPARSER_PKGDIR)"
