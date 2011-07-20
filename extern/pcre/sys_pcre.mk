PCRE_DIR:=$(EXTERN_DIR)/pcre

ifneq ($(BUILTIN_PCRE),)
    PCRE_CONFIG:=$(PCRE_DIR)/pcre-config
    # If pcre-config exists, use that to extract flags
    ifneq ($(wildcard $PCRE_CONFIG), )
        PCRE_CFLAGS:=$(shell $(PCRE_CONFIG) --cflags-posix)
        PCRE_LDFLAGS:=$(shell $(PCRE_CONFIG) --libs-posix)
    else
        PCRE_CFLAGS:=-DPCRE_STATIC -I$(EXTERN_DIR)/pcre
        PCRE_LDFLAGS:=-L$(PCRE_DIR) -lpcreposix -lpcre
    endif
else
    PCRE_CFLAGS:=
    PCRE_LDFLAGS:=-lpcreposix -lpcre
endif 

PCRE_DEP:=$(PCRE_DIR)/.pcre_install_done

define PCRE_INCLUDE
CFLAGS+=$(PCRE_CFLAGS)
LDFLAGS+=$(PCRE_LDFLAGS)

$(PCRE_DEP):
	$(MAKE) -C "$(PCRE_DIR)"
endef
