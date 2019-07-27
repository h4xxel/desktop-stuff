# dwm version
VERSION = 6.0

# Customize below to fit your system

# paths
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man

X11INC = /usr/X11R6/include
X11LIB = /usr/X11R6/lib

# Xinerama
XINERAMALIBS = -L${X11LIB} -lXinerama
XINERAMAFLAGS = -DXINERAMA

# includes and libs
INCS = -I. -I/usr/include -I${X11INC} `pkg-config --cflags freetype2 xft dbus-1 pango pangoxft` -pthread
LIBS = -L/usr/lib -lc -L${X11LIB} -lX11 ${XINERAMALIBS} `pkg-config --libs freetype2 xft dbus-1 pango pangoxft` -lasound -pthread -lm

# flags
CPPFLAGS = -DVERSION=\"${VERSION}\" ${XINERAMAFLAGS}
CFLAGS = -g -std=gnu99 -Wall -O0 ${INCS} ${CPPFLAGS}
#CFLAGS = -std=gnu99 -Wall -Os ${INCS} ${CPPFLAGS}
LDFLAGS = -g ${LIBS}
#LDFLAGS = -s ${LIBS}

# Solaris
#CFLAGS = -fast ${INCS} -DVERSION=\"${VERSION}\"
#LDFLAGS = ${LIBS}

# compiler and linker
CC = cc
