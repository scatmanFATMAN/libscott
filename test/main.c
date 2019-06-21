#include <stdio.h>
#include "../src/scott.h"
#include "test.h"
#include "alist.h"

#define MODULE "Main"

int
main(int argc, char **argv) {
    int count;

    test_printf(MODULE, "Starting");

    count = alist_test();

    test_printf(MODULE, "Done");

    return count;
}
