#include <stdlib.h>
#include "stdio.h"

#if defined(_WIN32)
int
asprintf(char **buf, const char *fmt, ...) {
    va_list ap;
    int len;

    va_start(ap, fmt);
    len = vasprintf(buf, fmt, ap);
    va_end(ap);

    return len;
}

int
vasprintf(char **buf, const char *fmt, va_list ap) {
    char *str;
    int len;

    len = vscprintf(fmt, ap);
    if (len == -1) {
        return -1;
    }

    str = malloc((size_t)len + 1);
    if (str == NULL) {
        return -1;
    }

    len = vsnprintf(str, len + 1, fmt, ap);
    if (len == -1) {
        free(str);
        return -1;
    }

    *buf = str;
    return len;
}
#endif