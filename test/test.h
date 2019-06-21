#pragma once

#if defined(_WIN32)
#else
# define PR "\x1B[31m"
# define PG "\x1B[32m"
# define PX "\x1B[0m"
#endif

void test_printf(const char *module, const char *fmt, ...);

int test_run(const char *module, unsigned int number, const char *name, int (*test_func)(void *), void *user_data);
