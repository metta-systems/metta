#pragma once

// i386
#define _JBLEN 9
#define _JBTYPE int

typedef _JBTYPE jmp_buf[_JBLEN];

#define setjmp(buf) __builtin_setjmp(buf)
#define longjmp(buf, v) __builtin_longjmp(buf, v)
