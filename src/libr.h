/*
 *
 *  Copyright (c) 2008-2011 Erich Hoover
 *
 *  libr - Add resources into ELF binaries
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

#ifndef __LIBR_H
#define __LIBR_H

#include <sys/types.h>

#define DEPRECATED_FN           __attribute__ ((deprecated))
#define ALIAS_FN(fn)            __attribute__ ((weak, alias (#fn)))

/**
 * @addtogroup libr_status libr_status
 * @brief Enumeration of possible libr status values.
 * @{
 * \#include <libr.h>
 */
/** Possible libr status values */
typedef enum {
	LIBR_OK                     =   0, /**< Success */
	LIBR_ERROR_GETEHDR          =  -1, /**< Failed to obtain ELF header: */
	LIBR_ERROR_NOTABLE          =  -2, /**< No ELF string table */
	LIBR_ERROR_TABLE            =  -3, /**< Failed to open string table: */
	LIBR_ERROR_GETDATA          =  -4, /**< Failed to obtain data of section */
	LIBR_ERROR_GETSHDR          =  -5, /**< Failed to obtain ELF section header: */
	LIBR_ERROR_SIZEMISMATCH     =  -6, /**< Section's data size does not make sense */
	LIBR_ERROR_UPDATE           =  -7, /**< Failed to perform dynamic update: */
	LIBR_ERROR_NEWSECTION       =  -8, /**< Failed to create new section */
	LIBR_ERROR_NEWDATA          =  -9, /**< Failed to create data for section */
	LIBR_ERROR_REMOVESECTION    = -10, /**< Failed to remove section: */
	LIBR_ERROR_NOSECTION        = -11, /**< ELF resource section not found */
	LIBR_ERROR_STRPTR           = -12, /**< Failed to obtain section string pointer: */
	LIBR_ERROR_NOTRESOURCE      = -13, /**< Not a valid libr-resource */
	LIBR_ERROR_EXPANDSECTION    = -14, /**< Failed to expand section */
	LIBR_ERROR_WRONGFORMAT      = -15, /**< Invalid input file format */
	LIBR_ERROR_SETFLAGS         = -16, /**< Failed to set flags for section */
	LIBR_ERROR_NOPERM           = -17, /**< Open handle with LIBR_READ_WRITE access */
	LIBR_ERROR_NOSIZE           = -18, /**< Failed to obtain file size */
	LIBR_ERROR_SETFORMAT        = -19, /**< Failed to set output file format to input file format */
	LIBR_ERROR_SETARCH          = -20, /**< Failed to set output file architecture to input file architecture */
	LIBR_ERROR_OVERWRITE        = -21, /**< Section already exists, over-write not specified */
	LIBR_ERROR_COMPRESS         = -22, /**< Failed to compress resource data */
	LIBR_ERROR_INVALIDTYPE      = -23, /**< Invalid data storage type specified */
	LIBR_ERROR_MEMALLOC         = -24, /**< Failed to allocate memory for data */
	LIBR_ERROR_INVALIDPARAMS    = -25, /**< Invalid parameters passed to function */
	LIBR_ERROR_UNCOMPRESS       = -26, /**< Failed to uncompress resource data */
	LIBR_ERROR_ZLIBINIT         = -27, /**< zlib library initialization failed */
	LIBR_ERROR_OPENFAILED       = -28, /**< Failed to open input file */
	LIBR_ERROR_BEGINFAILED      = -29, /**< Failed to open ELF file: */
	LIBR_ERROR_WRITEPERM        = -30, /**< No write permission for file */
	LIBR_ERROR_UNSUPPORTED      = -31, /**< The requested operation is not supported by the backend */
} libr_status;
/**
 * @}
 */

typedef enum {
	LIBR_READ       = 0,
	LIBR_READ_WRITE = 1,
} libr_access_t;

typedef enum {
	LIBR_UNCOMPRESSED = 0,
	LIBR_COMPRESSED   = 1
} libr_type_t;

typedef enum {
	LIBR_NOOVERWRITE = 0,
	LIBR_OVERWRITE   = 1
} libr_overwrite_t;

