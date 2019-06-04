#pragma once

typedef struct lock_t lock_t;

lock_t * lock_init();
void lock_free(lock_t *lock);

void lock_read_lock(lock_t *lock);
void lock_read_unlock(lock_t *lock);

void lock_write_lock(lock_t *lock);
void lock_write_unlock(lock_t *lock);
