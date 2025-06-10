/*
 *
 *  Copyright (c) 2009 Erich Hoover
 *  Copyright (c) 2008-2009 Martin Rosenau
 *
 *  libr read-only Backend - Read resources from ELF binaries
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * To provide feedback, report bugs, or otherwise contact me:
 * ehoover at mines dot edu
 *
 */

/* Include compile-time parameters */
#include "config.h"

#include "libr.h"
#include "libr-internal.h"

/* malloc/free */
#include <stdlib.h>

/* For memory byte-wise compare */
#include <string.h>

/* For endian conversion */
#include "cvtendian.h"

#define RETURN_UNSUPPORTED RETURN(LIBR_ERROR_UNSUPPORTED, "The read-only backend does not support this operation");

#ifndef DOXYGEN_SHOULD_SKIP_THIS

typedef struct {
	unsigned char magic[4];
	eClass byte_size;
	eEncoding endian;
	unsigned char version;
	unsigned char padding[9];
} ElfPreHeader;

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

#define ELF_HALF(b)  sizeof(uint16_t)
#define ELF_WORD(b)  sizeof(uint32_t)
#define ELF_XWORD(b) ((b == ELFCLASS32) ? sizeof(uint32_t) : sizeof(uint64_t))
#define ELF_ADDR(b)  ELF_XWORD(b)
#define ELF_OFF(b)   ELF_XWORD(b)

/* ELF Header Offsets */
#define HDROFF_TYPE(b)      sizeof(ElfPreHeader)             /* ElfXX_Half  e_type; */
#define HDROFF_MACHINE(b)   HDROFF_TYPE(b)+ELF_HALF(b)       /* ElfXX_Half  e_machine; */
#define HDROFF_VERSION(b)   HDROFF_MACHINE(b)+ELF_HALF(b)    /* ElfXX_Word  e_version; */
#define HDROFF_ENTRY(b)     HDROFF_VERSION(b)+ELF_WORD(b)    /* ElfXX_Addr  e_entry; */
#define HDROFF_PHOFF(b)     HDROFF_ENTRY(b)+ELF_ADDR(b)      /* ElfXX_Off   e_phoff; */
#define HDROFF_SHOFF(b)     HDROFF_PHOFF(b)+ELF_OFF(b)       /* ElfXX_Off   e_shoff; */
#define HDROFF_FLAGS(b)     HDROFF_SHOFF(b)+ELF_OFF(b)       /* ElfXX_Word  e_flags; */
#define HDROFF_EHSIZE(b)    HDROFF_FLAGS(b)+ELF_WORD(b)      /* ElfXX_Half  e_ehsize; */
#define HDROFF_PHENTSIZE(b) HDROFF_EHSIZE(b)+ELF_HALF(b)     /* ElfXX_Half  e_phentsize; */
#define HDROFF_PHNUM(b)     HDROFF_PHENTSIZE(b)+ELF_HALF(b)  /* ElfXX_Half  e_phnum; */
#define HDROFF_SHENTSIZE(b) HDROFF_PHNUM(b)+ELF_HALF(b)      /* ElfXX_Half  e_shentsize; */
#define HDROFF_SHNUM(b)     HDROFF_SHENTSIZE(b)+ELF_HALF(b)  /* ElfXX_Half  e_shnum; */
#define HDROFF_SHSTRNDX(b)  HDROFF_SHNUM(b)+ELF_HALF(b)      /* ElfXX_Half  e_shstrndx; */

/* ELF Section Offsets */
#define SECOFF_NAME(b)      0                                /* ElfXX_Word  sh_name; */
#define SECOFF_TYPE(b)      SECOFF_NAME(b)+ELF_WORD(b)       /* ElfXX_Word  sh_type; */
#define SECOFF_FLAGS(b)     SECOFF_TYPE(b)+ELF_WORD(b)       /* ElfXX_XWord sh_flags; */
#define SECOFF_ADDR(b)      SECOFF_FLAGS(b)+ELF_XWORD(b)     /* ElfXX_Addr  sh_addr; */
#define SECOFF_OFFSET(b)    SECOFF_ADDR(b)+ELF_ADDR(b)       /* ElfXX_Off   sh_offset; */
#define SECOFF_SIZE(b)      SECOFF_OFFSET(b)+ELF_OFF(b)      /* ElfXX_XWord sh_size; */
#define SECOFF_LINK(b)      SECOFF_SIZE(b)+ELF_XWORD(b)      /* ElfXX_Word  sh_link; */
#define SECOFF_INFO(b)      SECOFF_LINK(b)+ELF_WORD(b)       /* ElfXX_Word  sh_info; */
#define SECOFF_ADDRALIGN    SECOFF_INFO(b)+ELF_WORD(b)       /* ElfXX_XWord sh_addralign; */
#define SECOFF_ENTSIZE      SECOFF_ADDRALIGN(b)+ELF_XWORD(b) /* ElfXX_XWord sh_entsize; */

