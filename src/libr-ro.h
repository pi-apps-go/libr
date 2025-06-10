#ifndef __LIBRRO_H
#define __LIBRRO_H

/* For file handle support */
#include <stdio.h>

/* For integer types with set bit-sizes */
#include <stdint.h>

/* 
 * NOTE: Packing the enum uses the smallest number of bytes
 * possible to represent the value.  This packing does not
 * guarantee that a "short enum" will be 8 bits, however,
 * for the small enumerations in the ELF specification this
 * IS the case (no enum requires more than 8 bits).
 */
#define SHORT_ENUM __attribute__ ((__packed__))

/* Type of byte-packing (endian) */
typedef enum SHORT_ENUM {
	ELFDATANONE, /* Invalid */
	ELFDATA2LSB, /* Least Significant Byte First */
	ELFDATA2MSB, /* Most Significant Byte First */
	ELFDATAMAX,  /* Invalid */
} eEncoding;

/* Type of target processor */
typedef enum SHORT_ENUM {
	ELFCLASSNONE, /* Invalid */
	ELFCLASS32,   /* 32-bit Field Alignment */
	ELFCLASS64,   /* 64-bit Field Alignment */
	ELFCLASSMAX,  /* Invalid */
} eClass;

#ifndef DOXYGEN_SHOULD_SKIP_THIS

#define ELFSTRING_MAX 200
typedef struct _libr_section {
	uint64_t size;
	uint64_t data_offset;
	uint32_t name_offset;
	char name[ELFSTRING_MAX];
} libr_section;

typedef struct _libr_file {
	FILE *handle;
	char *filename;
	eEncoding endian;
	eClass byte_size;
	libr_access_t access;
	libr_section *secdata;
	unsigned long total_sections;
} libr_file;

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

/* for a clean internal API */
typedef void libr_data;

#define enum_valid(val, name) (val > name##NONE && val < name##MAX)

#endif /* __LIBRRO_H */
