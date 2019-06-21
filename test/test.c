#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>
#include "test.h"

void
test_printf(const char *module, const char *fmt, ...) {
    char *text;
    va_list ap;
    struct tm tm;
    struct timeval tv;

    va_start(ap, fmt);
    vasprintf(&text, fmt, ap);
    va_end(ap);

    gettimeofday(&tv, NULL);
    localtime_r(&tv.tv_sec, &tm);

    printf("[%02d:%02d:%02d.%03d] [%-7s] %s\n", tm.tm_hour, tm.tm_min, tm.tm_sec, (int)tv.tv_usec / 1000, module, text);
    free(text);
}

int
test_run(const char *module, unsigned int number, const char *name, int (*test_func)(void *), void *user_data) {
    int count;

    test_printf(module, "-----------------------------------------------");
    test_printf(module, "Test %d: %s", number, name);

    count = test_func(user_data);

    if (count == 0) {
        test_printf(module, PG"Success!"PX);
    }
    else {
        test_printf(module, PR"Failed"PX);
    }

    return count;
}

