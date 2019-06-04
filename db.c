#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <mysql/mysql.h>
#include "lock.h"
#include "db.h"

struct db_t {
    MYSQL *mysql;
    lock_t *lock;
    char error[256];
};

struct db_result_t {
    MYSQL_RES *result;
    MYSQL_ROW row;
};

db_t *
db_init() {
    db_t *db;

    db = calloc(1, sizeof(*db));
    if (db == NULL) {
        return NULL;
    }

    db->lock = lock_init();
    if (db->lock == NULL) {
        free(db);
        return NULL;
    }

    return db;
}

void
db_free(db_t *db) {
    if (db == NULL) {
        return;
    }

    lock_write_lock(db->lock);
    if (db->mysql != NULL) {
        mysql_close(db->mysql);
    }
    lock_write_unlock(db->lock);

    lock_free(db->lock);

    free(db);
}

const char *
db_error(db_t *db) {
    return db->error;
}

bool
db_connect(db_t *db, const char *host, const char *user, const char *password, const char *database, unsigned int port) {
    bool success = true;

    lock_write_lock(db->lock);

    if (db->mysql != NULL) {
        mysql_close(db->mysql);
    }

    db->mysql = mysql_init(NULL);
    if (db->mysql == NULL) {
        snprintf(db->error, sizeof(db->error), "%s", "Out of memory");
        success = false;
    }

    if (success) {
        if (mysql_real_connect(db->mysql, host, user, password, database, port, NULL, 0) == NULL) {
            snprintf(db->error, sizeof(db->error), "%s", mysql_error(db->mysql));
            db_disconnect(db);
            success = false;
        }
    }

    lock_write_unlock(db->lock);

    return success;
}

void
db_disconnect(db_t *db) {
    lock_write_lock(db->lock);

    if (db->mysql != NULL) {
        mysql_close(db->mysql);
        db->mysql = NULL;
    }

    lock_write_unlock(db->lock);
}

bool
db_query(db_t *db, const char *query, unsigned int len) {
    int ret;

    lock_write_lock(db->lock);
    ret = mysql_real_query(db->mysql, query, (unsigned long)len);
    if (ret == -1) {
        snprintf(db->error, sizeof(db->error), "%s", mysql_error(db->mysql));
    }
    lock_write_unlock(db->lock);

    return ret == 0;

}

bool
db_queryf(db_t *db, const char *fmt, ...) {
    bool success;
    char *query;
    va_list ap;
    int len;

    va_start(ap, fmt);
    len = vasprintf(&query, fmt, ap);
    va_end(ap);

    if (len == -1) {
        snprintf(db->error, sizeof(db->error), "%s", "Out of memory");
        return false;
    }

    success = db_query(db, query, (unsigned int)len);
    free(query);

    return success;
}

db_result_t *
db_select(db_t *db, const char *query, unsigned int len) {
    db_result_t *result;
    int ret;

    result = calloc(1, sizeof(*result));
    if (result == NULL) {
        snprintf(db->error, sizeof(db->error), "%s", "Out of memory");
        return NULL;
    }

    lock_write_lock(db->lock);
    ret = mysql_real_query(db->mysql, query, (unsigned long)len);
    if (ret == -1) {
        snprintf(db->error, sizeof(db->error), "%s", mysql_error(db->mysql));
    }
    else {
        result->result = mysql_store_result(db->mysql);
        if (result->result == NULL) {
            snprintf(db->error, sizeof(db->error), "%s", mysql_error(db->mysql));
        }
    }
    lock_write_unlock(db->lock);

    if (result->result == NULL) {
        db_result_free(result);
        return NULL;
    }

    return result;
}

db_result_t *
db_selectf(db_t *db, const char *fmt, ...) {
    db_result_t *result;
    char *query;
    va_list ap;
    int len;

    va_start(ap, fmt);
    len = vasprintf(&query, fmt, ap);
    va_end(ap);

    if (len == -1) {
        snprintf(db->error, sizeof(db->error), "%s", "Out of memory");
        return NULL;
    }

    result = db_select(db, query, (unsigned int)len);
    free(query);

    return result;
}

char *
db_escape(db_t *db, const char *str) {
    size_t len;
    char *esc;

    len = strlen(str);
    esc = malloc(strlen(str) * 2 + 1);
    if (esc != NULL) {
        lock_write_lock(db->lock);
        mysql_real_escape_string(db->mysql, esc, str, len);
        lock_write_unlock(db->lock);
    }

    return esc;
}

void
db_result_free(db_result_t *result) {
    if (result->result != NULL) {
        mysql_free_result(result->result);
    }

    free(result);
}

bool
db_result_next(db_result_t *result) {
    result->row = mysql_fetch_row(result->result);

    return result->row != NULL;
}

const char *
db_result_str(db_result_t *result, unsigned int index) {
    return result->row[index];
}
