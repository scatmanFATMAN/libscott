#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include "stdio.h"
#include "string.h"
#include "endian.h"
#include "shapefile.h"

//https://www.esri.com/content/dam/esrisites/sitecore-archive/Files/Pdfs/library/whitepapers/pdfs/shapefile.pdf

#define SHAPEFILE_HEADER_MAGIC 0x0000270a

#define SHAPEFILE_HEADER_SIZE ((9 * sizeof(int32_t)) + (8 * sizeof(double)))

#define SHAPEFILE_SHP_RECORD_SIZE (2 * sizeof(int32_t))

#define SHAPEFILE_SHX_RECORD_SIZE (2 * sizeof(int32_t))

typedef struct {
    double x;
    double y;
} shapefile_shape_point_t;

struct shapefile_shape_t {
    int32_t type;
    void *geometry;
};

typedef struct {
    double min_x;
    double min_y;
    double max_x;
    double max_y;
} shapefile_mbr_t;

typedef struct {
    struct {
        double min;
        double max;
    } z;
    struct {
        double min;
        double max;
    } m;
} shapefile_range_t;

typedef struct {
    int32_t code;               //always SHAPEFILE_HEADER_MAGIC
    int32_t unused[5];
    int32_t length;             //length of file, including header
    int32_t version;
    int32_t type;
    shapefile_mbr_t mbr;        //minimum bounding rectangle
    shapefile_range_t range;    //range of Z (min/max) and M (min/max)
} shapefile_header_t;

typedef struct {
    FILE *f;
    shapefile_header_t header;
} shapefile_shp_t;

typedef struct {
    int32_t number;             //record number (1- based)
    int32_t length;             //record length
} shapefile_shp_record_header_t;

typedef struct {
    int32_t type;               //shape type
    shapefile_shape_t *shape;
} shapefile_shp_record_t;

typedef struct {
    FILE *f;
} shapefile_shx_t;

typedef struct {
    int32_t offset;             //record offset in the .shp file
    int32_t length;             //record length in the .shp file
} shapefile_shx_record_t;

struct shapefile_t {
    shapefile_shp_t shp;
    shapefile_shx_t shx;
    char error[256];
};

static const char *
shapefile_type_str(int32_t type) {
    switch (type) {
        case SHAPEFILE_TYPE_NULL:         return "Null";
        case SHAPEFILE_TYPE_POINT:        return "Point";
        case SHAPEFILE_TYPE_POLYLINE:     return "Polyline";
        case SHAPEFILE_TYPE_POLYGON:      return "Polygon";
        case SHAPEFILE_TYPE_MULTIPOINT:   return "MultiPoint";
        case SHAPEFILE_TYPE_POINT_Z:      return "PointZ";
        case SHAPEFILE_TYPE_POLYLINE_Z:   return "PolylineZ";
        case SHAPEFILE_TYPE_POLYGON_Z:    return "PolygonZ";
        case SHAPEFILE_TYPE_MULTIPOINT_Z: return "MultiPointZ";
        case SHAPEFILE_TYPE_POINT_M:      return "PointM";
        case SHAPEFILE_TYPE_POLYLINE_M:   return "PolylineM";
        case SHAPEFILE_TYPE_POLYGON_M:    return "PolygonM";
        case SHAPEFILE_TYPE_MULTIPOINT_M: return "MultiPointM";
        case SHAPEFILE_TYPE_MULTIPATCH:   return "MultiPatch";
    }

    return "Invalid";
}

static bool
shapefile_type_valid(int32_t type) {
    switch (type) {
        case SHAPEFILE_TYPE_NULL:
        case SHAPEFILE_TYPE_POINT:
        case SHAPEFILE_TYPE_POLYLINE:
        case SHAPEFILE_TYPE_POLYGON:
        case SHAPEFILE_TYPE_MULTIPOINT:
        case SHAPEFILE_TYPE_POINT_Z:
        case SHAPEFILE_TYPE_POLYLINE_Z:
        case SHAPEFILE_TYPE_POLYGON_Z:
        case SHAPEFILE_TYPE_MULTIPOINT_Z:
        case SHAPEFILE_TYPE_POINT_M:
        case SHAPEFILE_TYPE_POLYLINE_M:
        case SHAPEFILE_TYPE_POLYGON_M:
        case SHAPEFILE_TYPE_MULTIPOINT_M:
        case SHAPEFILE_TYPE_MULTIPATCH:
            return true;
    }

    return false;
}

