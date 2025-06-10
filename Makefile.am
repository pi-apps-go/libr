EXTRA_DIST = config.rpath m4/ChangeLog
LIBTOOL_DEPS = @LIBTOOL_DEPS@
instdir = @libdir@/pkgconfig
ACLOCAL_AMFLAGS = -I m4
SUBDIRS = po src man
SED_REPLACE = \
	-e 's=\@prefix\@=@prefix@=' \
	-e 's=\@libdir\@=@libdir@=' \
	-e 's=\@VERSION\@=@VERSION@=' \
	-e 's=\@exec_prefix\@=@exec_prefix@=' \
	-e 's=\@includedir\@=@includedir@/libr='

libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status --recheck

# Generate the pkg-config configuration file with all of
# the accurate installation parameters
libr.pc: libr.pc.in
	cat libr.pc.in | sed $(SED_REPLACE) > libr.pc
CLEANFILES=libr.pc

# Install the pkg-config configuration file
dist_inst_DATA = \
	libr.pc
