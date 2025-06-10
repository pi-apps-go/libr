/*
 *
 *  Copyright (c) 2008-2011 Erich Hoover
 *
 *  libr icons - Add icon resources into ELF binaries
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

#include "libr-icons.h"

/* For "one canvas" SVG documents */
#include "onecanvas.h"

/* For string manipulation */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

/* For handling files */
#include <sys/stat.h>

/* For C99 number types */
#include <stdint.h>

#define ICON_SECTION     ".icon"
#define TERM_LEN         1

#define OFFSET_ENTRIES   0
#define OFFSET_GUID      OFFSET_ENTRIES+sizeof(uint32_t)

#if defined(__i386)
	#define ID12FORMAT "%012llx"
#elif defined(__x86_64)
	#define ID12FORMAT "%012lx"
#else
	#define ID12FORMAT "%012lx"
	#warning "string formatting may be incorrect on this architecture."
#endif

#ifndef DOXYGEN_SHOULD_SKIP_THIS

typedef uint32_t ID8;
typedef uint16_t ID4;
typedef struct {uint64_t p:48;} __attribute__((__packed__)) ID12;

typedef struct {
	ID8  g1;
	ID4  g2;
	ID4  g3;
	ID4  g4;
	ID12 g5;
} __attribute__((__packed__)) UUID;

typedef struct {
	char *name;
	size_t offset;
	size_t entry_size;
	libr_icontype_t type;
	unsigned int icon_size;
} iconentry;

typedef struct{
	size_t size;
	char *buffer;
	iconentry entry;
} iconlist;

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

/*
 * Decode a UUID to its binary representation
 *
 * NOTE: The last 12-bit parameter cannot be obtained using (uint64_t *) with
 * some versions of GCC using some optimization levels.  This problem is very
 * frustrating to debug, so I do not recommend playing with it yourself.
 */
UUID guid_decode(char *guid)
{
	UUID id = {0x00000000, 0x0000, 0x0000, 0x0000, {0x000000000000} };
	uint64_t tmp12;
	
	sscanf(guid, "%08x-%04hx-%04hx-%04hx-" ID12FORMAT, &id.g1, &id.g2, &id.g3, &id.g4, &tmp12);
	id.g5.p = tmp12;
	return id;
}

/*
 * Return the size of the file represented by the file stream
 */
off_t fsize(FILE *handle)
{
	struct stat file_stat;
	
	if(fstat(fileno(handle), &file_stat) == ERROR)
		return ERROR;
	return file_stat.st_size;
}

/*
 * Create a new icon handle
 */
libr_icon *new_icon_handle(libr_icontype_t type, unsigned int icon_size, char *buffer, size_t buffer_size)
{
	libr_icon *icon_handle = (libr_icon *) malloc(sizeof(libr_icon));
	
	icon_handle->type = type;
	icon_handle->buffer = buffer;
	icon_handle->icon_size = icon_size;
	icon_handle->buffer_size = buffer_size;
	return icon_handle;
}

/*
 * Obtain an existing icon resource list
 */
int get_iconlist(libr_file *file_handle, iconlist *icons)
{
	if(icons == NULL)
	{
		/* Need to be able to return SOMETHING */
		return false;
	}
	/* Obtain the icon resource list */
	icons->buffer = libr_malloc(file_handle, ICON_SECTION, &(icons->size));
	if(icons->buffer == NULL)
		return false;
	return true;
}

/*
 * Get the next entry in an icon resource list
 */
iconentry *get_nexticon(iconlist *icons, iconentry *last_entry)
{
	size_t i;
	
	/* The icon list is needed both for the data buffer and for a call-specific iconentry instance */ 
	if(icons == NULL)
		return NULL;
	/* If this is the first call (last_entry == NULL) then return the first entry */
	if(last_entry == NULL)
		icons->entry.offset = sizeof(uint32_t)+sizeof(UUID);
	else
		icons->entry.offset += icons->entry.entry_size;
	/* Check to see if we've run out of entries */
	if(icons->entry.offset >= icons->size)
		return NULL;
	i = icons->entry.offset;
	memcpy(&(icons->entry.entry_size), &(icons->buffer[i]), sizeof(uint32_t));
	i += sizeof(uint32_t);
	icons->entry.type = icons->buffer[i];
	i += sizeof(unsigned char);
	switch(icons->entry.type)
	{
		case LIBR_SVG:
			icons->entry.icon_size = 0;
			icons->entry.name = &(icons->buffer[i]);
			break;
		case LIBR_PNG:
			memcpy(&(icons->entry.icon_size), &(icons->buffer[i]), sizeof(uint32_t));
			i += sizeof(uint32_t);
			icons->entry.name = &(icons->buffer[i]);
			break;
		default:
			/* Invalid entry type */
			return NULL;
	}
	return &(icons->entry);
}

