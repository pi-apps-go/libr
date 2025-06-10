/*
 *
 *  Copyright (c) 2009 Erich Hoover
 *
 *  libr i18n - Add language resources into ELF binaries
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

#include "libr-i18n.h"
#include "tempfiles.h"

/* Internationalization support */
#include <locale.h>

/* For string handling */
#include <string.h>
#include <stdio.h>

/*
 * Extract the internationalization resources from the binary
 * and setup gettext with the extracted folder.
 */
EXPORT_FN int libr_i18n_load(libr_file *handle, const char *domain)
{
	char *temp_folder;
	int ret = true;
	
	temp_folder = libr_extract_resources(handle);
	if(temp_folder == NULL)
		return false;
	if(!setlocale(LC_ALL, ""))
		ret = false;
	if(!bindtextdomain(domain, temp_folder))
		ret = false;
	if(!textdomain(domain))
		ret = false;
	if(!ret)
		cleanup_folder(temp_folder);
	else
		register_folder_cleanup(temp_folder);
	return ret;
}

EXPORT_FN int libr_i18n_autoload(const char *domain)
{
	libr_file *handle;
	
	/* Obtain the handle to the executable */
	if((handle = libr_open(NULL, LIBR_READ)) == NULL)
	{
		/* "Failed to open this executable (%s) for resources", progname() */
		return false;
	}
	/* Obtain the language files from the ELF binary */
	if(!libr_i18n_load(handle, domain))
	{
		/* "Failed to load language resources!" */
		goto failed;
	}
	
failed:
	libr_close(handle);
	return true;
}
