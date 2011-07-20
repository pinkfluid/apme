include ../../sys.mk

# Parameters
PCRE_VER:=8.12
PCRE_DIR:=pcre-$(PCRE_VER)
PCRE_ARCHIVE:=$(EXTERN_BUILD)/$(PCRE_DIR).tar.gz
PCRE_URL:=http://noom.ath.cx/mitja/$(PCRE_DIR).tar.gz
PCRE_FILES:=pcre-config .libs/libpcre.a .libs/libpcreposix.a pcre.h pcreposix.h

# Do not touch this
PCRE_BUILD:=$(EXTERN_BUILD)/$(PCRE_DIR)

PCRE_FETCH_DONE:=$(EXTERN_BUILD)/.pcre_fetch_done
PCRE_EXTRACT_DONE:=$(PCRE_BUILD)/.pcre_extract_done
PCRE_CONFIG_DONE:=$(PCRE_BUILD)/.pcre_config_done
PCRE_BUILD_DONE:=$(PCRE_BUILD)/.pcre_build_done
PCRE_INSTALL_DONE:=$(CURDIR)/.pcre_install_done

PCRE_SRC:=$(foreach file,$(PCRE_FILES), $(PCRE_BUILD)/$(file))
PCRE_INSTALL:=$(foreach file,$(PCRE_FILES), "$(notdir $(file))")

.PHONY: all clean dist-clean fetch build extract configure install

all: install

dist-clean: clean
	rm -f $(PCRE_INSTALL) "$(PCRE_INSTALL_DONE)" 

clean:
	rm -rf "$(PCRE_BUILD)"

fetch: $(PCRE_FETCH_DONE)
extract: $(PCRE_EXTRACT_DONE)
configure: $(PCRE_CONFIGURE_DONE)
build: $(PCRE_BUILD_DONE)
install: $(PCRE_INSTALL_DONE)

$(PCRE_ARCHIVE):
	@mkdir -p "$(EXTERN_BUILD)"
	(cd "$(EXTERN_BUILD)"; $(FETCH_CMD) "$(PCRE_URL)")

$(PCRE_FETCH_DONE): $(PCRE_ARCHIVE)
	@touch $(PCRE_FETCH_DONE)

$(PCRE_EXTRACT_DONE): $(PCRE_FETCH_DONE)
	@$(MAKE) clean
	cd "$(EXTERN_BUILD)" ; tar xzvf "$(PCRE_ARCHIVE)"
	@touch "$(PCRE_EXTRACT_DONE)"

# We need to use --libdir and --include dir with configure so pcre-config
# returns the correct filenames
# Also use --disable-shared so pcre-config returns the static linking 
# options
$(PCRE_CONFIG_DONE): $(PCRE_EXTRACT_DONE)
	cd "$(PCRE_BUILD)" ; ./configure --libdir=$(EXTERN_DIR)/pcre --includedir=$(EXTERN_DIR)/pcre --disable-shared
	@touch "$(PCRE_CONFIG_DONE)"

$(PCRE_BUILD_DONE): $(PCRE_CONFIG_DONE)
	cd "$(PCRE_BUILD)" ; make
	@touch "$(PCRE_BUILD_DONE)"

# Nifty trick, if file exists, add it to dependencies, otherwise ignore it 
$(PCRE_INSTALL_DONE): $(if $(wildcard $(PCRE_BUILD_DONE)), $(PCRE_BUILD_DONE), )
	@$(MAKE) build
	@cp -v $(PCRE_SRC) .
	@touch "$(PCRE_INSTALL_DONE)"