/*
 * Free an icon handle
 */
EXPORT_FN int libr_icon_close(libr_icon *icon)
{
	if(icon == NULL)
		return false;
	if(icon->buffer == NULL)
		return false;
	free(icon->buffer);
	free(icon);
	return true;
}

/*
 * Read an icon resource from an ELF file by name
 */
EXPORT_FN libr_icon *libr_icon_geticon_byname(libr_file *handle, char *icon_name)
{
	iconentry *entry = NULL;
	libr_icon *icon = NULL;
	size_t buffer_size = 0;
	unsigned int icon_size;
	libr_icontype_t type;
	char *buffer = NULL;
	int inlist = false;
	iconlist icons;
	
	if(!get_iconlist(handle, &icons))
	{
		/* Failed to obtain a list of ELF icons */
		return NULL;
	}
	/* Look for the icon name in the entry list */
	while((entry = get_nexticon(&icons, entry)) != NULL)
	{
		if(!strcmp(entry->name, icon_name))
		{
			type = entry->type;
			icon_size = entry->icon_size;
			inlist = true;
			break;
		}
	}
	if(!inlist)
	{
		/* Could not find icon name in the list of icons */
		return false;
	}
	/* Get the icon from the ELF binary */
	if(!libr_size(handle, icon_name, &buffer_size))
	{
		/* Failed to obtain ELF icon size */
		return NULL;
	}
	/* Allocate memory for the icon */
	buffer = (char *) malloc(buffer_size);
	if(buffer == NULL)
	{
		/* Failed to allocate memory for icon */
		return NULL;
	}
	/* Get the compressed icon from the ELF file */
	if(!libr_read(handle, icon_name, buffer))
	{
		/* Failed to obtain ELF icon */
		goto geticon_byname_complete;
	}
	icon = new_icon_handle(type, icon_size, buffer, buffer_size);
	
geticon_byname_complete:
	if(icon == NULL)
		free(buffer);
	return icon;
}

/*
 * Read an icon resource from an ELF file by the square icon size
 */
EXPORT_FN libr_icon *libr_icon_geticon_bysize(libr_file *handle, unsigned int iconsize)
{
	unsigned int closest_id = 0, i = 0, j = 0;
	int found_png = false, found_svg = false;
	unsigned long closest_size = 0;
	iconentry *entry = NULL;
	iconlist icons;
	
	if(!get_iconlist(handle, &icons))
	{
		/* Failed to obtain a list of ELF icons */
		return NULL;
	}
	/* Look for the closest size match, ignore SVG in case there are multiple icons */
	while((entry = get_nexticon(&icons, entry)) != NULL)
	{
		if(entry->type == LIBR_SVG)
			found_svg = true;
		if(entry->type == LIBR_PNG)
		{
			if(j == 0)
			{
				closest_size = entry->icon_size;
				found_png = true;
			}
			if(abs(iconsize-entry->icon_size) < closest_size)
			{
				closest_size = entry->icon_size;
				closest_id = i;
			}
			j++;
		}
		i++;
	}
	/* If any PNG files were found then use the file if:
	 *  1) There are no SVG files <OR>
	 *  2) The PNG is an EXACT size match 
	 */
	if(found_png)
	{
		i=0;
		entry = NULL;
		while((entry = get_nexticon(&icons, entry)) != NULL)
		{
			if(i == closest_id)
			{
				if(entry->icon_size == iconsize || !found_svg)
					return libr_icon_geticon_byname(handle, entry->name);
				break;
			}
			i++;
		}
	}
	/* Otherwise use the SVG (provided that there is one) */
	if(found_svg)
	{
		entry = NULL;
		while((entry = get_nexticon(&icons, entry)) != NULL)
		{
			if(entry->type == LIBR_SVG)
			{
				libr_icon *icon = libr_icon_geticon_byname(handle, entry->name);
				libr_icon *icon_onecanvas;
				char *buffer;
				
				/* should we report the requested size for SVG? */
				icon->icon_size = iconsize;
				
				/* if the SVG is a "one canvas" document then extract the correctly sized icon */
				if((buffer = onecanvas_geticon_bysize(icon->buffer, iconsize)) != NULL)
				{
					libr_icon_close(icon);
					icon_onecanvas = new_icon_handle(LIBR_SVG, iconsize, buffer, strlen(buffer));
					return icon_onecanvas;
				}
				return icon;
			}
		}
	}
	/* Give up */
	return NULL;
}

