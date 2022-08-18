#include <stdio.h>
#include "../src/scott.h"
#include "test.h"
#include "alist.h"
#include "shapefile.h"

#define MODULE "Main"

int
main(int argc, char **argv) {
    int count;

    test_printf(MODULE, "Starting");

    //count = alist_test();
    count = shapefile_test();

    test_printf(MODULE, "Done");

    return count;
}
