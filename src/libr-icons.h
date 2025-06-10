/*
 *
 *  Copyright (c) 2008-2010 Erich Hoover
 *
 *  libr-icons - Handle icon resources in ELF binaries
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

#ifndef __LIBR_ICONS_H
#define __LIBR_ICONS_H

#include "libr.h"

typedef enum {
	LIBR_SVG = 0,
	LIBR_PNG = 1
} libr_icontype_t;

#define UUIDSTR_LENGTH 37
#define GUIDSTR_LENGTH UUIDSTR_LENGTH

#ifdef __LIBR_BUILD__
	typedef struct {
		char *buffer;
		size_t buffer_size;
		libr_icontype_t type;
		unsigned int icon_size;
	} libr_icon;
#else
	typedef void libr_icon;
#endif

/*************************************************************************
 * libr Icon API
 *************************************************************************/

/**
 * @page libr_icon_close Release an icon resource handle.
 * @section SYNOPSIS
 * 	\#include <libr.h>
 * 	
 * 	<b>int libr_icon_close(libr_icon *icon);</b>
 * 
 * @section DESCRIPTION
 * 	Release the icon resource allocated by a call to
 * 	<b>libr_icon_geticon_byid</b>(3), <b>libr_icon_geticon_byname</b>(3),
 * 	<b>libr_icon_geticon_bysize</b>(3), <b>libr_icon_newicon_byfile</b>(3),
 * 	or <b>libr_icon_newicon_frombuffer</b>(3).
 * 	
 * 	@param icon The icon handle to release. 
 * 	@return Returns 1 on success, 0 on failure.
 * 
 * @section SA SEE ALSO
 * 	<b>libr_icon_geticon_byid</b>(3), <b>libr_icon_geticon_byname</b>(3),
 * 	<b>libr_icon_geticon_bysize</b>(3), <b>libr_icon_newicon_byfile</b>(3),
 * 	<b>libr_icon_newicon_frombuffer</b>(3)
 * 
 * @section AUTHOR
 * 	Erich Hoover <ehoover@mines.edu>
 */
int libr_icon_close(libr_icon *icon);

/*
libr_icon *libr_icon_geticon_byid(libr_file *handle, unsigned int iconid);
*/

/**
 * @page libr_icon_geticon_byname Retrieve an icon resource from an ELF
 * 	binary (by the icon resource's name).
 * @section SYNOPSIS
 * 	\#include <libr.h>
 * 	
 * 	<b>libr_icon *libr_icon_geticon_byname(libr_file *handle, char *iconname);</b>
 * 
 * @section DESCRIPTION
 * 	Return a resource handle to an icon stored in a libr-compatible ELF
 * 	binary.  When this handle is no-longer needed it must be unallocated
 * 	using <b>libr_icon_close</b>(3). 
 * 	
 * 	@param handle A handle returned by <b>libr_open</b>(3).
 * 	@param iconname The exact name of the resource to return. 
 * 	@return Returns a handle on success, NULL on failure.
 * 
 * @section SA SEE ALSO
 * 	<b>libr_open</b>(3), <b>libr_icon_close</b>(3)
 * 
 * @section AUTHOR
 * 	Erich Hoover <ehoover@mines.edu>
 */
libr_icon *libr_icon_geticon_byname(libr_file *handle, char *iconname);

/**
 * @page libr_icon_geticon_bysize Retrieve an icon resource from an ELF
 * 	binary (by the desired icon size).
 * @section SYNOPSIS
 * 	\#include <libr.h>
 * 	
 * 	<b>libr_icon *libr_icon_geticon_bysize(libr_file *handle, unsigned int iconsize);</b>
 * 
 * @section DESCRIPTION
 * 	Return a resource handle to the closest requested size icon stored
 * 	in a libr-compatible ELF binary.  When this handle is no-longer
 * 	needed it must be unallocated using <b>libr_icon_close</b>(3). 
 * 	
 * 	@param handle A handle returned by <b>libr_open</b>(3).
 * 	@param iconsize The size of the resource to return, use 0
 * 		to request an SVG icon.
 * 	@return Returns a handle on success, NULL on failure.
 * 
 * @section SA SEE ALSO
 * 	<b>libr_open</b>(3), <b>libr_icon_close</b>(3)
 * 
 * @section AUTHOR
 * 	Erich Hoover <ehoover@mines.edu>
 */
libr_icon *libr_icon_geticon_bysize(libr_file *handle, unsigned int iconsize);

/**
 * @page libr_icon_getuuid Retrieve the UUID of an application.
 * @section SYNOPSIS
 * 	\#include <libr.h>
 * 	
 * 	<b>int libr_icon_getuuid(libr_file *handle, char *uuid);</b>
 * 
 * @section DESCRIPTION
 * 	Returns the icon UUID corresponding to the ELF binary in hex notation
 * 	(XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX), which requires a 37 character
 * 	buffer (36 data characters and a NULL terminator). 
 * 	
 * 	@param handle A handle returned by <b>libr_open</b>(3).
 * 	@param uuid A buffer to store the UUID of the application.
 * 	@return Returns 1 on success, 0 on failure.
 * 
 * @section SA SEE ALSO
 * 	<b>libr_open</b>(3), <b>libr_icon_close</b>(3),
 * 		<b>libr_icon_setuuid</b>(3)
 * 
 * @section AUTHOR
 * 	Erich Hoover <ehoover@mines.edu>
 */
int libr_icon_getuuid(libr_file *handle, char *uuid);
DEPRECATED_FN int libr_icon_getguid(libr_file *handle, char *uuid);

char *libr_icon_malloc(libr_icon *icon, size_t *size);
/*
libr_icon *libr_icon_newicon_frombuffer(libr_icontype_t type, int iconsize, char *buffer, size_t size);
*/
libr_icon *libr_icon_newicon_byfile(libr_icontype_t type, unsigned int iconsize, char *iconfile);
/*
unsigned int libr_icon_num(libr_file *handle);
*/
int libr_icon_read(libr_icon *icon, char *buffer);
int libr_icon_size(libr_icon *icon, size_t *size);
int libr_icon_save(libr_icon *icon, char *filename);

/**
 * @page libr_icon_setuuid Write a UUID into an application binary.
 * @section SYNOPSIS
 * 	\#include <libr.h>
 * 	
 * 	<b>int libr_icon_setuuid(libr_file *handle, char *uuid);</b>
 * 
 * @section DESCRIPTION
 * 	Sets the icon UUID corresponding to the ELF binary in hex notation
 * 	(XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX), which requires a 37 character
 * 	buffer (36 data characters and a NULL terminator). 
 * 	
 * 	@param handle A handle returned by <b>libr_open</b>(3).
 * 	@param uuid A UUID to set for the application, can be generated by
 * 		the terminal program "uuid".
 * 	@return Returns 1 on success, 0 on failure.
 * 
 * @section SA SEE ALSO
 * 	<b>libr_open</b>(3), <b>libr_icon_close</b>(3),
 * 		<b>libr_icon_getuuid</b>(3)
 * 
 * @section AUTHOR
 * 	Erich Hoover <ehoover@mines.edu>
 */
int libr_icon_setuuid(libr_file *handle, char *uuid);
DEPRECATED_FN int libr_icon_setguid(libr_file *handle, char *guid);
int libr_icon_write(libr_file *handle, libr_icon *icon, char *iconname, libr_overwrite_t overwrite);

#endif /* __LIBR_ICONS_H */
