/*
 *
 *  Copyright (c) 2008 Erich Hoover
 *
 *  libr libelf Backend - Add resources into ELF binaries using libelf
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

#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

//#define MANUAL_LAYOUT            true

extern void libr_set_error(libr_intstatus error);

/*
 * Write the output file using libelf
 */
void write_output(libr_file *file_handle)
{
	/* Update the ELF file on the disk */
	if(elf_update(file_handle->elf_handle, ELF_C_NULL) < 0)
	{
		printf("elf_update() failed: %s.", elf_errmsg(-1));
		return;
	}
	if(elf_update(file_handle->elf_handle, ELF_C_WRITE) < 0)
	{
		printf("elf_update() failed: %s.", elf_errmsg(-1));
		return;
	}
	/* Close the handles */
	elf_end(file_handle->elf_handle);
	close(file_handle->fd_handle);
}

/*
 * Return the size of the file represented by the file descriptor
 */
off_t file_size(int fd)
{
	struct stat file_stat;
	
	if(fstat(fd, &file_stat) == ERROR)
		return ERROR;
	return file_stat.st_size;
}

/*
 * Open the handles for working with the file using libelf
 */
libr_intstatus open_handles(libr_file *file_handle, char *filename, libr_access_t access)
{
	const int elf_access[2] = {ELF_C_READ, ELF_C_RDWR};
	const int fd_access[2] = {O_RDONLY, O_RDWR};
	Elf *e = NULL;
	int fd = 0;
	
	if((fd = open(filename, fd_access[access], 0)) < 0)
		RETURN(LIBR_ERROR_OPENFAILED, "Failed to open input file");
	if((e = elf_begin(fd, elf_access[access], NULL)) == NULL)
		RETURN(LIBR_ERROR_BEGINFAILED, "Failed to open ELF file: %s.", elf_errmsg(-1));
	if(elf_kind(e) != ELF_K_ELF)
		RETURN(LIBR_ERROR_WRONGFORMAT, "Invalid input file format");
	
	file_handle->access = access;
	file_handle->fd_handle = fd;
	file_handle->elf_handle = e;
	file_handle->file_size = file_size(fd);
	file_handle->version = EV_CURRENT; /* This should probably match the rest of the file */
	RETURN_OK;
}

/*
 * Expand a section
 * (Only used when manually controlling ELF layout)
 */
#ifdef MANUAL_LAYOUT
libr_intstatus expand_section(Elf *e, Elf_Scn *scn, size_t size, int reset)
{
	size_t offset = 0, delta = 0;
	Elf_Scn *tmpscn = NULL;
	GElf_Shdr shdr;
	
	if(gelf_getshdr(scn, &shdr) != &shdr)
		RETURN(LIBR_INTERROR_GETSHDR, "Failed to obtain ELF header: %s", elf_errmsg(-1));
	if(reset)
	{
		delta = (size-shdr.sh_size); 
		shdr.sh_size = size;
	}
	else
	{
		delta = size;
		shdr.sh_size += size;
	}
	offset = shdr.sh_offset;
	if(gelf_update_shdr(scn, &shdr) < 0)
		RETURN(LIBR_ERROR_UPDATE, "Failed to perform dynamic update: %s.", elf_errmsg(-1));
	if(elf_update(e, ELF_C_NULL) < 0)
		RETURN(LIBR_ERROR_UPDATE, "Failed to perform dynamic update: %s.", elf_errmsg(-1));
	/* Update any section that follows this one data-wise */
/*
****** This does not work yet
	while((tmpscn = elf_nextscn(e, tmpscn)) != NULL)
	{
		if(tmpscn == scn)
			continue;
		if(gelf_getshdr(tmpscn, &shdr) != &shdr)
			return LIBR_INTERROR_GETSHDR;
		if(offset < shdr.sh_offset)
		{
			if((name = elf_strptr(e, ehdr.e_shstrndx, shdr.sh_name)) == NULL)
				RETURN(LIBR_ERROR_STRPTR, "Failed to obtain section string pointer: %s.", elf_errmsg(-1));
			shdr.sh_offset += delta;
			if(gelf_update_shdr(tmpscn, &shdr) < 0)
				RETURN(LIBR_ERROR_UPDATE, "Failed to perform dynamic update: %s.", elf_errmsg(-1));
			if(elf_update(e, ELF_C_NULL) < 0)
				RETURN(LIBR_ERROR_UPDATE, "Failed to perform dynamic update: %s.", elf_errmsg(-1));
		}
	}
*/
	return LIBR_OK;
}
#endif /* MANUAL_LAYOUT */

