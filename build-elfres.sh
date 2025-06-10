#!/bin/bash

# Build script for elfres debian package
set -e

echo "Building elfres debian package..."

# Install libr-dev if not already installed
if ! dpkg -l | grep -q libr-dev; then
    echo "Installing libr-dev dependency..."
    sudo dpkg -i libr-dev_0.6.0-1_amd64.deb || true
    sudo apt-get install -f -y
fi

# Change to elfres directory
cd elfres

# Clean any previous builds
echo "Cleaning previous builds..."
make clean || true
rm -rf debian-build debian/elfres debian/elfres-doc debian/files debian/.debhelper debian/debhelper-build-stamp

# Configure and build
echo "Configuring elfres..."
./configure --prefix=/usr

echo "Building elfres..."
make

# Install to temporary location for packaging
echo "Installing elfres for packaging..."
make DESTDIR=$(pwd)/debian/elfres install

# Build debian package
echo "Building debian package..."
dpkg-buildpackage -us -uc -b

echo "elfres debian package build complete!"
echo "Generated packages:"
ls -la ../elfres*.deb 