#ifdef __LIBR_BUILD__
	#include "libr-internal.h"
	#if __LIBR_BACKEND_libbfd__
		#include "libr-bfd.h"
	#elif __LIBR_BACKEND_libelf__
		#include "libr-elf.h"
	#elif __LIBR_BACKEND_readonly__
		#include "libr-ro.h"
	#else /* LIBR_BACKEND */
		#error "Unhandled backend"
	#endif /* LIBR_BACKEND */
	#include "libr-backends.h"
#else
	struct _libr_file;
	typedef struct _libr_file libr_file;
#endif /* __LIBR_BUILD__ */

/*************************************************************************
 * libr Resource Management API
 *************************************************************************/

/**
 * @page libr_clear Remove a resource from an ELF executable.
 * @section SYNOPSIS
 * 	\#include <libr.h>
 * 	
 * 	<b>int libr_clear(libr_file *handle, char *resourcename);</b>
 * 
 * @section DESCRIPTION
 * 	Removes a libr-compatible resource from an ELF executable.  The handle
 * 	must be opened using <b>libr_open</b>(3) with either <b>LIBR_WRITE</b>
 * 	 or <b>LIBR_READ_WRITE</b> access in order to remove a resource.
 * 
 * 	Please note that resource removal does not occur until the handle is
 * 	closed using <b>libr_close</b>(3).
 * 	
 * 	@param handle A handle returned by <b>libr_open</b>(3).
 * 	@param resourcename The name of the libr-compatible resource to remove. 
 * 
 * @section SA SEE ALSO
 * 	<b>libr_open</b>(3), <b>libr_close</b>(3)
 * 
 * @section AUTHOR
 * 	Erich Hoover <ehoover@mines.edu>
 */
int libr_clear(libr_file *handle, char *resourcename);

/**
 * @page libr_close Close a handle to an ELF executable.
 * @section SYNOPSIS
 * 	\#include <libr.h>
 * 	
 * 	<b>void libr_close(libr_file *handle);</b>
 * 
 * @section DESCRIPTION
 * 	Handles opened with <b>libr_open</b>(3) should be closed with
 * 	<b>libr_close</b>() when they are no-longer needed by the calling
 * 	application. 
 * 	
 * 	@param handle The handle to close. 
 * 
 * @section SA SEE ALSO
 * 	<b>libr_open</b>(3)
 * 
 * @section AUTHOR
 * 	Erich Hoover <ehoover@mines.edu>
 */
void libr_close(libr_file *handle);

/**
 * @page libr_errmsg Return a detailed description of the last
 * 	libr-related error.
 * @section SYNOPSIS
 * 	\#include <libr.h>
 * 	
 * 	<b>char *libr_errmsg(void);</b>
 * 
 * @section DESCRIPTION
 * 	Returns a detailed string describing the last error encountered by
 * 	the libr resource library.  The string is an internal error
 * 	description, so it should not be freed.
 * 	
 * 	If no errors have been encountered then NULL is returned. 
 * 
 * @section SA SEE ALSO
 * 	<b>libr_errno</b>(3)
 * 
 * @section AUTHOR
 * 	Erich Hoover <ehoover@mines.edu>
 */
char *libr_errmsg(void);

/**
 * @page libr_errno Return a status code describing the last
 * 	libr-related error.
 * @section SYNOPSIS
 * 	\#include <libr.h>
 * 	
 * 	<b>libr_status libr_errno(void);</b>
 * 
 * @section DESCRIPTION
 * 	Returns a code corresponding to the last error encountered by
 * 	the libr resource library.  For a detailed description of possible
 * 	return values see <b>libr_status</b>(3).
 * 	
 * 	To get a user-readable string corresponding to the last error the
 * 	<b>libr_errmsg</b>(3) function should be used instead. 
 * 	
 * 	If no errors have been encountered then <b>LIBR_OK</b> is returned. 
 * 
 * @section SA SEE ALSO
 * 	<b>libr_errmsg</b>(3)
 * 
 * @section AUTHOR
 * 	Erich Hoover <ehoover@mines.edu>
 */
libr_status libr_errno(void);