/*
 * Obtain the data from a section using libelf
 */
libr_data *get_data(libr_file *file_handle, libr_section *scn)
{
	return elf_getdata(scn, NULL);
}

/*
 * Create new data for a section using libelf
 */
libr_data *new_data(libr_file *file_handle, libr_section *scn)
{
	return elf_newdata(scn);
}

/*
 * Set data for a section using libelf (not written yet)
 */
libr_intstatus set_data(libr_file *file_handle, libr_section *scn, libr_data *data, off_t offset, char *buffer, size_t size)
{
	data->d_align = 1;
	data->d_off = offset;
	data->d_buf = buffer;
	data->d_type = ELF_T_BYTE;
	data->d_size = size;
	data->d_version = file_handle->version;
#ifdef MANUAL_LAYOUT
	if(expand_section(file_handle->elf_handle, scn, data->d_size, true) != LIBR_OK)
		RETURN(LIBR_ERROR_EXPANDSECTION, "Failed to expand section");
#else
	if(elf_update(file_handle->elf_handle, ELF_C_NULL) < 0)
		RETURN(LIBR_ERROR_UPDATE, "Failed to perform dynamic update: %s.", elf_errmsg(-1));
	if(elf_update(file_handle->elf_handle, ELF_C_WRITE) < 0)
		RETURN(LIBR_ERROR_UPDATE, "Failed to perform dynamic update: %s.", elf_errmsg(-1));
#endif /* MANUAL_LAYOUT */
	RETURN_OK;
}

/*
 * Find a named section from the ELF file using libelf
 */
libr_intstatus find_section(libr_file *file_handle, char *section, libr_section **retscn)
{
	Elf *e = file_handle->elf_handle;
	Elf_Scn *scn = NULL;
	char *name = NULL;
	GElf_Ehdr ehdr;
	GElf_Shdr shdr;
	uintmax_t si;
	
	if(gelf_getehdr(e, &ehdr) == NULL)
		RETURN(LIBR_ERROR_GETEHDR, "Failed to obtain ELF header: %s", elf_errmsg(-1));
	while((scn = elf_nextscn(e, scn)) != NULL)
	{
		if(gelf_getshdr(scn, &shdr) != &shdr)
			RETURN(LIBR_ERROR_GETSHDR, "Failed to obtain ELF section header: %s", elf_errmsg(-1));
		if((name = elf_strptr(e, ehdr.e_shstrndx, shdr.sh_name)) == NULL)
			RETURN(LIBR_ERROR_STRPTR, "Failed to obtain section string pointer: %s.", elf_errmsg(-1));
		
		si = (uintmax_t) elf_ndxscn(scn);
/*
printf("%d: %s (%d %d)\n", (long) si, name, (long) shdr.sh_offset, (long) shdr.sh_size);
*/
		if(strcmp(name, section) == 0)
		{
			*retscn = scn;
			RETURN_OK;
		}
	}
	RETURN(LIBR_ERROR_NOSECTION, "ELF resource section not found");
}

/*
 * Add a new section and create a name for it in the ELF string table using libelf
 */
