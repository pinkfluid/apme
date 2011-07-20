
ifneq ($(BUILTIN_PCRE),)
    PCRE_CONFIG:=$(EXTERN_DIR)/pcre/pcre-config
    # If pcre-config exists, use that to extract flags
    ifneq ($(wildcard $PCRE_CONFIG), )
        PCRE_CFLAGS:=$(shell $(PCRE_CONFIG) --cflags-posix)
        PCRE_LDFLAGS:=$(shell $(PCRE_CONFIG) --libs-posix)
    else
        PCRE_CFLAGS:=-DPCRE_STATIC -I$(EXTERN_DIR)/pcre
        PCRE_LDFLAGS:=-L$(EXTERN_DIR)/pcre -lpcreposix -lpcre
    endif
else
    PCRE_CFLAGS:=
    PCRE_LDFLAGS:=-lpcreposix -lpcre
endif 