static shapefile_shape_t *
shapefile_shape_new(int32_t type) {
    shapefile_shape_t *shape;

    shape = malloc(sizeof(*shape));
    if (shape == NULL) {
        return NULL;
    }

    shape->type = type;
    shape->geometry = NULL;

    return shape;
}

static void
shapefile_shape_point_free(shapefile_shape_point_t *point) {
}

static void
shapefile_shape_free(shapefile_shape_t *shape) {
    switch (shape->type) {
        case SHAPEFILE_TYPE_NULL:
            break;
        case SHAPEFILE_TYPE_POINT:
            shapefile_shape_point_free(shape->geometry);
            break;
        case SHAPEFILE_TYPE_POLYLINE:
        case SHAPEFILE_TYPE_POLYGON:
        case SHAPEFILE_TYPE_MULTIPOINT:
        case SHAPEFILE_TYPE_POINT_Z:
        case SHAPEFILE_TYPE_POLYLINE_Z:
        case SHAPEFILE_TYPE_POLYGON_Z:
        case SHAPEFILE_TYPE_MULTIPOINT_Z:
        case SHAPEFILE_TYPE_POINT_M:
        case SHAPEFILE_TYPE_POLYLINE_M:
        case SHAPEFILE_TYPE_POLYGON_M:
        case SHAPEFILE_TYPE_MULTIPOINT_M:
        case SHAPEFILE_TYPE_MULTIPATCH:
            break;
    }

    //SHAPEFILE_TYPE_NULL's geometry will be NULL
    if (shape->geometry != NULL) {
        free(shape->geometry);
    }

    free(shape);
}

static bool
shapefile_read(shapefile_t *shapefile, FILE *f, void *buf, size_t len, int32_t *length) {
    size_t count;

    count = fread(buf, 1, len, f);
    if (count != len) {
        snprintf(shapefile->error, sizeof(shapefile->error), "Error reading %zu bytes: Only read %zu", len, count);
        return false;
    }

    if (length != NULL) {
        *length -= (int32_t)count;
    }

    return true;
}

static bool
shapefile_read_int16_be(shapefile_t *shapefile, FILE *f, int16_t *value, int32_t *length) {
    int16_t data;

    if (!shapefile_read(shapefile, f, &data, sizeof(data), length)) {
        return false;
    }

    *value = be16toh(data);
    return true;
}

static bool
shapefile_read_int32_le(shapefile_t *shapefile, FILE *f, int32_t *value, int32_t *length) {
    int32_t data;

    if (!shapefile_read(shapefile, f, &data, sizeof(data), length)) {
        return false;
    }

    *value = le32toh(data);
    return true;
}

static bool
shapefile_read_int32_be(shapefile_t *shapefile, FILE *f, int32_t *value, int32_t *length) {
    int32_t data;

    if (!shapefile_read(shapefile, f, &data, sizeof(data), length)) {
        return false;
    }

    *value = be32toh(data);
    return true;
}

static bool
shapefile_read_double_le(shapefile_t *shapefile, FILE *f, double *value, int32_t *length) {
    double data;

    if (!shapefile_read(shapefile, f, &data, sizeof(data), length)) {
        return false;
    }

    *value = le64toh(data);
    return true;
}

static bool
shapefile_read_int32_size_be(shapefile_t *shapefile, FILE *f, int32_t *value, int32_t *length) {
    int16_t low, high;
    bool success;

    success = shapefile_read_int16_be(shapefile, f, &low, length) &&
              shapefile_read_int16_be(shapefile, f, &high, length);

    if (success) {
        *value = (high << 16) | (low & 0xFFFF);
    }

    return success;
}