/**
 * @page libr_list Obtain the name of a libr ELF resource (by index).
 * @section SYNOPSIS
 * 	\#include <libr.h>
 * 	
 * 	<b>char *libr_list(libr_file *file_handle, unsigned int resourceid);</b>
 * 
 * @section DESCRIPTION
 * 	Returns the name of a libr-compatible resource stored in an ELF binary
 * 	corresponding to the given resource index.  The index value ranges from
 * 	0 to the value returned by <b>libr_resources</b>(3), which returns the
 * 	total number of libr-compatible resources stored in the ELF binary.
 * 	
 * 	@param handle A handle returned by <b>libr_open</b>(3).
 * 	@param resourceid The index of the libr-compatible resource for which
 * 		the name will be returned.
 * 	
 * 	@return Returns a string containing the name of the resource section.  This
 * 		string is allocated when the function is called, so it <i>must be
 * 		unallocated</i> with a call to <b>free</b>(3) when it is no-longer
 * 		needed.  NULL is returned on failure.
 * 
 * @section SA SEE ALSO
 * 	<b>libr_open</b>(3), <b>free</b>(3)
 * 
 * @section AUTHOR
 * 	Erich Hoover <ehoover@mines.edu>
 */
char *libr_list(libr_file *file_handle, unsigned int resourceid);

/**
 * @page libr_malloc Obtain the data corresponding to a libr ELF resource.
 * @section SYNOPSIS
 * 	\#include <libr.h>
 * 	
 * 	<b>char *libr_malloc(libr_file *handle, char *resourcename, size_t *size);</b>
 * 
 * @section DESCRIPTION
 * 	Returns the contents of a libr-compatible resource stored in an ELF binary
 * 	corresponding to the given resource name.
 * 	
 * 	@param handle A handle returned by <b>libr_open</b>(3).
 * 	@param resourcename The name of the libr-compatible resource for which
 * 		the data will be returned.
 * 	@param size A pointer for storing the length of the data contained in the
 * 		the resource.  May be NULL.
 * 	
 * 	@return Returns NULL on failure, the pointer to a buffer containing the data
 * 		for the resource on success.  When the buffer is no-longer used it must
 * 		be unallocated using a call to <b>free</b>(3). 
 * 
 * @section SA SEE ALSO
 * 	<b>libr_open</b>(3), <b>free</b>(3)
 * 
 * @section AUTHOR
 * 	Erich Hoover <ehoover@mines.edu>
 */
char *libr_malloc(libr_file *handle, char *resourcename, size_t *size);

/**
 * @page libr_open Open an ELF executable file for resource management.
 * @section SYNOPSIS
 * 	\#include <libr.h>
 * 	
 * 	<b>libr_file *libr_open(char *filename, libr_access_t access);</b>
 * 
 * @section DESCRIPTION
 * 	<b>libr_open</b>() can be used on any ELF executable, however,
 * 	<b>libr_open</b>() called with <b>LIBR_READ</b> access is only useful
 * 	for executables that already contain libr-compatible stored resources.
 * 	
 * 	An application can easily access its own resources by passing NULL for
 * 	the filename and requesting <b>LIBR_READ</b> access.  For the obvious
 * 	reason that an actively-open application cannot edit itself, the
 * 	calling binary may only request <b>LIBR_READ</b> access.
 * 	
 * 	@param filename ELF executable to manage.  Pass a NULL pointer as the
 * 		filename in order to access the calling binary (<b>LIBR_READ</b>
 * 		access only) @param access Requested access type (<b>LIBR_READ</b>,
 * 		<b>LIBR_WRITE</b>, <b>LIBR_READ_WRITE</b>), the valid operations for
 * 		the returned handle will be restricted based upon the requested access.
 * 	@return Returns a libr file handle on success, NULL on failure.  The
 * 		handle should be freed with <b>libr_close</b>(3) when no-longer used. 
 * 
 * @section SA SEE ALSO
 * 	<b>libr_close</b>(3)
 * 
 * @section AUTHOR
 * 	Erich Hoover <ehoover@mines.edu>
 */
libr_file *libr_open(char *filename, libr_access_t access);

