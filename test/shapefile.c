#include <stdlib.h>
#include "../src/scott.h"
#include "test.h"
#include "shapefile.h"

#define MODULE "shapefile"

static bool
shapefile_shape(shapefile_shape_t *shape, void *user_data) {
    char *wkt;

    wkt = shapefile_shape_wkt(shape);

    printf("New shape\n");
    printf(" %s\n", wkt);

    free(wkt);

    return true;
}

static int
shapefile_test_main(void *user_data) {
    bool success;
    shapefile_t *file;
    shapefile_parse_cb_t cb;

    cb.shape = shapefile_shape;
    cb.user_data = NULL;

    file = shapefile_init();
    success = shapefile_parse_cb(file, "shapefile/Test_shapefile_AO", &cb);
    if (!success) {
        printf("ERROR: %s\n", shapefile_error(file));
    }
    shapefile_free(file);

    return success ? 0 : 1;
}

int
shapefile_test() {
    int count;

    count = test_run(MODULE, 1, "Main", shapefile_test_main, NULL);

    return count;
}