/*
 * Obtains the icon UUID for the ELF file
 */
EXPORT_FN int libr_icon_getuuid(libr_file *handle, char *uuid)
{
	UUID id = {0x00000000, 0x0000, 0x0000, 0x0000, {0x000000000000} };
	iconlist icons;
	
	if(!get_iconlist(handle, &icons))
	{
		/* Failed to obtain the list of ELF icons */
		return false;
	}
	/* Now store the GUID to the return string */
	memcpy(&id, &(icons.buffer[OFFSET_GUID]), sizeof(UUID));
	snprintf(uuid, GUIDSTR_LENGTH, "%08x-%04hx-%04hx-%04hx-" ID12FORMAT "\n", id.g1, id.g2, id.g3, id.g4, (uint64_t) id.g5.p);
	free(icons.buffer);
	return true;
}
EXPORT_FN int libr_icon_getguid(libr_file *handle, char *uuid) ALIAS_FN(libr_icon_getuuid);

/*
 * Allocate a buffer containing the data of an icon
 */
EXPORT_FN char *libr_icon_malloc(libr_icon *icon, size_t *size)
{
	char *iconfile = NULL;
	
	if(size == NULL)
	{
		/* No return size passed */
		return NULL;
	}
	if(!libr_icon_size(icon, size))
	{
		/* Failed to obtain embedded icon file size */
		return NULL;
	}
	iconfile = (char *) malloc(*size);
	if(!libr_icon_read(icon, iconfile))
	{
		/* Failed to obtain embedded icon file */
		free(iconfile);
		return NULL;
	}
	return iconfile;
}

/*
 * Create an icon resource to represent a file on the hard disk 
 */
EXPORT_FN libr_icon *libr_icon_newicon_byfile(libr_icontype_t type, unsigned int icon_size, char *icon_file)
{
	libr_icon *icon_handle = NULL;
	size_t len, buffer_size = 0;
	char *buffer = NULL;
	FILE *handle = NULL;
	
	/* Open a handle to the icon file */
	if((handle = fopen(icon_file, "r")) == NULL)
	{
		/* Failed to open icon file */
		return NULL;
	}
	/* Get the size of the icon file */
	if((buffer_size = fsize(handle)) == ERROR)
	{
		/* Failed to obtain the icon's file size */
		return NULL;
	}
	/* Allocate a buffer for the uncompressed icon */
	buffer = (char *) malloc(buffer_size);
	if(buffer == NULL)
	{
		/* Failed to allocate a buffer for the icon data */
		return NULL;
	}
	/* Read the uncompressed image from the disk */
	if((len = fread(buffer, 1, buffer_size, handle)) <= 0)
	{
		/* Failed to read icon from disk */
		goto newicon_complete;
	}
	fclose(handle);
	if(len != buffer_size)
	{
		/* Failed to read the entire icon */
		goto newicon_complete;
	}
	/* Allocate the icon handle */
	icon_handle = new_icon_handle(type, icon_size, buffer, buffer_size);
	
newicon_complete:
	if(icon_handle == NULL)
		free(buffer);
	return icon_handle;
}

/*
 * Copy the icon resource into a buffer
 */
EXPORT_FN int libr_icon_read(libr_icon *icon, char *buffer)
{
	if(icon == NULL)
		return false;
	memcpy(buffer, icon->buffer, icon->buffer_size);
	return true;
}

/*
 * Get the memory size of an icon resource
 */
EXPORT_FN int libr_icon_size(libr_icon *icon, size_t *size)
{
	if(icon == NULL)
		return false;
	*size = icon->buffer_size;
	return true;
}

/*
 * Save the icon resource to a file
 */
EXPORT_FN int libr_icon_save(libr_icon *icon, char *filename)
{
	FILE *file = NULL;
	int ret = false;
	size_t len;
	
	if(icon == NULL)
		return false;
	/* Open the file to store the image */
	if((file = fopen(filename, "w")) == NULL)
	{
		/* Failed to open file to write the icon */
		return false;
	}
	/* Store the uncompressed icon to disk */
	if((len = fwrite(icon->buffer, 1, icon->buffer_size, file)) <= 0)
	{
		/* Failed to write output file */
		goto saveicon_complete;
	}
	if(len != icon->buffer_size)
	{
		/* Did not write the entire file */ 
		goto saveicon_complete;
	}
	ret = true;
	
saveicon_complete:
	/* Close remaining resources */
	fclose(file);
	return ret;
}

