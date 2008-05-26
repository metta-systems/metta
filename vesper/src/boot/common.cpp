#include "Types.h"

void *memset (void *__s, int __c, size_t __n)
{
	char *c = (char *)__s;
	for(size_t i = 0; i < __n; i++)
		c[i] = __c;
	return __s;
}