static bool
shapefile_read_header(shapefile_t *shapefile, FILE *f, shapefile_header_t *header) {
    bool success;

    success = shapefile_read_int32_be(shapefile,      f, &header->code,        NULL) &&
              shapefile_read_int32_be(shapefile,      f, &header->unused[0],   NULL) &&
              shapefile_read_int32_be(shapefile,      f, &header->unused[1],   NULL) &&
              shapefile_read_int32_be(shapefile,      f, &header->unused[2],   NULL) &&
              shapefile_read_int32_be(shapefile,      f, &header->unused[3],   NULL) &&
              shapefile_read_int32_be(shapefile,      f, &header->unused[4],   NULL) &&
              shapefile_read_int32_size_be(shapefile, f, &header->length,      NULL) &&
              shapefile_read_int32_le(shapefile,      f, &header->version,     NULL) &&
              shapefile_read_int32_le(shapefile,      f, &header->type,        NULL) &&
              shapefile_read_double_le(shapefile,     f, &header->mbr.min_x,   NULL) &&
              shapefile_read_double_le(shapefile,     f, &header->mbr.max_x,   NULL) &&
              shapefile_read_double_le(shapefile,     f, &header->mbr.min_y,   NULL) &&
              shapefile_read_double_le(shapefile,     f, &header->mbr.max_y,   NULL) &&
              shapefile_read_double_le(shapefile,     f, &header->range.z.min, NULL) &&
              shapefile_read_double_le(shapefile,     f, &header->range.z.max, NULL) &&
              shapefile_read_double_le(shapefile,     f, &header->range.m.min, NULL) &&
              shapefile_read_double_le(shapefile,     f, &header->range.m.max, NULL);

    if (success) {
        if (header->code != SHAPEFILE_HEADER_MAGIC) {
            snprintf(shapefile->error, sizeof(shapefile->error), "Header magic number %08x is invalid", header->code);
            success = false;
        }
    }

    if (success) {
        if (!shapefile_type_valid(header->type)) {
            snprintf(shapefile->error, sizeof(shapefile->error), "Header Type %d is invalid", header->type);
            success = false;
        }
    }

    if (success) {
        if (header->length < SHAPEFILE_HEADER_SIZE) {
            snprintf(shapefile->error, sizeof(shapefile->error), "Size in header %d cannot be less than header size %lu", header->length, SHAPEFILE_HEADER_SIZE);
            success = false;
        }
    }

    return success;
}

static bool
shapefile_read_shp_record_header(shapefile_t *shapefile, FILE *f, shapefile_shp_record_header_t *header, int32_t *length) {
    return shapefile_read_int32_be(shapefile,      shapefile->shp.f, &header->number, length) &&
           shapefile_read_int32_size_be(shapefile, shapefile->shp.f, &header->length, length);
}

static bool
shapefile_read_shp_record_null(shapefile_t *shapefile, FILE *f, shapefile_shp_record_header_t *record_header, shapefile_shp_record_t *record, int32_t *length) {
    return true;
}

static bool
shapefile_read_shp_record_point(shapefile_t *shapefile, FILE *f, shapefile_shp_record_header_t *record_header, shapefile_shp_record_t *record, int32_t *length) {
    shapefile_shape_point_t *point;

    point = malloc(sizeof(*point));
    if (point == NULL) {
        strlcpy(shapefile->error, "Out of memory", sizeof(shapefile->error));
        return false;
    }

    record->shape->geometry = point;

    return shapefile_read_double_le(shapefile, shapefile->shp.f, &point->x, length) &&
           shapefile_read_double_le(shapefile, shapefile->shp.f, &point->y, length);
}

static bool
shapefile_read_shp_record(shapefile_t *shapefile, FILE *f, shapefile_shp_record_header_t *record_header, shapefile_shp_record_t *record, int32_t *length) {
    bool success = false;

    if (!shapefile_read_int32_le(shapefile, shapefile->shp.f, &record->type, length)) {
        return false;
    }

    if (!shapefile_type_valid(record->type)) {
        snprintf(shapefile->error, sizeof(shapefile->error), "Shape type %d in record %d is not valid", record->type, record_header->number);
        return false;
    }

    record->shape = shapefile_shape_new(record->type);

    switch (record->type) {
        case SHAPEFILE_TYPE_NULL:
            success = shapefile_read_shp_record_null(shapefile, shapefile->shp.f, record_header, record, length);
            break;
        case SHAPEFILE_TYPE_POINT:
            success = shapefile_read_shp_record_point(shapefile, shapefile->shp.f, record_header, record, length);
            break;
        case SHAPEFILE_TYPE_POLYLINE:
        case SHAPEFILE_TYPE_POLYGON:
        case SHAPEFILE_TYPE_MULTIPOINT:
        case SHAPEFILE_TYPE_POINT_Z:
        case SHAPEFILE_TYPE_POLYLINE_Z:
        case SHAPEFILE_TYPE_POLYGON_Z:
        case SHAPEFILE_TYPE_MULTIPOINT_Z:
        case SHAPEFILE_TYPE_POINT_M:
        case SHAPEFILE_TYPE_POLYLINE_M:
        case SHAPEFILE_TYPE_POLYGON_M:
        case SHAPEFILE_TYPE_MULTIPOINT_M:
        case SHAPEFILE_TYPE_MULTIPATCH:
            snprintf(shapefile->error, sizeof(shapefile->error), "Shape type %d (%s) in record %d is not supported", record->type, shapefile_type_str(record->type), record_header->number);
            success = false;
            break;
    }

    if (!success) {
        shapefile_shape_free(record->shape);
    }

    return success;
}

