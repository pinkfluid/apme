include ../../sys.mk
include ../../package.mk

all: WXW

WXW_PREFIX      := $(PKG_DIR)/wxwidgets
WXW_VER         := 2.9.3
WXW_URL         := http://downloads.sourceforge.net/project/wxwindows/$(WXW_VER)/wxWidgets-$(WXW_VER).tar.bz2

WXW_CONFIG      := --disable-shared
# --enable-monolithic builds a SINGLE monolithic library
WXW_CONFIG      += --enable-monolithic
ifndef NATIVE_BUILD
WXW_CONFIG     += --host=$(XBUILD_TARGET)
WXW_CONFIG     += --build=$(XBUILD_MACH)
endif

$(eval $(call MAKE_PACKAGE,WXW))

$(WXW_TARGET_DONE): $(WXW_PREFIX)/build.mk

$(WXW_PREFIX)/build.mk:
	echo CFLAGS += $(shell "$(WXW_PREFIX)/wx-config" --cflags) > "$@"
	echo LDFLAGS += $(shell "$(WXW_PREFIX)/wx-config" --libs) >> "$@"

fetch: $(FETCH_PKGS)

clean: WXW_clean 
	rm -rf "$(WXW_PREFIX)"
