#ifndef __LIBR_I18N_H
#define __LIBR_I18N_H

#include "libr.h"
#include "gettext.h"

#define _(string) gettext(string)
/* for strings used in structures (must manually call gettext!): */
#define N_(string) (string)

int libr_i18n_autoload(const char *domain);
int libr_i18n_load(libr_file *handle, const char *domain);

#endif /* __LIBR_I18N_H */
