#
# Standard CFLAGS
#
# CFLAGS:=-g
CFLAGS+=-Wall -Wextra -O2 -Werror
CXXFLAGS+=$(CFLAGS)

#
# Standard LDFLAGS
#
LDFLAGS+=

#
# The MinGW-W64 cross-build is the default now, use the option below to override it
#
#NATIVE_BUILD:=true

# 
# Command used to fetch remote packages
#
FETCH_CMD=wget -ct0

