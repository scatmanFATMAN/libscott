#pragma once

#include <stddef.h>
#include <stdbool.h>

typedef struct buffer_t buffer_t;

buffer_t * buffer_init();
void buffer_free(buffer_t *buffer);

size_t buffer_length(buffer_t *buffer);

bool buffer_append(buffer_t *buffer, unsigned char *data, size_t len);
