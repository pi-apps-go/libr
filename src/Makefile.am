libr_la_includedir = $(includedir)/libr
LIBTOOL_DEPS = @LIBTOOL_DEPS@

INCLUDES = \
	-D__LIBR_BACKEND_@BACKEND_NAME@__ \
	-D__LIBR_BUILD__ \
	@LIBGLADE_CFLAGS@ \
	@BACKEND_CFLAGS@ \
	@EXTRA_CFLAGS@

lib_LTLIBRARIES = \
	libr.la

libr_la_SOURCES = \
	libr-@LIBR_BACKEND@.c \
	tempfiles.c \
	onecanvas.c \
	libr-icons.c \
	libr-i18n.c \
	libr-gtk.c \
	libr.c

libr_la_include_HEADERS = \
	gettext.h \
	libr-icons.h \
	libr-i18n.h \
	libr-gtk.h \
	libr.h

libr_la_LIBADD = \
	@BACKEND_LIBS@ \
	@EXTRA_LIBS@

# If not in a fakeroot environment then run ldconfig
install: install-am
	@if [ ! -n "${FAKEROOTKEY}" ]; then \
		echo "Regenerating system dependencies..."; \
		ldconfig; \
	fi
