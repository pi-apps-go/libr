/*
 *
 *  Copyright (c) 2008-2009 Erich Hoover
 *
 *  libr GTK support - Convenience functions for using resources in GTK applications
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
#include "libr-gtk.h"
#include "libr-icons.h"
#include "tempfiles.h"

/* For loading GTK/GDK images */
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib/gthread.h>

/* For loading GLADE files */
#include <glade/glade.h>

/* For loading GTK+ Builder files */
#include <gtk/gtk.h>

/* For malloc/free */
#include <stdlib.h>

/* For string handling */
#include <string.h>

typedef gchar * (*GladeFileCallback)(GladeXML *, const gchar *, guint *);
GladeFileCallback glade_set_file_callback(GladeFileCallback callback, gpointer user_data);

/* Use weak binding for all glade and GTK+ requirements */
#pragma weak glade_set_file_callback

#pragma weak gtk_window_set_default_icon_list
#pragma weak gdk_pixbuf_loader_get_pixbuf
#pragma weak gtk_builder_add_from_string
#pragma weak gdk_pixbuf_loader_set_size
#pragma weak g_type_check_instance_cast
#pragma weak gtk_builder_add_from_file
#pragma weak glade_xml_new_from_buffer
#pragma weak gdk_pixbuf_loader_write
#pragma weak gdk_pixbuf_loader_close
#pragma weak gdk_pixbuf_loader_new
#pragma weak g_signal_connect_data
#pragma weak g_signal_connect
#pragma weak gtk_builder_new
#pragma weak g_object_unref
#pragma weak glade_xml_new
#pragma weak g_list_append
#pragma weak glade_init
#pragma weak gtk_init
#pragma weak g_free

#define GLADE_SECTION   ".glade"
#define BUILDER_SECTION ".ui"

/*
 * Handle the resource request from libglade
 */
gchar *libr_glade_read_resource(GladeXML *gladefile, const gchar *filename, guint *size, gpointer user_data)
{
	return libr_malloc((libr_file *) user_data, (char *) filename, (size_t *) size); 
}

/*
 * Handle the resource request from GtkBuilder
 */
gboolean libr_gtk_read_resource(GtkBuilder *builder, const gchar *filename, gchar **data, gsize *size, GError **error, gpointer user_data)
{
	if(data == NULL)
		return FALSE;
	*data = libr_malloc((libr_file *) user_data, (char *) filename, (size_t *) size);
	if(*data == NULL)
		return FALSE;
	return TRUE;
}

/*
 * Load the libglade resource appropriately for the currently installed version
 * (AKA, hurray hacks!)
 */
GladeXML *libr_new_glade(libr_file *handle, char *gladefile, size_t gladefile_size)
{
	if(glade_set_file_callback) /* The not-yet (ever?) existing way */
	{
		/* Register a callback for libglade to load our resources */
		if(glade_set_file_callback((GladeFileCallback) libr_glade_read_resource, handle) != NULL)
			printf("warning: over-wrote an application callback!\n");
		/* Initialize libglade almost as usual, just use a buffer instead of a file */
		return glade_xml_new_from_buffer(gladefile, gladefile_size, NULL, NULL);
	}
	else /* The hacky way */
	{
		char *glade_file[PATH_MAX];
		GladeXML *ret = NULL;
		char *temp_folder;
		
		temp_folder = libr_extract_resources(handle);
		if(temp_folder == NULL)
			return NULL;
		strcpy((char*)glade_file, temp_folder);
		strcat((char*)glade_file, "/");
		strcat((char*)glade_file, GLADE_SECTION);
		ret = glade_xml_new((char*)glade_file, NULL, NULL);
		if(ret == NULL)
			cleanup_folder(temp_folder);
		else
			register_folder_cleanup(temp_folder);
		return ret;
	}
}

/*
 * Load the GtkBuilder resource appropriately for the currently installed version
 * (AKA, hurray hacks!)
 */
