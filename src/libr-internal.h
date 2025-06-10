#ifndef __LIBR_INTERNAL_H
#define __LIBR_INTERNAL_H

#define false                       0
#define true                        1
#define ERROR                      -1
#define EXPORT_FN                  __attribute__((visibility ("protected")))
#define INTERNAL_FN                __attribute__ ((visibility ("internal")))
#define LIBR_TEMPFILE              "/tmp/libr-temp.XXXXXX"
#define LIBR_TEMPFILE_LEN          22

#ifndef DOXYGEN_SHOULD_SKIP_THIS

typedef struct {
	char *message;
	libr_status status;
	const char *function;
} libr_intstatus;

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

struct _libr_file;

void libr_set_error(libr_intstatus error);
libr_intstatus make_status(const char *function, libr_status code, char *message, ...);
/* Only called directly by cleanup routine, all other calls should be through libr_close */
void libr_close_internal(struct _libr_file *file_handle);

#define SET_ERROR(code,...)           make_status(__FUNCTION__, code, __VA_ARGS__)
#define RETURN(code,...)              return SET_ERROR(code, __VA_ARGS__)
#define RETURN_OK                     return SET_ERROR(LIBR_OK, NULL)
#define PUBLIC_RETURN(code,message)   {SET_ERROR(code, message); return (code == LIBR_OK);}

#endif /* __LIBR_INTERNAL_H */
