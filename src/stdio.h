#pragma once

#include <stdio.h>
#if defined(_WIN32)
# include <stdarg.h>
#endif

#if defined(_WIN32)
# define vscprintf _vscprintf
#endif

#if defined(_WIN32)
int asprintf(char **buf, const char *fmt, ...);
int vasprintf(char **buf, const char *fmt, va_list ap);
#endif