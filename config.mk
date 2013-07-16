#
# Standard CFLAGS
#
# CFLAGS:=-g
CFLAGS+=-Wall -Wextra -O2 -Werror

#
# Standard LDFLAGS
#
LDFLAGS+=

#
# Do a MINGW cross-compile under Cygwin
#
XBUILD_CYGWIN:=true
#
# Do a MINGW cross-compile under Linux
#
XBUILD_LINUX:=true

# 
# Command used to fetch remote packages
#
FETCH_CMD=wget -ct0
