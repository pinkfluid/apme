include ../../sys.mk
include ../../package.mk

all: INIPARSER

INIPARSER_PREFIX    := $(PKG_DIR)/iniparser
INIPARSER_VER       := 3.1
INIPARSER_URL       := http://ndevilla.free.fr/iniparser/iniparser-$(INIPARSER_VER).tar.gz
INIPARSER_DIR       := $(CURDIR)/iniparser

INIPARSER_CONFIG    := --disable-shared
ifdef XBUILD
INIPARSER_CONFIG    += --host=$(XBUILD_TARGET)
INIPARSER_CONFIG    += --build=$(XBUILD_MACH)
endif

$(eval $(call MAKE_PACKAGE,INIPARSER))

INIPARSER_CONFIGURE := @true
INIPARSER_MAKE_INSTALL := @true
INIPARSER_MAKE := (cd "$(INIPARSER_BUILD)" && $(MAKE) CC="$(CC)" CFLAGS="$(CFLAGS)" AR="$(AR)" LD="$(LD)" libiniparser.a)
INIPARSER_FILES:=$(INIPARSER_DIR)/libiniparser.a
INIPARSER_FILES+=$(INIPARSER_DIR)/src/iniparser.h
INIPARSER_FILES+=$(INIPARSER_DIR)/src/dictionary.h

# Iniparser doesn't have a proper make install target
$(INIPARSER_TARGET_INSTALL): $(INIPARSER_PREFIX)/libiniparser.a

$(INIPARSER_PREFIX)/libiniparser.a: $(INIPARSER_TARGET_BUILD)
	@mkdir -p "$(INIPARSER_PREFIX)"
	for file in $(INIPARSER_FILES); do cp -v "$${file}" "$(INIPARSER_PREFIX)"; done

$(INIPARSER_TARGET_DONE): $(INIPARSER_PREFIX)/build.mk

$(INIPARSER_PREFIX)/build.mk:
	echo 'CFLAGS  += -I"$(PKG_DIR)/iniparser"' > "$@"
	echo 'LDFLAGS += -L"$(PKG_DIR)/iniparser" -liniparser' >> "$@"

    
clean: INIPARSER_clean 
	rm -rf "$(INIPARSER_PREFIX)"
