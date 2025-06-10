#ifndef __CVTENDIAN_H
#define __CVTENDIAN_H

/* Support for swapping bytes (endian conversion) */
#include <byteswap.h>

/* For obtaining the host endian type */
#include <endian.h>
#if (__BYTE_ORDER == __LITTLE_ENDIAN)
#	define HOST_ENDIAN  ELFDATA2LSB
#elif (__BYTE_ORDER == __BIG_ENDIAN)
#	define HOST_ENDIAN  ELFDATA2MSB
#else
#	error "Failed to detect host endian type"
#endif

/*
 * Convert the endian of a parameter
 */
static int ConvertEndian(void *ptr, int bytes)
{
	switch(bytes)
	{
		case 2:
		{
			uint16_t *value = (uint16_t *) ptr;
			
			*value = bswap_16(*value);
		}	return 1;
		case 4:
		{
			uint32_t *value = (uint32_t *) ptr;
			
			*value = bswap_32(*value);
		}	return 1;
		case 8:
		{
			uint64_t *value = (uint64_t *) ptr;
			
			*value = bswap_64(*value);
		}	return 1;
		default:
			break;
	}
	return 0;
}

#endif /* __CVTENDIAN_H */
