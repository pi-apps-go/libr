#ifndef __LIBR_BFD_H
#define __LIBR_BFD_H

#include <sys/types.h>
#include <stdint.h>
#include <bfd.h>

/* Forward declarations needed for the types below */
#ifndef __LIBR_INTERNAL_H
/* Define the types we need if not already included */
#define LIBR_TEMPFILE_LEN          22
typedef enum {
	LIBR_READ       = 0,
	LIBR_READ_WRITE = 1,
} libr_access_t;
#endif

/* Modern BFD compatibility - remove the old architecture check as it's no longer reliable */
/*
#if BFD_HOST_64BIT_LONG
	#if defined(__i386)
		#error "Using incorrect binutils header file for architecture."
	#endif
#else
	#if defined(__amd64)
		#error "Using incorrect binutils header file for architecture."
	#endif
#endif
*/

#ifndef DOXYGEN_SHOULD_SKIP_THIS

typedef struct _libr_file {
	int fd_handle;
	bfd *bfd_read;
	bfd *bfd_write;
	char *filename;
	mode_t filemode;
	uid_t fileowner;
	gid_t filegroup;
	char tempfile[LIBR_TEMPFILE_LEN];
	libr_access_t access;
} libr_file;

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

/* for a clean internal API */
typedef asection libr_section;
typedef void libr_data;

#endif /* __LIBR_BFD_H */