int libr_new_builder(libr_file *handle, char *builderfile, size_t builderfile_size, GtkBuilder *builder)
{
	/* Register a callback for GtkBuilder to load our resources */
	if(g_signal_connect(builder, "load-resource", (GCallback) libr_gtk_read_resource, handle))
	{
		/* Initialize GtkBuilder almost as usual, just use a buffer instead of a file */
		if(gtk_builder_add_from_string(builder, builderfile, builderfile_size, NULL) == 0)
			return false;
		return true;
	}
	else /* The hacky way */
	{
		char *builder_file[PATH_MAX];
		char *temp_folder;
		int ret = false;
		
		temp_folder = libr_extract_resources(handle);
		if(temp_folder == NULL)
			return false;
		strcpy((char*)builder_file, temp_folder);
		strcat((char*)builder_file, "/");
		strcat((char*)builder_file, BUILDER_SECTION);
		ret = gtk_builder_add_from_file(builder, (char*)builder_file, NULL);
		if(ret == 0)
			cleanup_folder(temp_folder);
		else
			register_folder_cleanup(temp_folder);
		g_free(temp_folder);
		return (ret != 0);
	}
}

/*
 * Return a GTK icon list
 */
EXPORT_FN IconList *libr_gtk_iconlist(libr_file *handle)
{
	int sizes[] = {16, 32, 48, 96, 128};
	IconList *icons = NULL;
	GdkPixbuf *icon = NULL;
	int sizes_len = 5, i;
	
	if(handle == NULL)
	{
		/* Must pass a file handle to obtain the icons from */
		return NULL;
	}
	if(gtk_init == NULL)
	{
		/* GTK+ was not linked with the application */
		return false;
	}
	/* Go through the list of GTK "required" image sizes and build the icons */
	for(i=0;i<sizes_len;i++)
	{ 
		libr_icon *icon_handle = libr_icon_geticon_bysize(handle, sizes[i]);
		GdkPixbufLoader *stream = gdk_pixbuf_loader_new();
		char *iconfile = NULL;
		size_t iconfile_size;
		
		if(icon_handle == NULL)
		{
			/* Failed to find an icon */
			printf("Failed to find an icon\n");
			continue;
		}
		iconfile = libr_icon_malloc(icon_handle, &iconfile_size);
		if(iconfile == NULL)
		{
			/* Failed to obtain embedded icon */
			continue;
		}
		libr_icon_close(icon_handle);
		/* TODO: Use the "size-prepared" signal to properly scale the width and height
void user_function (GdkPixbufLoader *loader, gint width, gint height, gpointer user_data)
		 */ 
		gdk_pixbuf_loader_set_size(stream, sizes[i], sizes[i]);
		if(!gdk_pixbuf_loader_write(stream, (unsigned char *)iconfile, iconfile_size, NULL))
		{
			/* Failed to process image */
			continue;
		}
		if(!gdk_pixbuf_loader_close(stream, NULL))
		{
			/* Failed to create image */
			continue;
		}
		icon = gdk_pixbuf_loader_get_pixbuf(stream);
		if(icon == NULL)
		{
			/* Failed to convert image to pixbuf */
			continue;
		}
		icons = g_list_append(icons, icon);
	}
	return icons;
}

/*
 * Shared GtkBuilder resource loading
 */
GtkBuilder *libr_gtk_load_internal(libr_file *handle, char *resource_name)
{
	GtkBuilder *builder = NULL;
	size_t builder_length;
	char *builder_data;
	
	/* Obtain the GtkBuilder XML definition */
	builder_data = libr_malloc(handle, resource_name, &builder_length);
	if(builder_data == NULL)
	{
		/* Failed to obtain embedded GtkBuilder definition file */
		goto failed;
	}
	/* Setup the GtkBuilder environment */
	builder = gtk_builder_new();
	if(builder == NULL)
		goto failed;
	if(!libr_new_builder(handle, builder_data, builder_length, builder))
	{
		/* Failed to build interface from resource file */
		g_object_unref(G_OBJECT(builder));
		goto failed;
	}
failed:
	free(builder_data);
	return builder;
}

/*
 * Load the requested GtkBuilder resource and any applicable icons
 */
EXPORT_FN int libr_gtk_load(BuilderHandle **gtk_ret, char *resource_name)
{
	libr_file *handle;
	int ret = false;
	
	if(gtk_ret == NULL)
	{
		/* Why on earth would you call this without obtaining the handle to the resource? */
		return false;
	}
	if(gtk_builder_new == NULL)
	{
		/* GtkBuilder was not linked with the application */
		return false;
	}
	/* Obtain the handle to the executable */
	if((handle = libr_open(NULL, LIBR_READ)) == NULL)
	{
		/* "Failed to open this executable (%s) for resources", progname() */
		return false;
	}
	register_internal_handle(handle);
	*gtk_ret = libr_gtk_load_internal(handle, resource_name);
	if(*gtk_ret == NULL)
		goto failed;
	ret = true;
failed:
	if(!ret)
		libr_close(handle);
	return ret;
}

