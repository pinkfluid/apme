include ../../sys.mk

# Parameters
PCRE_VER:=8.12
PCRE_DIR:=pcre-$(PCRE_VER)
PCRE_ARCHIVE:=$(PCRE_DIR).tar.gz
PCRE_URL:=http://noom.ath.cx/mitja/$(PCRE_ARCHIVE)
PCRE_FILES:=.libs/libpcre.a .libs/libpcreposix.a pcre.h pcreposix.h

# Do not touch this
PCRE_BUILD:=$(EXTERN_BUILD)/$(PCRE_DIR)
PCRE_FETCH_DONE:=$(EXTERN_BUILD)/.pcre_fetch_done
PCRE_EXTRACT_DONE:=$(PCRE_BUILD)/.extract_done
PCRE_CONFIG_DONE:=$(PCRE_BUILD)/.config_done
PCRE_BUILD_DONE:=$(PCRE_BUILD)/.build_done
PCRE_INSTALL_DONE:=$(CURDIR)/.install_done

PCRE_SRC:=$(foreach file,$(PCRE_FILES), $(PCRE_BUILD)/$(file))
PCRE_INSTALL:=$(foreach file,$(PCRE_FILES), "$(notdir $(file))")

.PHONY: all clean dist-clean fetch build extract configure install

all: install

clean:
	rm -rf "$(PCRE_BUILD)"

dist-clean: clean
	rm -f $(PCRE_INSTALL) "$(PCRE_FETCH_DONE)" "$(PCRE_INSTALL_DONE)" "$(EXTERN_BUILD)/$(PCRE_ARCHIVE)"

fetch: $(PCRE_FETCH_DONE)
extract: $(PCRE_EXTRACT_DONE)
configure: $(PCRE_CONFIGURE_DONE)
build: $(PCRE_BUILD_DONE)
install: $(PCRE_INSTALL_DONE)

$(PCRE_FETCH_DONE): $(EXTERN_BUILD)/$(PCRE_ARCHIVE)
	@mkdir -p "$(EXTERN_BUILD)"
	(cd "$(EXTERN_BUILD)"; $(FETCH_CMD) "$(PCRE_URL)")
	touch "$(PCRE_FETCH_DONE)"

$(PCRE_EXTRACT_DONE): $(PCRE_FETCH_DONE)
	@$(MAKE) clean
	cd "$(EXTERN_BUILD)" ; tar xzvf "$(PCRE_ARCHIVE)"
	@touch "$(PCRE_EXTRACT_DONE)"

$(PCRE_CONFIG_DONE): $(PCRE_EXTRACT_DONE)
	cd "$(PCRE_BUILD)" ; ./configure
	@touch "$(PCRE_CONFIG_DONE)"

$(PCRE_BUILD_DONE): $(PCRE_CONFIG_DONE)
	cd "$(PCRE_BUILD)" ; make
	@touch "$(PCRE_BUILD_DONE)"

$(PCRE_INSTALL_DONE):
	@$(MAKE) build
	@cp -v $(PCRE_SRC) .
	@touch "$(PCRE_INSTALL_DONE)"
