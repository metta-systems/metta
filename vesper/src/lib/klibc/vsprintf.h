//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
//
// vsprintf family
//
#pragma once

// ==================================================================
// from vsprintf.c
// FIXME: do we need to export this uglyness at all?

extern "C" unsigned long simple_strtoul(const char *,char **,unsigned int);
extern "C" long simple_strtol(const char *,char **,unsigned int);
extern "C" unsigned long long simple_strtoull(const char *,char **,unsigned int);
extern "C" long long simple_strtoll(const char *,char **,unsigned int);
extern "C" int strict_strtoul(const char *, unsigned int, unsigned long *);
extern "C" int strict_strtol(const char *, unsigned int, long *);
extern "C" int strict_strtoull(const char *, unsigned int, unsigned long long *);
extern "C" int strict_strtoll(const char *, unsigned int, long long *);
extern "C" int sprintf(char * buf, const char * fmt, ...)
                __attribute__ ((format (printf, 2, 3)));
extern "C" int vsprintf(char *buf, const char *, va_list)
                __attribute__ ((format (printf, 2, 0)));
extern "C" int snprintf(char * buf, size_t size, const char * fmt, ...)
                __attribute__ ((format (printf, 3, 4)));
extern "C" int vsnprintf(char *buf, size_t size, const char *fmt, va_list args)
                __attribute__ ((format (printf, 3, 0)));
extern "C" int scnprintf(char * buf, size_t size, const char * fmt, ...)
                __attribute__ ((format (printf, 3, 4)));
extern "C" int vscnprintf(char *buf, size_t size, const char *fmt, va_list args)
                __attribute__ ((format (printf, 3, 0)));

extern "C" int sscanf(const char *, const char *, ...)
                __attribute__ ((format (scanf, 2, 3)));
extern "C" int vsscanf(const char *, const char *, va_list)
                __attribute__ ((format (scanf, 2, 0)));

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
