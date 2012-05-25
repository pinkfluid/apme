ifeq ($(findstring clean, $(MAKECMDGOALS)),)
include $(PKG_DIR)/iniparser/build.mk
endif

DEPS += $(PKG_DIR)/iniparser/build.mk

$(PKG_DIR)/iniparser/build.mk:
	$(MAKE) -C "$(EXTERN_DIR)/iniparser"