libr_intstatus add_section(libr_file *file_handle, char *section, Elf_Scn **retscn)
{
	Elf_Scn *scn = NULL, *strscn = NULL;
	Elf *e = file_handle->elf_handle;
#ifdef MANUAL_LAYOUT
	size_t tblsize = 0;
#endif /* MANUAL_LAYOUT */
	Elf_Data *data;
	GElf_Ehdr ehdr;
	GElf_Shdr shdr;
	
	if(gelf_getehdr(e, &ehdr) == NULL)
		RETURN(LIBR_ERROR_GETEHDR, "Failed to obtain ELF header: %s", elf_errmsg(-1));
	/* TODO: Support creating a string table for objects that don't have one */
	if(!ehdr.e_shstrndx)
		RETURN(LIBR_ERROR_NOTABLE, "No ELF string table");
	strscn = elf_getscn(e, ehdr.e_shstrndx);
	if(strscn == NULL) 
		RETURN(LIBR_ERROR_TABLE, "Failed to open string table: %s.", elf_errmsg(-1));
	data = elf_newdata(strscn);
	if(data == NULL)
		RETURN(LIBR_ERROR_NEWDATA, "Failed to create data for section");
	data->d_align = 1;

#ifdef MANUAL_LAYOUT
{
	GElf_Shdr strshdr;
	
	if(gelf_getshdr(strscn, &strshdr) != &strshdr)
		RETURN(LIBR_ERROR_GETSHDR, "Failed to obtain ELF section header: %s", elf_errmsg(-1));
	data->d_off = strshdr.sh_size;
#endif /* MANUAL_LAYOUT */

	data->d_size = (size_t) strlen(section)+1;
	data->d_type = ELF_T_BYTE;
	data->d_buf = section;
	data->d_version = file_handle->version;

#ifdef MANUAL_LAYOUT
	if(expand_section(e, strscn, data->d_size, false) != LIBR_OK)
		return false;
}
#else
	/* Update the internal offset information */
	if(elf_update(e, ELF_C_NULL) < 0)
		RETURN(LIBR_ERROR_UPDATE, "Failed to perform dynamic update: %s.", elf_errmsg(-1));
#endif /* MANUAL_LAYOUT */

	/* seek to the end of the section data */
	if((scn = elf_newscn(e)) == NULL)
		RETURN(LIBR_ERROR_NEWSECTION, "Failed to create new section");
	if(gelf_getshdr(scn, &shdr) != &shdr)
		RETURN(LIBR_ERROR_GETSHDR, "Failed to obtain ELF section header: %s", elf_errmsg(-1));
	shdr.sh_addralign = 1;
#ifdef MANUAL_LAYOUT
	shdr.sh_offset = file_handle->file_size;
#endif /* MANUAL_LAYOUT */
	shdr.sh_size = 0;
	shdr.sh_name = data->d_off;
	shdr.sh_type = SHT_NOTE; /* TODO: Does "NOTE" type fit best? */
	shdr.sh_flags = SHF_WRITE;
	shdr.sh_entsize = 0;
	if(gelf_update_shdr(scn, &shdr) < 0)
		RETURN(LIBR_ERROR_UPDATE, "Failed to perform dynamic update: %s.", elf_errmsg(-1));
	*retscn = scn;
	RETURN_OK;
}

/*
 * Remove a section and eliminate it from the ELF string table using libelf
 */