/*
 * Automatically load the ".ui" GtkBuilder resource and any applicable icons
 */
EXPORT_FN int libr_gtk_autoload(BuilderHandle **gtk_ret, IconList **icons_ret, int set_default_icon)
{
	GList *icons = NULL;
	libr_file *handle;
	int ret = false;
	
	if(gtk_ret == NULL)
	{
		/* Why on earth would you call this without obtaining the handle to the resource? */
		return false;
	}
	if(gtk_builder_new == NULL)
	{
		/* GtkBuilder was not linked with the application */
		return false;
	}
	/* Obtain the handle to the executable */
	if((handle = libr_open(NULL, LIBR_READ)) == NULL)
	{
		/* "Failed to open this executable (%s) for resources", progname() */
		return false;
	}
	register_internal_handle(handle);
	/* Obtain the icons from the ELF binary */
	icons = libr_gtk_iconlist(handle);
	/* Set the embedded icons as the default icon list (if requested) */
	if(icons != NULL && set_default_icon)
		gtk_window_set_default_icon_list(icons);
	*gtk_ret = libr_gtk_load_internal(handle, BUILDER_SECTION);
	if(*gtk_ret == NULL)
		goto failed;
	if(icons_ret)
		*icons_ret = icons;
	ret = true;
failed:
	if(!ret)
		libr_close(handle);
	return ret;
}

/*
 * Shared libglade resource loading
 */
GladeXML *libr_glade_load_internal(libr_file *handle, char *resource_name)
{
	GladeXML *glade = NULL;
	size_t glade_length;
	char *glade_data;
	
	/* Obtain the GLADE XML definition */
	glade_data = libr_malloc(handle, resource_name, &glade_length);
	if(glade_data == NULL)
	{
		/* Failed to obtain embedded glade file */
		goto failed;
	}
	/* Initialize libglade appropriate for the available version */
	glade = libr_new_glade(handle, glade_data, glade_length);
	if(glade == NULL)
	{
		/* Failed to initialize embedded glade file */
		goto failed;
	}
failed:
	free(glade_data);
	return glade;
}

/*
 * Load the requested libglade resource and any applicable icons
 */
EXPORT_FN int libr_glade_load(GladeHandle **glade_ret, char *resource_name)
{
	libr_file *handle;
	int ret = false;
	
	if(glade_ret == NULL)
	{
		/* Why on earth would you call this without obtaining the handle to the resource? */
		return false;
	}
	if(glade_init == NULL)
	{
		/* libglade was not linked with the application */
		return false;
	}
	/* Obtain the handle to the executable */
	if((handle = libr_open(NULL, LIBR_READ)) == NULL)
	{
		/* "Failed to open this executable (%s) for resources", progname() */
		return false;
	}
	register_internal_handle(handle);
	*glade_ret = libr_glade_load_internal(handle, resource_name);
	if(*glade_ret == NULL)
		goto failed;
	ret = true;
failed:
	if(!ret)
		libr_close(handle);
	return ret;
}

/*
 * Automatically load the ".glade" resource and any applicable icons
 */
EXPORT_FN int libr_glade_autoload(GladeHandle **glade_ret, IconList **icons_ret, int set_default_icon)
{
	libr_file *handle = NULL;
	GList *icons = NULL;
	
	if(glade_ret == NULL)
	{
		/* Why on earth would you call this without obtaining the handle to the resource? */
		return false;
	}
	if(glade_init == NULL)
	{
		/* libglade was not linked with the application */
		return false;
	}
	/* Obtain the handle to the executable */
	if((handle = libr_open(NULL, LIBR_READ)) == NULL)
	{
		/* "Failed to open this executable (%s) for resources", progname() */
		return false;
	}
	register_internal_handle(handle);
	icons = libr_gtk_iconlist(handle);
	/* Set the embedded icons as the default icon list (if requested) */
	if(icons != NULL && set_default_icon)
		gtk_window_set_default_icon_list(icons);
	/* Return the libglade and icon handles for the application */
	*glade_ret = libr_glade_load_internal(handle, GLADE_SECTION);
	if(icons_ret)
		*icons_ret = icons;
	return true;
}
