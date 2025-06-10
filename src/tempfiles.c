/*
 *
 *  Copyright (c) 2009 Erich Hoover
 *
 *  libr temp files - Handle temporary files and handles that require cleanup
 *                    when libr closes.
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

#include "tempfiles.h"

/* For malloc/free and mkdtemp */
#include <stdlib.h>

/* For string handling */
#include <string.h>
#include <stdio.h>

/* For directory cleanup */
#include <unistd.h>
#include <dirent.h>

/* For directory creation */
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/* Hold on to folder names for cleanup when libr is removed from memory */
typedef struct CLEANUPFOLDER {
	char *folder;
	struct CLEANUPFOLDER *next;
} CleanupFolder;
CleanupFolder *folders_to_remove = NULL;

/* Hold on to libr handles for cleanup when libr is removed from memory */
typedef struct CLEANUPHANDLE {
	int internal; /* do not warn the user about cleaning this handle up */
	libr_file *handle;
	struct CLEANUPHANDLE *next;
} CleanupHandle;
CleanupHandle *handles_to_remove = NULL;

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

/*
 * Register a folder for cleanup when libr is removed from memory
 */
void register_folder_cleanup(char *temp_folder)
{
	CleanupFolder *folder = malloc(sizeof(CleanupFolder));
	
	folder->folder = strdup(temp_folder);
	folder->next = NULL;
	if(folders_to_remove != NULL)
	{
		CleanupFolder *f;
		
		for(f = folders_to_remove; f->next != NULL; f = f->next) {}
		f->next = folder;
	}
	else
		folders_to_remove = folder;
}

/*
 * Register a libr handle for cleanup when libr is removed from memory
 */
void register_handle_cleanup(libr_file *handle)
{
	CleanupHandle *h = malloc(sizeof(CleanupHandle));
	
	h->handle = handle;
	h->internal = FALSE;
	h->next = NULL;
	if(handles_to_remove != NULL)
	{
		CleanupHandle *i;
		
		for(i = handles_to_remove; i->next != NULL; i = i->next) {}
		i->next = h;
	}
	else
		handles_to_remove = h;
}

/*
 * Remove a libr handle from the cleanup list
 */
void unregister_handle_cleanup(libr_file *handle)
{
	CleanupHandle *i, *last = NULL;
	int found = FALSE;

	if(handles_to_remove == NULL)
	{
		printf("Unregistering handle with no list of cleanup handles!\n");
		return;
	}
	for(i = handles_to_remove; i != NULL; last = i, i = i->next)
	{
		if(i->handle == handle)
		{
			if(last == NULL)
				handles_to_remove = i->next;
			else
				last->next = i->next;
			free(i);
			found = TRUE;
			break;
		}
	}
	if(!found)
		printf("Could not find handle to remove from cleanup list!\n");
}

/*
 * Flag a handle as internal (do not warn about unsafe cleanup)
 */
void register_internal_handle(libr_file *handle)
{
	int found = FALSE;
	CleanupHandle *i;

	if(handles_to_remove == NULL)
	{
		printf("No cleanup list!\n");
		return;
	}
	for(i = handles_to_remove; i != NULL; i = i->next)
	{
		if(i->handle == handle)
		{
			i->internal = TRUE;
			found = TRUE;
			break;
		}
	}
	if(!found)
		printf("Could not find handle in cleanup list!\n");
}

/*
 * Cleanup a temporary folder used to hack the inability to load resources from a buffer
 */
void cleanup_folder(char *temp_folder)
{
	char *filepath = (char *) malloc(PATH_MAX);
	DIR *dir = opendir(temp_folder);
	struct dirent *file;
	
	while((file = readdir(dir)) != NULL)
	{
		char *filename = file->d_name;
		
		/* Do not delete "self" or "parent" directory entries */
		if(!strcmp(filename, ".") || !strcmp(filename, ".."))
			continue;
		/* But delete anything else */
		strcpy(filepath, temp_folder);
		strcat(filepath, "/");
		strcat(filepath, filename);
		if(file->d_type == DT_DIR)
			cleanup_folder(filepath);
		else
		{
			if(unlink(filepath))
				printf("libr failed to cleanup '%s' in temporary folder: %m\n", filename);
		}
	}
	free(filepath);
	closedir(dir);
	if(rmdir(temp_folder) != 0)
		printf("libr failed to remove temporary folder: %m\n");
}