libr_intstatus remove_section(libr_file *file_handle, libr_section *scn)
{
	unsigned int table_size, str_size;
	char *buffer = NULL, *tmp = NULL;
	Elf *e = file_handle->elf_handle;
	int remaining_size;
	Elf_Scn *strscn;
	Elf_Data *data;
	GElf_Ehdr ehdr;
	GElf_Shdr shdr;
	
	if(gelf_getehdr(e, &ehdr) == NULL)
		RETURN(LIBR_ERROR_GETEHDR, "Failed to obtain ELF header: %s", elf_errmsg(-1));
	/* Grab the string table */
	if(!ehdr.e_shstrndx)
		RETURN(LIBR_ERROR_NOTABLE, "No ELF string table");
	strscn = elf_getscn(e, ehdr.e_shstrndx);
	if(strscn == NULL)
		RETURN(LIBR_ERROR_TABLE, "Failed to open string table: %s.", elf_errmsg(-1));
	if((data = elf_getdata(strscn, NULL)) == NULL)
		RETURN(LIBR_ERROR_GETDATA, "Failed to obtain data of section");
	/* Find where the section name is in the string table */
	if(gelf_getshdr(scn, &shdr) != &shdr)
		RETURN(LIBR_ERROR_GETSHDR, "Failed to obtain ELF section header: %s", elf_errmsg(-1));
	table_size = data->d_size;
	buffer = (char *) data->d_buf;
	/* Excise the string from the table */
	str_size = strlen(&buffer[shdr.sh_name])+1;
	remaining_size = table_size-(shdr.sh_name+str_size);
	if(remaining_size < 0)
		RETURN(LIBR_ERROR_SIZEMISMATCH, "Section's data size does not make sense");
	if(remaining_size > 0)
	{
		/* If there is data after our icon entry in the table then it must be moved before resizing
		 * NOTE: Using memcpy with overlapping addresses is not allowed, use temporary buffer. 
		 */
		tmp = (char *) malloc(remaining_size);
		memcpy(tmp, &buffer[shdr.sh_name+str_size], remaining_size);
		memcpy(&buffer[shdr.sh_name], tmp, remaining_size);
		free(tmp);
	}
	data->d_size -= str_size;
	/* Update the internal offset information */
	if(elf_update(e, ELF_C_NULL) < 0)
		RETURN(LIBR_ERROR_UPDATE, "Failed to perform dynamic update: %s.", elf_errmsg(-1));
#ifdef MANUAL_LAYOUT
{
	GElf_Shdr strshdr;
	
	if(gelf_getshdr(strscn, &strshdr) != &strshdr)
		RETURN(LIBR_ERROR_GETSHDR, "Failed to obtain ELF section header: %s", elf_errmsg(-1));
	strshdr.sh_size -= str_size;
	if(gelf_update_shdr(strscn, &strshdr) < 0)
		RETURN(LIBR_ERROR_UPDATE, "Failed to perform dynamic update: %s.", elf_errmsg(-1));
}
#endif /* MANUAL_LAYOUT */

	/* Clear the section itself and update the offsets */
	if(elfx_remscn(e, scn) == 0)
		RETURN(LIBR_ERROR_REMOVESECTION, "Failed to remove section: %s.", elf_errmsg(-1));
	RETURN_OK;
}

/*
 * Return the pointer to the actual data in the section
 */
void *data_pointer(libr_section *scn, libr_data *data)
{ 
	return data->d_buf;
}

/*
 * Return the size of the data in the section
 */
size_t data_size(libr_section *scn, libr_data *data)
{
	return data->d_size;
}

/*
 * Return the next section in the ELF file
 */
libr_section *next_section(libr_file *file_handle, libr_section *scn)
{
	return elf_nextscn(file_handle->elf_handle, scn);
}

/*
 * Retrieve the name of a section
 */
char *section_name(libr_file *file_handle, libr_section *scn)
{
	char *name = NULL;
	GElf_Shdr shdr;
	GElf_Ehdr ehdr;
	
	if(gelf_getehdr(file_handle->elf_handle, &ehdr) == NULL)
		return NULL;
	if(gelf_getshdr(scn, &shdr) != &shdr)
		return NULL;
	if((name = elf_strptr(file_handle->elf_handle, ehdr.e_shstrndx, shdr.sh_name)) == NULL)
		return NULL;
	return strdup(name);
}

/*
 * Initialize the libelf backend
 */
void initialize_backend(void)
{
	if(elf_version(EV_CURRENT) == EV_NONE)
		return; //errx(EX_SOFTWARE, "ELF library initialization failed: %s", elf_errmsg(-1));
}
