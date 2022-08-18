#pragma once

#include <stdbool.h>

#define SHAPEFILE_TYPE_NULL         0
#define SHAPEFILE_TYPE_POINT        1
#define SHAPEFILE_TYPE_POLYLINE     3
#define SHAPEFILE_TYPE_POLYGON      5
#define SHAPEFILE_TYPE_MULTIPOINT   8
#define SHAPEFILE_TYPE_POINT_Z      11
#define SHAPEFILE_TYPE_POLYLINE_Z   13
#define SHAPEFILE_TYPE_POLYGON_Z    15
#define SHAPEFILE_TYPE_MULTIPOINT_Z 18
#define SHAPEFILE_TYPE_POINT_M      21
#define SHAPEFILE_TYPE_POLYLINE_M   23
#define SHAPEFILE_TYPE_POLYGON_M    25
#define SHAPEFILE_TYPE_MULTIPOINT_M 28
#define SHAPEFILE_TYPE_MULTIPATCH   31

typedef struct shapefile_t shapefile_t;
typedef struct shapefile_shape_t shapefile_shape_t;

typedef struct {
    bool (*shape)(shapefile_shape_t *shape, void *user_data);
    void *user_data;
} shapefile_parse_cb_t;

shapefile_t * shapefile_init();
void shapefile_free(shapefile_t *shapefile);

bool shapefile_parse_cb(shapefile_t *shapefile, const char *path, shapefile_parse_cb_t *cb);

const char * shapefile_error(shapefile_t *shapefile);