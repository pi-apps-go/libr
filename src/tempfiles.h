#ifndef __TEMPFILES_H
#define __TEMPFILES_H

#include "libr.h"

void cleanup_folder(char *temp_folder);
void register_handle_cleanup(libr_file *handle);
void unregister_handle_cleanup(libr_file *handle);
void register_internal_handle(libr_file *handle);
void register_folder_cleanup(char *temp_folder);
char *libr_extract_resources(libr_file *handle);

#endif /* __TEMPFILES_H */
