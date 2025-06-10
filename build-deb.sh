#!/bin/bash

set -e

# Build the software
echo "Building libr library..."
make

echo "Building gnome-elf-thumbnailer..."
make -C ../gnome-elf-thumbnailer

# Create package directories
BUILDDIR=$(pwd)/debian-build
rm -rf $BUILDDIR
mkdir -p $BUILDDIR

# Create libr0 package
echo "Creating libr0 package..."
LIBR0_DIR=$BUILDDIR/libr0
mkdir -p $LIBR0_DIR/DEBIAN
mkdir -p $LIBR0_DIR/usr/lib

# Install libr library (skip ldconfig by setting FAKEROOTKEY)
FAKEROOTKEY=1 make DESTDIR=$LIBR0_DIR install

# Create libr0 control file
cat > $LIBR0_DIR/DEBIAN/control << EOF
Package: libr0
Version: 0.6.0-1
Section: libs
Priority: optional
Architecture: amd64
Depends: libc6 (>= 2.17), libbinutils
Maintainer: Erich E. Hoover <ehoover@mines.edu>
Description: ELF binary resource library
 Store and retrieve resources from ELF binaries.
 .
 This library allows storing and retrieving resources (such as icons)
 directly within ELF executable files.
EOF

# Create libr-dev package
echo "Creating libr-dev package..."
LIBR_DEV_DIR=$BUILDDIR/libr-dev
mkdir -p $LIBR_DEV_DIR/DEBIAN

# Copy development files (headers, etc.) - they're installed in /usr/local/
mkdir -p $LIBR_DEV_DIR/usr/local/include
mkdir -p $LIBR_DEV_DIR/usr/local/lib/pkgconfig
cp -r $LIBR0_DIR/usr/local/include/* $LIBR_DEV_DIR/usr/local/include/ 2>/dev/null || true
cp $LIBR0_DIR/usr/local/lib/*.a $LIBR_DEV_DIR/usr/local/lib/ 2>/dev/null || true
cp $LIBR0_DIR/usr/local/lib/*.la $LIBR_DEV_DIR/usr/local/lib/ 2>/dev/null || true
cp $LIBR0_DIR/usr/local/lib/pkgconfig/*.pc $LIBR_DEV_DIR/usr/local/lib/pkgconfig/ 2>/dev/null || true

# Remove dev files from libr0
rm -rf $LIBR0_DIR/usr/local/include
rm -f $LIBR0_DIR/usr/local/lib/*.a $LIBR0_DIR/usr/local/lib/*.la
rm -rf $LIBR0_DIR/usr/local/lib/pkgconfig

# Create libr-dev control file
cat > $LIBR_DEV_DIR/DEBIAN/control << EOF
Package: libr-dev
Version: 0.6.0-1
Section: libdevel
Priority: optional
Architecture: amd64
Depends: libr0 (= 0.6.0-1)
Maintainer: Erich E. Hoover <ehoover@mines.edu>
Description: ELF binary resource library - development files
 Store and retrieve resources from ELF binaries.
 .
 This package contains the development files needed to build applications
 that use the libr library.
EOF

# Create gnome-elf-thumbnailer package
echo "Creating gnome-elf-thumbnailer package..."
THUMBNAILER_DIR=$BUILDDIR/gnome-elf-thumbnailer
mkdir -p $THUMBNAILER_DIR/DEBIAN
mkdir -p $THUMBNAILER_DIR/usr/bin
mkdir -p $THUMBNAILER_DIR/usr/share/thumbnailers

# Install gnome-elf-thumbnailer
make -C ../gnome-elf-thumbnailer DESTDIR=$THUMBNAILER_DIR install

# Create gnome-elf-thumbnailer control file
cat > $THUMBNAILER_DIR/DEBIAN/control << EOF
Package: gnome-elf-thumbnailer
Version: 0.6.0-1
Section: graphics
Priority: optional
Architecture: amd64
Depends: libc6 (>= 2.17), libcairo2 (>= 1.2.4), librsvg2-2 (>= 2.14.4), libgdk-pixbuf-2.0-0 (>= 2.22.0), libr0 (>= 0.6.0)
Recommends: elficon
Maintainer: Erich E. Hoover <ehoover@mines.edu>
Description: GNOME thumbnailer for ELF executables with embedded icons
 A native C thumbnailer for GNOME that generates thumbnails for ELF
 executable files. It can extract embedded icons from ELF files using
 the elficon utility and libr library, or create informative fallback
 thumbnails showing the architecture and executable type.
 .
 Features:
  * Extracts embedded SVG icons with full color depth
  * Creates professional gradient fallback thumbnails
  * Fast native C implementation using Cairo and librsvg
  * Integrates seamlessly with GNOME file manager
 .
 This package integrates with GNOME's thumbnailing system to provide
 rich visual previews of executable files in the file manager.
EOF

# Build packages
echo "Building .deb packages..."
cd $BUILDDIR

dpkg-deb --build libr0
dpkg-deb --build libr-dev  
dpkg-deb --build gnome-elf-thumbnailer

echo "Packages built successfully:"
ls -la *.deb

echo "To install:"
echo "sudo dpkg -i libr0.deb libr-dev.deb gnome-elf-thumbnailer.deb" 