#ifndef __LIBR_BACKENDS_H
#define __LIBR_BACKENDS_H

/*
 * All of the backend functions are explicitly declared internal to prevent any custom backend
 * from leaving out one of these critical functions.
 */
INTERNAL_FN libr_intstatus add_section(libr_file *file_handle, char *resource_name, libr_section **retscn);
INTERNAL_FN void *data_pointer(libr_section *scn, libr_data *data);
INTERNAL_FN size_t data_size(libr_section *scn, libr_data *data);
INTERNAL_FN libr_intstatus find_section(libr_file *file_handle, char *section, libr_section **retscn);
INTERNAL_FN libr_data *get_data(libr_file *file_handle, libr_section *scn);
INTERNAL_FN void initialize_backend(void);
INTERNAL_FN libr_data *new_data(libr_file *file_handle, libr_section *scn);
INTERNAL_FN libr_section *next_section(libr_file *file_handle, libr_section *scn);
INTERNAL_FN libr_intstatus remove_section(libr_file *file_handle, libr_section *scn);
INTERNAL_FN char *section_name(libr_file *file_handle, libr_section *scn);
INTERNAL_FN libr_intstatus set_data(libr_file *file_handle, libr_section *scn, libr_data *data, off_t offset, char *buffer, size_t size);
INTERNAL_FN libr_intstatus open_handles(libr_file *file_handle, char *filename, libr_access_t access);
INTERNAL_FN void write_output(libr_file *file_handle);

#endif /* __LIBR_BACKENDS_H */
