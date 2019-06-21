/**
 * @file hash.c
 */

#include <stdlib.h>
#include "string.h"
#include "hash.h"

/**
 * @brief The structure that represents each item in the hash.
 *
 * This structure represnts each item in the hash. The key is stored along
 * with it because if there is a collision (eg. more than 1 item exists for a
 * hash code), then we'll need to loop through the list and compare keys.
 */
typedef struct {
    char *key;  /**< The key for the item, used in linear comparison. */
    void *data; /**< The user data for this item. */
} hash_item_t;

/**
 * @brief The hash code function.
 *
 * This is the hash code function which turns a string based key into a
 * numeric key. Depending on what <tt>HASH_FUNC</tt> is set to,
 * different hash functions can be used.
 *
 * @param[in] hash The hash.
 * @param[in] key  The key to generate a hash code from.
 * @return The hash code.
 */
static unsigned int
hash_code(hash_t *hash, const char *key) {
#if HASH_FUNC == HASH_DJB2
    unsigned int c, code;

    code = 5381;
    while ((c = *key++) != '\0')
        code = ((code << 5) + code) + c;

    return code % hash->capacity;
#elif HASH_FUNC == HASH_SDBM
    unsigned int c, code;

    code = 0;
    while ((c = *key++) != '\0')
        code = c + (code << 6) + (code << 16) - code;

    return code % hash->capacity;
#else
# error "No hash function defined"
#endif
}

static bool
hash_create(hash_t *hash, unsigned int capacity) {
    unsigned int i;

    hash->capacity = capacity;
    hash->buckets = malloc(sizeof(alist_t) * hash->capacity);
    if (hash->buckets == NULL) {
        return false;
    }

    for (i = 0; i < hash->capacity; i++) {
        alist_init(&hash->buckets[i]);
    }

    return true;
}

static bool
hash_rehash_func(const char *key, void *data, void *user_data) {
    hash_t *tmp;

    tmp = (hash_t *)user_data;

    if (!hash_set(tmp, key, data)) {
        return false;
    }

    return true;
}

static bool
hash_rehash(hash_t *hash) {
    hash_t tmp;

    if (!hash_init2(&tmp, hash->capacity * 2)) {
        return false;
    }

    if (!hash_foreach(hash, hash_rehash_func, &tmp)) {
        hash_free(&tmp);
        return false;
    }

    hash_free(hash);
    hash->buckets = tmp.buckets;
    hash->size = tmp.size;
    hash->capacity = tmp.capacity;

    return true;
}
    
void
hash_init(hash_t *hash) {
    hash->buckets = NULL;
    hash->size = 0;
    hash->capacity = 0;
}

bool
hash_init2(hash_t *hash, unsigned int capacity) {
    hash->buckets = NULL;
    hash->size = 0;

    return hash_create(hash, capacity);
}

void
hash_free(hash_t *hash) {
    hash_item_t *item;
    unsigned int i;

    if (hash->capacity > 0) {
        for (i = 0; i < hash->capacity; i++) {
            while (alist_size(&hash->buckets[i]) > 0) {
                item = alist_remove(&hash->buckets[i], 0);
                free(item->key);
                free(item);
            }

            alist_free(&hash->buckets[i]);
        }

        free(hash->buckets);
    }
}

void
hash_free_func(hash_t *hash, void (*free_func)(void *)) {
    hash_item_t *item;
    unsigned int i;

    if (hash->capacity > 0) {
        for (i = 0; i < hash->capacity; i++) {
            while (alist_size(&hash->buckets[i]) > 0) {
                item = alist_remove(&hash->buckets[i], 0);
                free_func(item->data);
                free(item->key);
                free(item);
            }

            alist_free(&hash->buckets[i]);
        }

        free(hash->buckets);
    }
}

unsigned int
hash_size(hash_t *hash) {
    return hash->size;
}

bool
hash_set(hash_t *hash, const char *key, void *data) {
    unsigned int code;
    hash_item_t *item;

    if (hash->capacity == 0) {
        if (!hash_create(hash, 512)) {
            return false;
        }
    }
    else if ((double)hash->size / (double)hash->capacity >= 0.50) {
        if (!hash_rehash(hash)) {
            return false;
        }
    }

    item = malloc(sizeof(*item));
    if (item == NULL) {
        return false;
    }

    item->key = strdup(key);
    if (item->key == NULL) {
        free(item);
        return false;
    }

    item->data = data;
    code = hash_code(hash, key);

    if (!alist_add(&hash->buckets[code], item)) {
        free(item->key);
        free(item);
        return false;
    }

    ++hash->size;

    return true;
}

bool
hash_contains(hash_t *hash, const char *key) {
    return hash_get(hash, key) != NULL;
}

void *
hash_get(hash_t *hash, const char *key) {
    unsigned int i, code;
    hash_item_t *item;

    if (hash->capacity == 0)
        return NULL;

    code = hash_code(hash, key);

    for (i = 0; i < alist_size(&hash->buckets[code]); i++) {
        item = (hash_item_t *)alist_get(&hash->buckets[code], i);

        if (strcmp(item->key, key) == 0)
            return item->data;
    }

    return NULL;
}

void *
hash_delete(hash_t *hash, const char *key) {
    unsigned int i, code;
    void *data;
    hash_item_t *item;

    code = hash_code(hash, key);

    for (i = 0; i < alist_size(&hash->buckets[code]); i++) {
        item = (hash_item_t *)alist_get(&hash->buckets[code], i);

        if (strcmp(item->key, key) == 0) {
            data = item->data;
            alist_remove(&hash->buckets[code], i);
            free(item->key);
            free(item);
            --hash->size;
            return data;
        }
    }

    return NULL;
}

bool
hash_delete_func(hash_t *hash, const char *key, void (*free_func)(void *)) {
    void *data;

    data = hash_delete(hash, key);
    if (data == NULL)
        return false;

    free_func(data);
    return true;
}

bool
hash_foreach(hash_t *hash, bool (*iterate_func)(const char *, void *, void *), void *user_data) {
    unsigned int i, j;
    hash_item_t *item;

    for (i = 0; i < hash->capacity; i++) {
        for (j = 0; j < alist_size(&hash->buckets[i]); j++) {
            item = (hash_item_t *)alist_get(&hash->buckets[i], j);

            if (!iterate_func(item->key, item->data, user_data)) {
                return false;
            }
        }
    }

    return true;
}

void
hash_print(hash_t *hash, FILE *f) {
    unsigned int i, j;
    hash_item_t *item;

    fprintf(f, "Hash size: %u\n", hash->size);
    fprintf(f, "Hash capacity: %u\n", hash->capacity);

    for (i = 0; i < hash->capacity; i++) {
        if (alist_size(&hash->buckets[i]) > 0) {
            fprintf(f, "%u)", i);

            for (j = 0; j < alist_size(&hash->buckets[i]); j++) {
                item = (hash_item_t *)alist_get(&hash->buckets[i], j);

                fprintf(f, " \"%s\"", item->key);
            }

            fprintf(f, "\n");
        }
    }
}
