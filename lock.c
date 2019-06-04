#include <stdlib.h>
#include <string.h>
#if defined(_WIN32)
# include <Windows.h>
#else
# include <pthread.h>
#endif
#include "lock.h"

struct lock_t {
#if defined(_WIN32)
    SRWLOCK lock;
#else
    pthread_rwlock_t lock;
#endif
};

lock_t *
lock_init() {
    lock_t *lock;

    lock = calloc(1, sizeof(*lock));
    if (lock == NULL) {
        return NULL;
    }

#if defined(_WIN32)
    InitializeSRWLock(&lock->lock);
#else
    pthread_rwlock_init(&lock->lock, NULL);
#endif

    return lock;
}

void
lock_free(lock_t *lock) {
    if (lock == NULL) {
        return;
    }

#if defined(_WIN32)
#else
    pthread_rwlock_destroy(&lock->lock);
#endif

    free(lock);
}

void
lock_read_lock(lock_t *lock) {
#if defined(_WIN32)
    AcquireSRWLockShared(&lock->lock);
#else
    pthread_rwlock_wrlock(&lock->lock);
#endif
}

void
lock_read_unlock(lock_t *lock) {
#if defined(_WIN32)
    ReleaseSRWLockShared(&lock->lock);
#else
    pthread_rwlock_unlock(&lock->lock);
#endif
}

void
lock_write_lock(lock_t *lock) {
#if defined(_WIN32)
    AcquireSRWLockExclusive(&lock->lock);
#else
    pthread_rwlock_rdlock(&lock->lock);
#endif
}

void
lock_write_unlock(lock_t *lock) {
#if defined(_WIN32)
    ReleaseSRWLockExclusive(&lock->lock);
#else
    pthread_rwlock_unlock(&lock->lock);
#endif
}
