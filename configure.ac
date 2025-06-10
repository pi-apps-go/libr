AC_INIT(libr.c)
AM_INIT_AUTOMAKE(libr,0.6.0)
AC_CONFIG_SRCDIR([src/libr.c])

AC_PROG_CC
AC_PROG_CC_STDC
AC_HEADER_STDC
AC_PROG_INSTALL
AM_GNU_GETTEXT([external])

# It is important to build both a static and a shared version of the libr 
# library.  The shared version is for future-proofing, whereas the static 
# version should be used to make a more portable binary until libr is
# more widely distributed (as in distributed at all).
AC_ENABLE_SHARED
AC_ENABLE_STATIC

# Setup libtool
AC_PROG_LIBTOOL
AC_SUBST(LIBTOOL_DEPS)

## Handle optional libraries (only header files are used)
PKG_CHECK_MODULES(LIBGLADE, libglade-2.0 >= 2.0.0)

## Handle required components
# Is this the best check for zlib that can be made?
AC_CHECK_HEADERS(zlib.h math.h pthread.h)
EXTRA_LIBS="-lz -lm -lpthread"
EXTRA_CFLAGS="-fvisibility=hidden"
AC_SUBST(EXTRA_CFLAGS)
AC_SUBST(EXTRA_LIBS)

## Handle backend configuration
LIBR_BACKEND="bfd"
BACKEND_NAME="libbfd"
AC_ARG_ENABLE(libelf, [  --enable-libelf         use the libelf backend ], [
	if   test "$enableval" = "yes" ; then
		LIBR_BACKEND="elf"
		BACKEND_NAME="libelf"
	fi
])
AC_ARG_ENABLE(libbfd, [  --enable-libbfd         use the libbfd backend (default) ], [
	if test "$enableval" == "yes" ; then
		LIBR_BACKEND="bfd"
		BACKEND_NAME="libbfd"
	fi
])
AC_ARG_ENABLE(ro, [  --enable-ro             use the read-only backend ], [
	if test "$enableval" == "yes" ; then
		LIBR_BACKEND="ro"
		BACKEND_NAME="readonly"
	fi
])
echo "Using ${BACKEND_NAME} backend."
if test "${LIBR_BACKEND}" == "bfd" ; then
	AC_CHECK_HEADER([bfd.h], [], [
		AC_MSG_ERROR([Could not find libbfd header file (bfd.h)! This file is usually included in the package binutils-dev.])
	])
	BACKEND_PKG="binutils"
	BACKEND_LIBS="-lbfd"
	BACKEND_CFLAGS=""
	AC_SUBST(BACKEND_LIBS)
	AC_SUBST(BACKEND_CFLAGS)
elif test "${LIBR_BACKEND}" == "elf" ; then
	# Is libelf 0.8.2 safe enough? testing is currently on 0.8.6
	BACKEND_PKG="elfutils"
	pkg_modules="libelf >= 0.8.2"
	PKG_CHECK_MODULES(BACKEND, [$pkg_modules])
fi
AC_SUBST(BACKEND_NAME)
AC_SUBST(LIBR_BACKEND)
AC_SUBST(BACKEND_PKG)

## Handle target architecture configuration
UNAMEM=`uname -m`;
if test "$UNAMEM" = "i386" ; then
	ARCH="i386";
elif test "$UNAMEM" = "i486" ; then
	ARCH="i386";
elif test "$UNAMEM" = "i586" ; then
	ARCH="i386";
elif test "$UNAMEM" = "i686" ; then
	ARCH="i386";
elif test "$UNAMEM" = "x86_64"; then
	ARCH="amd64";
fi
if test "$ARCH" = "i386" ; then
	CFLAGS="$CFLAGS -m32";
	AC_MSG_NOTICE([Ready for 32-bit compilation.]);
elif test "$ARCH" = "amd64" ; then
	CFLAGS="$CFLAGS -m64";
	AC_MSG_NOTICE([Ready for 64-bit compilation.]);
else
	AC_MSG_ERROR([Target architecture ${ARCH} is invalid!]);
fi
AC_SUBST(ARCH)
## END Handle target architecture configuration

AC_CONFIG_HEADERS(config.h)
AC_OUTPUT(
	src/Makefile
	man/Makefile
	po/Makefile.in
	Makefile
)
