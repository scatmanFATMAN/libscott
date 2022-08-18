#pragma once

#include <string.h>

#if defined(_WIN32)
# define strdup _strdup
#endif

#if defined(_WIN32)
size_t strlcpy(char *dst, const char *src, size_t size);
#endif