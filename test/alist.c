#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "../src/scott.h"
#include "test.h"
#include "alist.h"

#define MODULE "alist"

typedef struct {
    alist_t *list;
    char **items;
    unsigned int size;
} alist_test_t;

static bool
alist_test_create(alist_test_t *data, unsigned int size) {
    unsigned int i;

    memset(data, 0, sizeof(*data));

    data->list = alist_init();
    data->items = calloc(size, sizeof(char *));
    data->size = size;

    for (i = 0; i < size; i++) {
        asprintf(&data->items[i], "Item %d", i);
    }

    for (i = 0; i < size; i++) {
        alist_add(data->list, data->items[i]);
    }

    if (alist_size(data->list) != size) {
        test_printf(MODULE, "Expected list size %u, but got %u", size, alist_size(data->list));
        return false;
    }

    return true;
}

static void
alist_test_free(alist_test_t *data) {
    unsigned int i;

    if (data->list != NULL) {
        alist_free(data->list);
    }

    if (data->items != NULL) {
        for (i = 0; i < data->size; i++) {
            if (data->items[i] != NULL) {
                free(data->items[i]);
            }
        }

        free(data->items);
    }
}

static int
alist_test_add(unsigned int size) {
    bool success;
    const char *item;
    unsigned int i;
    alist_test_t data;

    success = alist_test_create(&data, size);

    for (i = 0; success && i < data.size; i++) {
        item = alist_get(data.list, i);

        if (strcmp(item, data.items[i]) != 0) {
            test_printf(MODULE, "Expected '%s' at index %d, but got '%s'", data.items[i], i, item);
            success = false;
        }
    }

    alist_test_free(&data);

    return success ? 0 : 1;
}

static int
alist_test_add_small(void *user_data) {
    return alist_test_add(10);
}

static int
alist_test_add_big(void *user_data) {
    return alist_test_add(100000);
}

static int
alist_remove_all(int size) {
    bool success;
    const char *item;
    unsigned int i;
    alist_test_t data;

    success = alist_test_create(&data, size);
    for (i = 0; success && i < data.size; i++) {
        item = alist_remove(data.list, 0);

        if (strcmp(item, data.items[i]) != 0) {
            test_printf(MODULE, "Expected '%s' at index %d, but got '%s'", data.items[i], i, item);
            success = false;

        }
    }

    if (success) {
        if (alist_size(data.list) != 0) {
            test_printf(MODULE, "Expected array size 0, but got %u", alist_size(data.list));
            success = false;
        }
    }

    alist_test_free(&data);

    return success ? 0 : 1;
}

static int
alist_remove_all_small(void *user_data) {
    return alist_remove_all(10);
}

static int
alist_remove_all_big(void *user_data) {
    return alist_remove_all(100000);
}

int
alist_test() {
    int count;

    count = test_run(MODULE, 1, "Add 10 Items", alist_test_add_small, NULL) + 
            test_run(MODULE, 2, "Add 100000000 Items", alist_test_add_big, NULL) +
            test_run(MODULE, 3, "Add 10 Items and Remove Them All", alist_remove_all_small, NULL) + 
            test_run(MODULE, 4, "Add 1000000 Items and Remove Them All", alist_remove_all_big, NULL);

    return count;
}