/**
 * @page libr_read Read out the contents of a libr ELF resource.
 * @section SYNOPSIS
 * 	\#include <libr.h>
 * 	
 * 	<b>int libr_read(libr_file *handle, char *resourcename, char *buffer);</b>
 *
 * @section WARNING
 * 	This function does not allocate memory for the buffer, so the buffer must
 * 	be large enough to fit the resource data.  For this reason it is suggested
 * 	that <b>libr_malloc</b>(3) be used in preference over this function.
 * 
 * @section DESCRIPTION
 * 	Reads the contents of a resource embedded in an ELF binary, the resource
 * 	must be compatible with the libr specification.
 * 	
 * 	@param handle A handle returned by <b>libr_open</b>(3).
 * 	@return Returns 1 on success, 0 on failure. 
 * 
 * @section SA SEE ALSO
 * 	<b>libr_open</b>(3)
 * 
 * @section AUTHOR
 * 	Erich Hoover <ehoover@mines.edu>
 */
int libr_read(libr_file *handle, char *resourcename, char *buffer);

/**
 * @page libr_resources Returns the number of resources contained in
 * 	the ELF binary.
 * @section SYNOPSIS
 * 	\#include <libr.h>
 * 	
 * 	<b>unsigned int libr_resources(libr_file *handle);</b>
 *
 * @section DESCRIPTION
 * 	Returns the total number of libr-compatible resources contained
 * 	in the ELF binary.  Intended to be used with <b>libr_list</b>(3)
 * 	to return the full list of resources contained in the binary. 
 * 	
 * 	@param handle A handle returned by <b>libr_open</b>(3).
 * 	@return The total number of libr resources in the binary.
 * 
 * @section SA SEE ALSO
 * 	<b>libr_open</b>(3), <b>libr_list</b>(3)
 * 
 * @section AUTHOR
 * 	Erich Hoover <ehoover@mines.edu>
 */
unsigned int libr_resources(libr_file *handle);

/**
 * @page libr_size Returns the uncompressed size of a libr resource.
 * @section SYNOPSIS
 * 	\#include <libr.h>
 * 	
 * 	<b>int libr_size(libr_file *handle, char *resourcename, size_t *size);</b>
 *
 * @section DESCRIPTION
 * 	Obtain the total number of bytes consumed by the uncompressed
 * 	version of the specific libr-resource.  Intended to be used with
 * 	<b>libr_read</b>(3) in order to allocate a large enough buffer
 * 	for the resource.
 * 	
 * 	@param handle A handle returned by <b>libr_open</b>(3).
 * 	@param resourcename The name of the resource for which the
 * 		size of the data section will be returned.
 * 	@param size A pointer for storing the size of the data section.
 * 		This pointer cannot be NULL.
 * 	@return Returns 1 on success, 0 on failure. 
 * 
 * @section SA SEE ALSO
 * 	<b>libr_open</b>(3), <b>libr_read</b>(3)
 * 
 * @section AUTHOR
 * 	Erich Hoover <ehoover@mines.edu>
 */
int libr_size(libr_file *handle, char *resourcename, size_t *size);

/**
 * @page libr_write Adds a libr resource to an ELF binary.
 * @section SYNOPSIS
 * 	\#include <libr.h>
 * 	
 * 	<b>int libr_write(libr_file *handle, char *resourcename, char *buffer, size_t size, libr_type_t type, libr_overwrite_t overwrite);</b>
 *
 * @section DESCRIPTION
 * 	Adds a libr-compatible resource into the ELF binary.  The handle
 * 	must be opened using <b>libr_open</b>(3) with either <b>LIBR_WRITE</b>
 * 	or <b>LIBR_READ_WRITE</b> access in order to add a resource.
 * 	
 * 	@param handle A handle returned by <b>libr_open</b>(3).
 * 	@param resourcename The name of the resource to create.
 * 	@param buffer A string containing the data of the resource.
 * 	@param size The total size of the buffer.
 * 	@param type The method which should be used for storing the 
 * 		data (either <b>LIBR_UNCOMPRESSED</b> or
 * 		<b>LIBR_COMPRESSED</b>).
 * 	@param overwrite Whether overwriting an existing resource
 * 		should be permitted (either <b>LIBR_NOOVERWRITE</b> or
 * 		<b>LIBR_OVERWRITE</b>). 
 * 	@return Returns 1 on success, 0 on failure. 
 * 
 * @section SA SEE ALSO
 * 	<b>libr_open</b>(3)
 * 
 * @section AUTHOR
 * 	Erich Hoover <ehoover@mines.edu>
 */
int libr_write(libr_file *handle, char *resourcename, char *buffer, size_t size, libr_type_t type, libr_overwrite_t overwrite);

#endif /* __LIBR_H */