static bool
shapefile_parse_shx(shapefile_t *shapefile, const char *path_prefix) {
    shapefile_header_t header;
    bool success = false;
    char *path;
    int len;

    len = asprintf(&path, "%s.shx", path_prefix);
    if (len == -1) {
        strlcpy(shapefile->error, "Out of memory", sizeof(shapefile->error));
        return false;
    }

    shapefile->shx.f = fopen(path, "r");
    if (shapefile->shx.f == NULL) {
        snprintf(shapefile->error, sizeof(shapefile->error), "Error opening %s: %s", path, strerror(errno));
        goto done;
    }

    //read the shapefile header, just because. we don't need to store it though because it's the same exact header
    //in the .shp file. we're store that one
    if (!shapefile_read_header(shapefile, shapefile->shx.f, &header)) {
        goto done;
    }

    success = true;

done:
    if (!success) {
        if (shapefile->shx.f != NULL) {
            fclose(shapefile->shx.f);
            shapefile->shx.f = NULL;
        }
    }

    free(path);

    return success;
}

static bool
shapefile_parse_shp(shapefile_t *shapefile, const char *path_prefix, bool *stop, shapefile_parse_cb_t *cb) {
    shapefile_shp_record_header_t record_header;
    shapefile_shp_record_t record;
    int32_t length;
    bool success = true;
    char *path;
    int len;

    len = asprintf(&path, "%s.shp", path_prefix);
    if (len == -1) {
        strlcpy(shapefile->error, "Out of memory", sizeof(shapefile->error));
        return false;
    }

    shapefile->shp.f = fopen(path, "r");
    if (shapefile->shp.f == NULL) {
        snprintf(shapefile->error, sizeof(shapefile->error), "Error opening %s: %s", path, strerror(errno));
        success = false;
    }

    if (success) {
        success = shapefile_read_header(shapefile, shapefile->shp.f, &shapefile->shp.header);
    }

    if (success) {
        length = shapefile->shp.header.length - SHAPEFILE_HEADER_SIZE;

        while (success && !*stop && length > 0) {
            success = shapefile_read_shp_record_header(shapefile, shapefile->shp.f, &record_header, &length);
            if (!success) {
                break;
            }

            success = shapefile_read_shp_record(shapefile, shapefile->shp.f, &record_header, &record, &length);
            if (!success) {
                //if we fail, the record->shape doesn't need to be free'd
                break;
            }

            if (cb != NULL) {
                //call the callback, which returns true on success, and false to stop
                *stop = !cb->shape(record.shape, cb->user_data);

                shapefile_shape_free(record.shape);
            }
            else {
                //adding to a list?
            }
        }
    }

    if (!success) {
        if (shapefile->shx.f != NULL) {
            fclose(shapefile->shx.f);
            shapefile->shx.f = NULL;
        }
    }

    free(path);

    return success;
}

shapefile_t *
shapefile_init() {
    shapefile_t *shapefile;

    shapefile = calloc(1, sizeof(*shapefile));

    return shapefile;
}

void
shapefile_free(shapefile_t *shapefile) {
    free(shapefile);
}

bool
shapefile_parse_cb(shapefile_t *shapefile, const char *path, shapefile_parse_cb_t *cb) {
    char *path_prefix, *ptr;
    bool success, stop;

    //if <path>/<file>.shp is passed in, strip the .shp so we can get the base name
    path_prefix = strdup(path);
    ptr = strrchr(path_prefix, '.');
    if (ptr != NULL) {
        *ptr = '\0';
    }

    stop = false;

    success = shapefile_parse_shx(shapefile, path_prefix) &&
              shapefile_parse_shp(shapefile, path_prefix, &stop, cb);

    free(path_prefix);
    return success;
}


const char *
shapefile_error(shapefile_t *shapefile) {
    return shapefile->error;
}