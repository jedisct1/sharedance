localedir = $(datadir)/locale

INCLUDES = -DLOCALEDIR=\"$(localedir)\"

sbin_PROGRAMS = \
	sharedanced
	
sharedanced_SOURCES = \
	sharedanced.c \
	sharedanced.h \
	sharedanced_p.h \
	process.c \
	process.h \
	bsd-getopt_long.c \
	bsd-getopt_long.h \
	fakesnprintf.c \
	fakesnprintf.h \
	mysnprintf.c \
	mysnprintf.h \
	daemonize.c \
	daemonize.h \
	log.c \
	log.h \
	expire.c \
	expire.h \
	gettext.h \
	globals.h

sharedanced_LDADD = \
	$(LTLIBINTL)

