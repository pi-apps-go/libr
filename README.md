# libr - Library to manage resources in ELF binaries

See the INSTALL file for general installation instructions.

## What happened to the original and what does it bring new?
This library is a fixed up version intended to work on modern systems, due to the fact the original hasn't been maintained since 2011 and things have changed since then.

## What is the purpose of this library?
This library is intended to provide an easy to use mechanism for managing
(embedding, retrieving, deleting) resources in ELF binaries.  The library
provides a solid API and ABI that implements the preliminary spec for adding
ELF resources (icons or otherwise) documented at:
https://wiki.ubuntu.com/ELFIconSpec
Please note that should a backward-incompatible change occur to the API/ABI
then the shared library version code will be bumped.

## Why are there multiple backends?
Originally this library was written to use libelf, unfortunately libelf has
some issues with reordering data in small executables.  Until these issues are 
resolved please use libbfd (the default backend) or the read-only backend.
If you would like to experiment with the libelf backend you can try and set
resources on the application "alsamixer", which is a commonly installed
application that is known to break.

## What is the read-only backend?
The read-only backend is a dependency-free backend that is capable of reading
libr resources.  Support for this backend is thanks to Martin Rosenau.
This backend is currently a new edition to libr, however, in the long-run the
read-only backend will be the recommended (default) backend for the purpose of
most applications.  Only a special class of application (primarily elfres)
needs to actually add and remove resources in a binary, most applications
need only read resources already added by elfres.

## NOTICE! This library is licensed under the LGPL v2.1 while the backend for libbfd is licensed under the LGPL v3.  
You may choose to distribute your 
modifications to this variant of the library under the LGPL v3, in accordance
with Section 5 of that license.  Should you wish to excise libbfd from this
library you may choose to remove libr_bfd.h and libr_bfd.c and compile against
libelf instead, the backend may be selected at configure time:
./configure --libr-backend=libelf

I consider the ability to select the library backend as satisfying the LGPL v3
Section 5a requirement and this notice (and the notice contained within
libr_bfd.c) to satisfy the Section 5b requirement.
