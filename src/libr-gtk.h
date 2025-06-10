/*
 *
 *  Copyright (c) 2008 Erich Hoover
 *
 *  libr-gtk - Convenience support for GTK+
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

#ifndef __LIBR_GTK_H
#define __LIBR_GTK_H

#include "libr.h"

#ifndef GLADE_H
	typedef void GladeHandle;
#else
	typedef GladeXML GladeHandle;
#endif
#ifndef __GTK_BUILDER_H__
	typedef void BuilderHandle;
#else
	typedef GtkBuilder BuilderHandle;
#endif
#ifndef __G_LIB_H__
	typedef void IconList;
#else
	typedef GList IconList;
#endif

/* GTK Convenience API */
IconList *libr_gtk_iconlist(libr_file *handle);
int libr_gtk_autoload(BuilderHandle **gtk_ret, IconList **icons_ret, int set_default_icon);
int libr_gtk_load(BuilderHandle **gtk_ret, char *resource_name);
int libr_glade_autoload(GladeHandle **glade_ret, IconList **icons_ret, int set_default_icon);
int libr_glade_load(GladeHandle **glade_ret, char *resource_name);

#endif /* __LIBR_GTK_H */