/*
 * Safely read a parameter from the ELF binary
 */
static int read_param(FILE *handle, void *result, size_t bytes, eClass endian)
{
	if(fread(result, 1, bytes, handle) != bytes)
		return 0;
	if(ferror(handle))
		return 0;
	if(endian != HOST_ENDIAN && !ConvertEndian(result, bytes))
		return 0;
	return 1;
}

/*
 * The read-only backend requires no initialization
 */
void initialize_backend(void)
{
	if(sizeof(ElfPreHeader) != 16)
		fprintf(stderr, "WARNING: Your compiler did not properly pack important structures!\n");
}

/*
 * The read-only backend cannot write an output file
 */
void write_output(libr_file *file_handle) {}

/*
 * The read-only backend cannot add sections
 */
libr_intstatus add_section(libr_file *file_handle, char *resource_name, libr_section **retscn)
{
	RETURN_UNSUPPORTED;
}

/*
 * Return the name of a section
 */
char *section_name(libr_file *file_handle, libr_section *scn)
{
	if(scn == NULL)
		return NULL;
	return scn->name;
}

/*
 * Return the pointer to the actual data in the section
 */
void *data_pointer(libr_section *scn, libr_data *data)
{
	return (void *) data;
}

/*
 * Return the size of the data in the section
 */
size_t data_size(libr_section *scn, libr_data *data)
{
	return scn->size;
}

/*
 * Find the resource stored in the ELF binary
 */
libr_intstatus find_section(libr_file *file_handle, char *section, libr_section **retscn)
{
	char *test_name;
	int i;
	
	for(i=0; i<file_handle->total_sections; i++)
	{
		test_name = section_name(file_handle, &(file_handle->secdata[i]));
		if(test_name != NULL && strcmp(test_name, section) == 0)
			break;
	}
	if(i >= file_handle->total_sections)
		RETURN(LIBR_ERROR_NOSECTION, "ELF resource section not found");
	
	/* Found the resource, hurray! */
	*retscn = &(file_handle->secdata[i]);
	RETURN_OK;
}

/*
 * Read the section from the ELF binary
 */
libr_data *get_data(libr_file *file_handle, libr_section *scn)
{
	FILE *handle = file_handle->handle;
	libr_data *data = NULL;
	size_t n;
	
	fseek(handle, scn->data_offset, SEEK_SET);
	data = (libr_data *) malloc(scn->size);
	n = fread(data, 1, scn->size, handle);
	if(n == 0)
		goto failed; /* Empty section? */
	if(ferror(handle))
		goto failed;
	
	/* Succeeded in reading the data */
	return data;
failed:
	free(data);
	return NULL;
}

/*
 * UNSUPORTED BY BACKEND: Create a new data section
 */
libr_data *new_data(libr_file *file_handle, libr_section *scn)
{
	return NULL;
}

/*
 * Find the next section given a section pointer
 */
libr_section *next_section(libr_file *file_handle, libr_section *scn)
{
	int total_sections = file_handle->total_sections;
	libr_section *test_scn = NULL;
	int i;
	
	if(total_sections == 0)
		return NULL;
	/* Requesting the first section */
	if(scn == NULL)
	{
		i = 0;
		/* Do not return an empty section */
		while(test_scn == NULL || test_scn->size == 0)
		{
			if(i > total_sections)
				return NULL;
			test_scn = &(file_handle->secdata[i++]);
		}
		return test_scn;
	}
	/* Return the next section given a section pointer */
	for(i=0; i<total_sections; i++)
	{
		test_scn = &(file_handle->secdata[i]);
		
		if(test_scn == scn && (i+1) < total_sections)
		{
			libr_section *next_scn = &(file_handle->secdata[i+1]);
			
			/* Returning empty sections is pointless */
			if(next_scn->size != 0)
				return next_scn;
		}
	}
	return NULL;
}

/*
 * UNSUPORTED BY BACKEND: Remove a section
 */
libr_intstatus remove_section(libr_file *file_handle, libr_section *scn)
{
	RETURN_UNSUPPORTED;
}

/*
 * UNSUPORTED BY BACKEND: Set the data for a section
 */
libr_intstatus set_data(libr_file *file_handle, libr_section *scn, libr_data *data, off_t offset, char *buffer, size_t size)
{
	RETURN_UNSUPPORTED;
}

/*
 * Open a handle to the ELF binary (provided that read-only access is requested)
 */