/*
 * Sets the icon GUID for the ELF file
 */
EXPORT_FN int libr_icon_setuuid(libr_file *handle, char *guid)
{
	int ret = false;
	iconlist icons;
	UUID id;
	int i;
	
	/* First check the GUID string */
	for(i=0;i<strlen(guid);i++)
	{
		if(!isxdigit(guid[i]))
		{
			if(guid[i] == '-' && (i == 8 || i == 13 || i == 18 || i == 23))
				continue;
			/* not a valid GUID string */
			return false;
		}
	}
	id = guid_decode(guid);
	/* Now check existing resources */
	if(!get_iconlist(handle, &icons))
	{
		/* No icons exist in the file, create a new icon section with the GUID */
		uint32_t entries = 0;
		
		icons.size = sizeof(uint32_t)+sizeof(UUID);
		icons.buffer = (char *) malloc(icons.size);
		memcpy(&(icons.buffer[OFFSET_ENTRIES]), &entries, sizeof(uint32_t));
	}
	/* Set the GUID and write the resource */
	if(!libr_write(handle, ICON_SECTION, icons.buffer, icons.size, LIBR_UNCOMPRESSED, LIBR_OVERWRITE))
	{
		/* failed to write icon resource */
		goto setguid_complete;
	}
	ret = true;
	
setguid_complete:
	free(icons.buffer);
	return ret;
}
EXPORT_FN int libr_icon_setguid(libr_file *handle, char *uuid) ALIAS_FN(libr_icon_setuuid);

/*
 * Add an icon resource to an ELF file
 */
EXPORT_FN int libr_icon_write(libr_file *handle, libr_icon *icon, char *icon_name, libr_overwrite_t overwrite)
{
	size_t entry_size, i;
	iconentry *entry = NULL;
	iconlist icons;
	int ret = false;
	
	/* Check to make sure the user did not make a poor name choice */
	if(!strcmp(icon_name, ICON_SECTION))
	{
		/* ".icon" is a reserved section name */
		return false;
	
	}
	/* Check to make sure the file supports icon resources */
	if(!get_iconlist(handle, &icons))
	{
		/* A GUID must be set first */
		return false;
	}
	/* First add the icon as a new named section */
	if(!libr_write(handle, icon_name, icon->buffer, icon->buffer_size, LIBR_COMPRESSED, overwrite))
	{
		/* Failed to add the icon as a resource */
		goto writeicon_complete;
	}
	/* Look to see if the icon already has an entry */
	while((entry = get_nexticon(&icons, entry)) != NULL)
	{
		if(!strcmp(entry->name, icon_name))
		{
			ret = true;
			goto writeicon_complete;
		}
	}
	/* Now add the icon to the list of icon resources in the ".icon" section */
	switch(icon->type)
	{
		case LIBR_SVG:
			entry_size = sizeof(uint32_t)+sizeof(unsigned char)+strlen(icon_name)+TERM_LEN;
			break;
		case LIBR_PNG:
			entry_size = sizeof(uint32_t)+sizeof(unsigned char)+sizeof(uint32_t)+strlen(icon_name)+TERM_LEN;
			break;
		default:
			/* Unhandled icon type */
			goto writeicon_complete;
	}
	icons.buffer = (char *) realloc(icons.buffer, icons.size+entry_size);
	if(icons.buffer == NULL)
	{
		/* Failed to expand memory size */
		goto writeicon_complete;
	}
	i = icons.size; 
	memcpy(&(icons.buffer[i]), &entry_size, sizeof(uint32_t));
	i+=sizeof(uint32_t);
	icons.buffer[i] = icon->type;
	i+=sizeof(unsigned char);
	if(icon->type == LIBR_PNG)
	{
		memcpy(&(icons.buffer[i]), &icon->icon_size, sizeof(uint32_t));
		i+=sizeof(uint32_t);
	}
	memcpy(&(icons.buffer[i]), icon_name, strlen(icon_name));
	i+=strlen(icon_name);
	icons.buffer[i] = '\0';
	icons.size += entry_size;
	if(i != (icons.size-1))
		printf("Really dangerous, buffer size mismatch!\n");
	/* Write the updated icon table */
	if(!libr_write(handle, ICON_SECTION, icons.buffer, icons.size, LIBR_UNCOMPRESSED, LIBR_OVERWRITE))
	{
		/* failed to write icon resource */
		goto writeicon_complete;
	}
	ret = true;
	
writeicon_complete:
	if(icons.buffer)
		free(icons.buffer);
	return ret;
}