/*
 * Perform cleanup when libr is removed from memory
 */
void do_cleanup(void) __attribute__((destructor));
void do_cleanup(void)
{
	CleanupFolder *f, *fnext;
	CleanupHandle *h, *hnext;
	
	/* Cleanup folders */
	for(f = folders_to_remove; f != NULL; f = fnext)
	{
		folders_to_remove = NULL;
		fnext = f->next;
		cleanup_folder(f->folder);
		free(f->folder);
		free(f);
	}
	/* Cleanup handles */
	for(h = handles_to_remove; h != NULL; h = hnext)
	{
		handles_to_remove = NULL;
		hnext = h->next;
		/* Unless the handle was created internally then warn the developer to cleanup their act */
		if(!h->internal)
			printf("Warning: Application did not cleanup resource handle: %p\n", h->handle);
		libr_close_internal(h->handle);
		free(h);
	}
}

/*
 * Build all the directories required by a resource
 * (and construct the output string)
 */
int make_valid_path(char *out_path, size_t maxpath, char *start_folder, char *resource_name)
{
	char *a, *c = resource_name;
	
	strcpy(out_path, start_folder);
	while((a=strchr(c, '/')) != NULL)
	{
		strcat(out_path, "/");
		strncat(out_path, c, (size_t) (a-c));
		if(mkdir(out_path, S_IRUSR|S_IWUSR|S_IXUSR) != 0)
		{
			if(errno != EEXIST)
			{
				printf("failed to make directory: %s %m\n", out_path);
				return false;
			}
		}
		c = a+1;
	}
	strcat(out_path, "/");
	strcat(out_path, c);
	return true;
}

/*
 * Extract all the resources from the ELF file for use by the resource loader
 */
char *libr_extract_resources(libr_file *handle)
{
	char *temp_mask = strdup(LIBR_TEMPFILE);
	char *temp_folder;
	int i = 0;
	
	temp_folder = mkdtemp(temp_mask);
	if(temp_folder == NULL)
	{
		/* failed to extract ELF resources, could not create a temporary path */
		goto failed;
	}
	/* If this library cannot dynamically load resources then pull out all the resources to a temporary directory */
	for(i=0;i<libr_resources(handle);i++)
	{
		char *resource_name = libr_list(handle, i);
		char *file_path[PATH_MAX];
		size_t resource_size;
		FILE *file_handle;
		char *resource;
		
		resource = libr_malloc(handle, resource_name, &resource_size);
		if(!make_valid_path((char *)file_path, sizeof(file_path), temp_folder, resource_name))
		{
			/* failed to build the path required by a resource */
			cleanup_folder(temp_folder);
			temp_folder = NULL;
			goto failed;
		}
		file_handle = fopen((const char *) file_path, "w");
		if(file_handle == NULL)
		{
			/* failed to extract ELF resources, could not write to temporary path */
			cleanup_folder(temp_folder);
			temp_folder = NULL;
			goto failed;
		}
		/* if the resource is empty then fwrite will fail */
		if( (resource_size != 0) && (fwrite(resource, resource_size, 1, file_handle) != 1) )
		{
			/* failed to extract ELF resources, temporary path out of space? */
			cleanup_folder(temp_folder);
			temp_folder = NULL;
			goto failed;
		}
		fclose(file_handle);
		free(resource);
	}
failed:
	if(temp_folder != NULL)
		temp_folder = strdup(temp_folder);
	free(temp_mask);
	return temp_folder;
}
