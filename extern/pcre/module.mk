ifeq ($(findstring clean, $(MAKECMDGOALS)),)
include $(PKG_DIR)/pcre/build.mk
endif

DEPS += $(PKG_DIR)/pcre/build.mk

$(PKG_DIR)/pcre/build.mk:
	$(MAKE) -C "$(EXTERN_DIR)/pcre"
