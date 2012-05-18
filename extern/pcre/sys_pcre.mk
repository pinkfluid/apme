PCRE_PKGDIR:=$(EXTERN_DIR)/pcre/pkg

ifneq ($(BUILTIN_PCRE),)
    PCRE_PKGCONFIG:=$(PCRE_PKGDIR)/pcre-config
    # If pcre-config exists, use that to extract flags
    ifneq ($(wildcard $PCRE_PKGCONFIG), )
        PCRE_CFLAGS:=$(shell $(PCRE_PKGCONFIG) --cflags-posix)
        PCRE_LDFLAGS:=$(shell $(PCRE_PKGCONFIG) --libs-posix)
    else
        PCRE_CFLAGS:=-DPCRE_STATIC -I$(PCRE_PKGDIR)
        PCRE_LDFLAGS:=-L$(PCRE_PKGDIR) -lpcreposix -lpcre
    endif
else
    PCRE_CFLAGS:=
    PCRE_LDFLAGS:=-lpcreposix -lpcre
endif 

PCRE_DEP:=$(PCRE_PKGDIR)

define PCRE_INCLUDE
CFLAGS+=$(PCRE_CFLAGS)
LDFLAGS+=$(PCRE_LDFLAGS)

$(PCRE_DEP):
	$(MAKE) -C "$(EXTERN_DIR)/pcre"
endef
