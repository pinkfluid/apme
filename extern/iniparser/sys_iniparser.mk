INIPARSER_PKGDIR := $(EXTERN_DIR)/iniparser/pkg

INIPARSER_DEP:=$(INIPARSER_PKGDIR)

define INIPARSER_INCLUDE
CFLAGS+=-I$(INIPARSER_PKGDIR)
LDFLAGS+=-L$(INIPARSER_PKGDIR) -liniparser

$(INIPARSER_DEP):
	$(MAKE) -C "$(EXTERN_DIR)/iniparser"
endef