ifeq ($(findstring clean, $(MAKECMDGOALS)),)
include $(PKG_DIR)/wxwidgets/build.mk
endif

DEPS += $(PKG_DIR)/wxwidgets/build.mk

$(PKG_DIR)/wxwidgets/build.mk:
	$(MAKE) -C "$(EXTERN_DIR)/wxwidgets"
