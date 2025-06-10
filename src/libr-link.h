#ifndef __LIBR_LINK_H
#define __LIBR_LINK_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

typedef struct {
	void **symbol;
	char *name;
} symbol_table;

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

#define SYMBOL_TABLE(tbl, ...) \
const symbol_table tbl[] = { \
        __VA_ARGS__ \
        {NULL, NULL} \
}

#define OVERRIDE_SYMBOL(a)            FN_##a
#define SYMBOL(sym)                   {(void **)&FN_##sym, #sym}
#define DEFINE_SYMBOL(ret, fn, ...)   ret (*OVERRIDE_SYMBOL(fn))(__VA_ARGS__)
#define LOAD_SYMBOLS(lib, tbl)        load_symbols(lib, tbl, sizeof(tbl)/sizeof(symbol_table))

int load_symbols(const char *library, const symbol_table *table, int entries);

#endif /* __LIBR_LINK_H */