libr_intstatus open_handles(libr_file *file_handle, char *filename, libr_access_t access)
{
	const char elf_magic[] = {'\x7F','E','L','F'};
	uint16_t total_sections, sh_size, strings_sec;
	ElfPreHeader file_info;
	libr_section *secdata;
	FILE *handle = NULL;
	uint64_t sh_offset;
	unsigned long i;
	
	if(access == LIBR_READ_WRITE)
		RETURN_UNSUPPORTED;
	handle = fopen(filename, "rb");
	if(!handle)
		RETURN(LIBR_ERROR_OPENFAILED, "Failed to open input file");
	if(fread(&file_info, 1, sizeof(ElfPreHeader), handle) != sizeof(ElfPreHeader))
		RETURN(LIBR_ERROR_WRONGFORMAT, "Failed to read pre-header bytes from input file");
	if(memcmp(file_info.magic, elf_magic, sizeof(elf_magic)) != 0)
		RETURN(LIBR_ERROR_WRONGFORMAT, "Invalid input file format: not an ELF binary");

	/* Confirm processor (byte size) and packing (endian) */
	if(!enum_valid(file_info.byte_size, ELFCLASS))
		RETURN(LIBR_ERROR_WRONGFORMAT, "Invalid input file format: invalid byte size");
	if(!enum_valid(file_info.endian, ELFDATA))
		RETURN(LIBR_ERROR_WRONGFORMAT, "Invalid input file format: invalid endian type");
	
	/* Get the file offset to the Section Header tables */
	fseek(handle, HDROFF_SHOFF(file_info.byte_size), SEEK_SET);
	if(!read_param(handle, &sh_offset, ELF_OFF(file_info.byte_size), file_info.endian))
		RETURN(LIBR_ERROR_WRONGFORMAT, "Invalid input file format: failed to read section header offset");
	/* Get the size of the Section Header tables */
	fseek(handle, HDROFF_SHENTSIZE(file_info.byte_size), SEEK_SET);
	if(!read_param(handle, &sh_size, ELF_HALF(file_info.byte_size), file_info.endian))
		RETURN(LIBR_ERROR_WRONGFORMAT, "Invalid input file format: failed to read section header size");
	/* Get the total number of sections */
	fseek(handle, HDROFF_SHNUM(file_info.byte_size), SEEK_SET);
	if(!read_param(handle, &total_sections, ELF_HALF(file_info.byte_size), file_info.endian))
		RETURN(LIBR_ERROR_WRONGFORMAT, "Invalid input file format: failed to read total number of sections");
	/* Get the ID of the "strings" section */
	fseek(handle, HDROFF_SHSTRNDX(file_info.byte_size), SEEK_SET);
	if(!read_param(handle, &strings_sec, ELF_HALF(file_info.byte_size), file_info.endian))
		RETURN(LIBR_ERROR_WRONGFORMAT, "Invalid input file format: failed to read string section ID");
	if(strings_sec >= total_sections)
		RETURN(LIBR_ERROR_WRONGFORMAT, "Invalid input file format: invalid string section ID");
	secdata = (libr_section *) malloc(sizeof(libr_section)*total_sections);
	
	/* Load section information */
	for(i=0; i<total_sections; i++)
	{
		long sec_start = sh_offset+sh_size*i;
		
		/* Grab the offset in the string table to the name of the section */
		fseek(handle, sec_start+SECOFF_NAME(file_info.byte_size), SEEK_SET);
		if(!read_param(handle, &(secdata[i].name_offset), ELF_WORD(file_info.byte_size), file_info.endian))
			RETURN(LIBR_ERROR_WRONGFORMAT, "Invalid input file format: failed to read section name offset");
		/* Grab the offset to the data for the section */
		fseek(handle, sec_start+SECOFF_OFFSET(file_info.byte_size), SEEK_SET);
		if(!read_param(handle, &(secdata[i].data_offset), ELF_OFF(file_info.byte_size), file_info.endian))
			RETURN(LIBR_ERROR_WRONGFORMAT, "Invalid input file format: failed to read section data offset");
		/* Grab the size of the data for the section */
		fseek(handle, sec_start+SECOFF_SIZE(file_info.byte_size), SEEK_SET);
		if(!read_param(handle, &(secdata[i].size), ELF_XWORD(file_info.byte_size), file_info.endian))
			RETURN(LIBR_ERROR_WRONGFORMAT, "Invalid input file format: failed to read section size");
	}
	/* Locate the name offset within the "strings" section and load the string */
	for(i=0; i<total_sections; i++)
	{
		long stringsec_start = secdata[strings_sec].data_offset;
		size_t n;
		
		fseek(handle, stringsec_start+secdata[i].name_offset, SEEK_SET);
		n = fread(secdata[i].name, 1, ELFSTRING_MAX-1, handle);
		if(ferror(handle))
			RETURN(LIBR_ERROR_WRONGFORMAT, "Invalid input file format: failed to read string");
		secdata[i].name[n] = '\0';
	}
	
	/* Hold onto the important parameters */
	file_handle->secdata = secdata;
	file_handle->total_sections = total_sections;
	file_handle->endian = file_info.endian;
	file_handle->byte_size = file_info.byte_size;
	file_handle->handle = handle;
	file_handle->filename = filename;
	file_handle->access = access;
	RETURN_OK;
}
