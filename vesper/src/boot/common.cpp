#include "common.h"
#include "DefaultConsole.h"

// Copy len bytes from src to dest.
// void memcpy(void *dest, const uint8_t *src, uint32_t len)
// {
//     const uint8_t *sp = (const uint8_t *)src;
//     uint8_t *dp = (uint8_t *)dest;
//     for(; len != 0; len--) *dp++ = *sp++;
// }

// Write __n copies of __c into __s.
// void *memset (void *__s, int __c, size_t __n)
// {
// 	uint8_t *c = (uint8_t *)__s;
// 	for(; __n > 0; __n--)
// 		*c++ = __c;
// 	return __s;
// }

// Compare two strings. Should return -1 if
// str1 < str2, 0 if they are equal or 1 otherwise.
// int strcmp(char *str1, char *str2)
// {
// 	int result = 0;
//
// 	if (!str1 && !str2) return 0;
// 	if (!str1) return -1;
// 	if (!str2) return 1;
//
// 	do
// 	{
// 		result = *str1++ - *str2++;
// 	}
// 	while(!result && (*(str1-1) != '\0' && *(str2-1) != '\0'));
//
// 	return result;
// }

// Copy the NULL-terminated string src into dest, and
// return dest.
// char *strcpy(char *dest, const char *src)
// {
// 	char *orig_dest = dest;
// 	do
// 	{
// 		*dest++ = *src++;
// 	}
// 	while (*src != 0);
// 	*dest = 0;
// 	return orig_dest;
// }

// Concatenate the NULL-terminated string src onto
// the end of dest, and return dest.
// char *strcat(char *dest, const char *src)
// {
//     while (*dest != 0)
//     {
//         dest++;
//     }
//
//     do
//     {
//         *dest++ = *src++;
//     }
//     while (*src != 0);
//     return dest;
// }

// int strlen(char *src)
// {
//     int i = 0;
//     while (*src++)
//         i++;
//     return i;
// }

// =========================================================================== //

void panic(const char *message, const char *file, uint32_t line)
{
	// We encountered a massive problem and have to stop.
	asm volatile("cli"); // Disable interrupts.

	kconsole.set_attr(RED, YELLOW);
	kconsole.print("PANIC(");
	kconsole.print(message);
	kconsole.print(") at ");
	kconsole.print(file);
	kconsole.print(":");
	kconsole.print_int(line);
	kconsole.newline();

	// Halt by going into an infinite loop.
	while(1) {}
}

void panic_assert(const char *desc, const char *file, uint32_t line)
{
	// An assertion failed, and we have to panic.
	asm volatile("cli"); // Disable interrupts.

	kconsole.set_attr(WHITE, RED);
	kconsole.print("ASSERTION-FAILED(");
	kconsole.print(desc);
	kconsole.print(") at ");
	kconsole.print(file);
	kconsole.print(":");
	kconsole.print_int(line);
	kconsole.newline();

	// Halt by going into an infinite loop.
	while(1) {}
}
