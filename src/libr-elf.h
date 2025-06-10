#ifndef __LIBR_ELF_H
#define __LIBR_ELF_H

/* Handle ELF files */
#include <libelf.h>
#include <gelf.h>

#ifndef DOXYGEN_SHOULD_SKIP_THIS

typedef struct _libr_file {
	int fd_handle;
	Elf *elf_handle;
	size_t file_size;
	libr_access_t access;
	unsigned int version;
} libr_file;

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

/* for a clean internal API */
typedef Elf_Scn libr_section;
typedef Elf_Data libr_data;

#endif /* __LIBR_ELF_H */
