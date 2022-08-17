#include <stdlib.h>
#include <string.h>
#if defined(_WIN32)
# include <Windows.h>
#else
# include <sys/mman.h>
#endif
#include "buffer.h"

#define BUFFER_FLAGS_SECURE      0x01
#define BUFFER_FLAGS_FREE_MEMORY 0x02

struct buffer_t {
    unsigned char *data;
    size_t capacity;
    size_t len;
    int flags;
};

static bool
buffer_security_add(unsigned char *data, size_t len) {
#if defined(_WIN32)
    if (!VirtualLock(data, len)) {
        return false;
    }
#else
    if (mlock(data, len) != 0) {
        return false;
    }
#endif

    return true;
}

static void
buffer_security_remove(unsigned char *data, size_t len) {
#if defined(_WIN32)
    VirtualUnlock(data, len);
#else
    munlock(data, len);
#endif
}

buffer_t *
buffer_init() {
    return buffer_init_ex(0);
}

buffer_t *
buffer_init_ex(size_t capacity) {
    buffer_t *buffer;

    buffer = calloc(1, sizeof(*buffer));
    if (buffer == NULL) {
        return NULL;
    }

    if (capacity > 0) {
        buffer->data = malloc(capacity);
        if (buffer->data == NULL) {
            free(buffer);
            return false;
        }

        buffer->capacity = capacity;
    }

    return buffer;

}

void
buffer_free(buffer_t *buffer) {
    if (buffer == NULL) {
        return;
    }

    if (buffer->data != NULL) {
        if (buffer->flags & BUFFER_FLAGS_SECURE) {
            memset(buffer->data, 0, buffer->capacity);
            buffer_security_remove(buffer->data, buffer->capacity);
        }

        free(buffer->data);
    }

    free(buffer);
}

static void
buffer_flag_set(buffer_t *buffer, int flag, bool value) {
    if (value) {
        buffer->flags |= flag;
    }
    else {
        buffer->flags &= ~flag;
    }
}

bool
buffer_set_secure(buffer_t *buffer, bool value) {
    if (buffer->data != NULL) {
        if (value) {
            if (!buffer_security_add(buffer->data, buffer->capacity)) {
                return false;
            }
        }
        else {
            buffer_security_remove(buffer->data, buffer->capacity);
        }
    }

    buffer_flag_set(buffer, BUFFER_FLAGS_SECURE, value);

    return true;
}

void
buffer_set_free_memory(buffer_t *buffer, bool value) {
    buffer_flag_set(buffer, BUFFER_FLAGS_FREE_MEMORY, value);
}

size_t
buffer_length(buffer_t *buffer) {
    return buffer->len;
}

static bool
buffer_grow_secure(buffer_t *buffer, size_t new_capacity) {
    unsigned char *new_data;

    new_data = malloc(new_capacity);
    if (new_data == NULL) {
        return false;
    }

    if (!buffer_security_add(new_data, new_capacity)) {
        free(new_data);
        return false;
    }

    memcpy(new_data, buffer->data, buffer->len);

    memset(buffer->data, 0, buffer->capacity);
    buffer_security_remove(buffer->data, buffer->capacity);
    free(buffer->data);

    buffer->data = new_data;
    buffer->capacity = new_capacity;

    return true;
}

static bool
buffer_grow_insecure(buffer_t *buffer, size_t new_capacity) {
    unsigned char *new_data;

    new_data = realloc(buffer->data, new_capacity);
    if (new_data == NULL) {
        return false;
    }

    buffer->data = new_data;
    buffer->capacity = new_capacity;

    return true;

}

static bool
buffer_grow(buffer_t *buffer, size_t len) {
    size_t new_capacity;
    bool success;

    if (buffer->capacity == 0) {
        new_capacity = len * 4;
    }
    else {
        new_capacity = (buffer->capacity * 2) + (len * 2);
    }

    if (buffer->flags & BUFFER_FLAGS_SECURE) {
        success = buffer_grow_secure(buffer, new_capacity);
    }
    else {
        success = buffer_grow_insecure(buffer, new_capacity);
    }

    return success;
}

bool
buffer_write(buffer_t *buffer, unsigned char *data, size_t len) {
    if (buffer->len + len > buffer->capacity) {
        if (!buffer_grow(buffer, len)) {
            return false;
        }
    }

    memcpy(buffer->data + buffer->len, data, len);
    buffer->len += len;

    return true;
}

bool
buffer_write_uint8(buffer_t *buffer, uint8_t data) {
    return buffer_write(buffer, (unsigned char *)&data, sizeof(data));
}

bool
buffer_write_uint16(buffer_t *buffer, uint16_t data) {
    return buffer_write(buffer, (unsigned char *)&data, sizeof(data));
}

bool
buffer_write_uint32(buffer_t *buffer, uint32_t data) {
    return buffer_write(buffer, (unsigned char *)&data, sizeof(data));
}

bool
buffer_write_uint64(buffer_t *buffer, uint64_t data) {
    return buffer_write(buffer, (unsigned char *)&data, sizeof(data));
}

bool
buffer_write_int8(buffer_t *buffer, int8_t data) {
    return buffer_write(buffer, (unsigned char *)&data, sizeof(data));
}

bool
buffer_write_int16(buffer_t *buffer, int16_t data) {
    return buffer_write(buffer, (unsigned char *)&data, sizeof(data));
}

bool
buffer_write_int32(buffer_t *buffer, int32_t data) {
    return buffer_write(buffer, (unsigned char *)&data, sizeof(data));
}

bool
buffer_write_int64(buffer_t *buffer, int64_t data) {
    return buffer_write(buffer,(unsigned char *) &data, sizeof(data));
}

bool
buffer_write_char(buffer_t *buffer, char data) {
    return buffer_write(buffer, (unsigned char *)&data, sizeof(data));
}

bool
buffer_write_float(buffer_t *buffer, float data) {
    return buffer_write(buffer, (unsigned char *)&data, sizeof(data));
}

bool
buffer_write_double(buffer_t *buffer, double data) {
    return buffer_write(buffer, (unsigned char *)&data, sizeof(data));
}

const unsigned char *
buffer_data(buffer_t *buffer) {
    return buffer->data;
}

size_t
buffer_remove(buffer_t *buffer, size_t len) {
    if (len > buffer->len) {
        len = buffer->len;
    }

    if (len > 0) {
        memmove(buffer->data, buffer->data + len, len);
        buffer->len -= len;

        //zero out the remaining memory if this is a secure buffer
        if (buffer->flags & BUFFER_FLAGS_SECURE) {
            memset(buffer->data + len, 0, len);
        }
    }

    return len;
}

void
buffer_clear(buffer_t *buffer) {
    if (buffer->data == NULL) {
        return;
    }

    //zero out the memory if this is a secure buffer
    if (buffer->flags & BUFFER_FLAGS_SECURE) {
        memset(buffer->data, 0, buffer->capacity);
    }

    if (buffer->flags & BUFFER_FLAGS_FREE_MEMORY) {
        if (buffer->flags & BUFFER_FLAGS_SECURE) {
            buffer_security_remove(buffer->data, buffer->capacity);
        }

        free(buffer->data);
        buffer->data = NULL;
        buffer->capacity = 0;
    }

    buffer->len = 0;
}
