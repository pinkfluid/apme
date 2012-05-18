FETCH := wget -ct0

#
# Standar configure/make/make install template
#
define CHECK_DEP

ifndef $2_TARGET
$$(error $1 needs $2, but it is not defined yet. Please consider changing the order of rules generation)
endif

endef

define MAKE_PACKAGE

# System wide config flags
$1_BASE_CONFIG := --prefix="$($1_PREFIX)"
$1_BASE_CONFIG += --bindir="$($1_PREFIX)"
$1_BASE_CONFIG += --includedir="$($1_PREFIX)"
$1_BASE_CONFIG += --libdir="$($1_PREFIX)"

# Extract the PKG name from URL
ifndef $1_PKG
$1_PKG := $$(notdir $$($1_URL))
endif

ifndef $1_PREFIX
$$(error $1_PREFIX is a required setting)
endif

# Extract the PKG name from the PKG name
ifndef $1_DIR
$1_DIR := $$(notdir $$($1_PKG))
$1_DIR := $$($1_DIR:.tar.gz=)
$1_DIR := $$($1_DIR:.tar.bz2=)
$1_DIR := $(CURDIR)/$$($1_DIR)
endif

# If not explicitly defined, the build folder is the same as "DIR"
ifndef $1_BUILD
$1_BUILD := $$($1_DIR)
endif

# Pseudo targets, these must not be phony targets so dependencies work
$1_PSEUDO_DEPS      := $$($1_DIR)/.$1_0_deps
$1_PSEUDO_EXTRACT   := $$($1_DIR)/.$1_1_extract
$1_PSEUDO_CONFIGURE := $$($1_DIR)/.$1_2_configure
$1_PSEUDO_BUILD     := $$($1_DIR)/.$1_3_build
$1_PSEUDO_INSTALL   := $$($1_DIR)/.$1_4_install
$1_PSEUDO_DONE      := $$($1_DIR)/.$1_5_done

# Define targets for each phase
$1_TARGET_DEPS      ?= $$($1_PSEUDO_DEPS)
$1_TARGET_EXTRACT   ?= $$($1_PSEUDO_EXTRACT)
$1_TARGET_CONFIGURE ?= $$($1_PSEUDO_CONFIGURE)
$1_TARGET_BUILD     ?= $$($1_PSEUDO_BUILD)
$1_TARGET_INSTALL   ?= $$($1_PSEUDO_INSTALL)
$1_TARGET_DONE      ?= $$($1_PSEUDO_DONE)

# Generic target that builds all other targets:
# This must not contain PHONY targets -- this is mainly used for dependencies
$1_TARGET           := $$($1_TARGET_DONE)

# Check if all dependencies exist
$(foreach DEP,$($1_NEEDS),$(call CHECK_DEP,$1,$(DEP)))

# Automatically add dependencies to the DEPS target -- use a recursively
# expanded variable since the dependencies might not be definde yet
$1_DEPS = $(foreach DEP,$($1_NEEDS),$$($(DEP)_TARGET))
$$($1_TARGET_DEPS): $$($1_DEPS)

# Other default targets
$$($1_TARGET_DEPS): $$($1_PKG)
$$($1_TARGET_EXTRACT): $$($1_TARGET_DEPS)
$$($1_TARGET_CONFIGURE): $$($1_TARGET_EXTRACT)
$$($1_TARGET_BUILD): $$($1_TARGET_CONFIGURE)
$$($1_TARGET_INSTALL): $$($1_TARGET_BUILD)
$$($1_TARGET_DONE): $$($1_TARGET_INSTALL)


# Some variables used by the standard targets
$1_MAKE ?= (cd "$$($1_BUILD)" && $(MAKE) all)
$1_MAKE_INSTALL ?= (cd "$$($1_BUILD)" && $(MAKE) install)
# This line effectively squashes the installation to a single directory
$1_CONFIGURE ?= (cd "$$($1_BUILD)" && ./configure $$($1_BASE_CONFIG) $$(SYS_EXTRA_CONFIG) $($1_CONFIG))
$1_EXTRACT ?= tar xvf "$$($1_PKG)"

ifdef $1_URL
FETCH_PKGS += $$($1_PKG)
$$($1_PKG):
	$(FETCH) "$$($1_URL)"
endif

$$($1_PSEUDO_DEPS):
	@mkdir -p "$$($1_DIR)"
	@mkdir -p "$$($1_BUILD)"
	@touch "$$($1_PSEUDO_DEPS)"

$$($1_PSEUDO_EXTRACT):
	$$($1_EXTRACT)
	@touch "$$($1_PSEUDO_EXTRACT)"

$$($1_PSEUDO_CONFIGURE):
	$$($1_CONFIGURE)
	@touch "$$($1_PSEUDO_CONFIGURE)"

$$($1_PSEUDO_BUILD):
	$$($1_MAKE)
	@touch "$$($1_PSEUDO_BUILD)"

$$($1_PSEUDO_INSTALL):
	$$($1_MAKE_INSTALL)
	@touch "$$($1_PSEUDO_INSTALL)"

$$($1_PSEUDO_DONE):
	@touch "$$($1_PSEUDO_DONE)"

CLEAN_PKGS += $1_clean

.PHONY: $1_clean
$1_clean:
	rm -rf "$$($1_DIR)"
	rm -rf "$$($1_BUILD)"

.PHONY: $1
$1: $$($1_TARGET)

.PHONY: $1_show 
$1_show:
	@echo $1 Download location: $$($1_URL)
	@echo $1 Package: $$($1_PKG)
	@echo $1 Directory: $$($1_DIR)
	@echo $1 Build directory: $$($1_BUILD)
	@echo $1 Dependencies: $$($1_DEPS)
	@echo ==============================
	@echo 0 DEP:        $$($1_TARGET_DEPS)
	@echo 1 EXTRACT:    $$($1_TARGET_EXTRACT)
	@echo 2 CONFIGURE:  $$($1_TARGET_CONFIGURE)
	@echo 3 BUILD:      $$($1_TARGET_BUILD)
	@echo 4 INSTALL:    $$($1_TARGET_INSTALL)
	@echo 5 DONE:       $$($1_TARGET_DONE)

endef

