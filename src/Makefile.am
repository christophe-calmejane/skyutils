
SUBDIRS = Windows

lib_LTLIBRARIES = libskyutils.la

include_HEADERS = skyutils.h

libskyutils_la_SOURCES = \
	liste.c  \
	memory.c  \
	registry.c  \
	skyutils.h  \
	socks.c  \
	string.c \
	threads.c \
	utils.c \
	web.c \
	arch.c \
	debug.c \
	debug.h \
	debug_config_w32.c

libskyutils_la_LDFLAGS = \
	-version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE) \
	-release $(LT_RELEASE) \
        @SU_PTHREAD_LIB@
