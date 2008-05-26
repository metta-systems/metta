#include "common.h"

void *memset (void *__s, int __c, size_t __n)
{
	uint8_t *c = (uint8_t *)__s;
	for(; __n > 0; __n--)
		*c++ = __c;
	return __s;
}
