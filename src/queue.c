/**
 * @file queue.c
 */

#include <stdlib.h>
#include <string.h>
#include "queue.h"

void
queue_init(queue_t *queue) {
    memset(queue, 0, sizeof(*queue));
}

void
queue_free(queue_t *queue) {
    queue_free_func(queue, NULL);
}

void
queue_free_func(queue_t *queue, void (*free_func)(void *)) {
    queue_node_t *node, *del;

    node = queue->head;
    while (node != NULL) {
        del = node;
        node = node->next;

        if (free_func != NULL) {
            free_func(del->data);
        }

        free(del);
    }

    memset(queue, 0, sizeof(*queue));
}

unsigned int
queue_size(queue_t *queue) {
    return queue->size;
}

bool
queue_push(queue_t *queue, void *data) {
    queue_node_t *node;

    node = malloc(sizeof(*node));
    if (node == NULL) {
        return false;
    }

    node->data = data;
    node->next = NULL;

    if (queue->head == NULL) {
        node->prev = NULL;
        queue->head = node;
        queue->tail = node;
    }
    else {
        node->prev = queue->tail;
        queue->tail->next = node;
        queue->tail = node;
    }

    ++queue->size;

    return true;
}

void *
queue_pop(queue_t *queue) {
    queue_node_t *node;
    void *data;

    if (queue->head == NULL) {
        return NULL;
    }

    node = queue->head;
    data = queue->head->data;

    if (node->next == NULL) {
        queue->head = NULL;
        queue->tail = NULL;
    }
    else {
        queue->head = node->next;
        node->prev = NULL;
    }

    free(node);
    --queue->size;

    return data;
}

void *
queue_peek(queue_t *queue) {
    return queue->head == NULL ? NULL : queue->head->data;
}

void *
queue_peek_back(queue_t *queue) {
    return queue->tail == NULL ? NULL : queue->tail->data;
}
