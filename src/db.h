#pragma once

#include <stdbool.h>

typedef struct db_t db_t;
typedef struct db_result_t db_result_t;

db_t * db_init();
void db_free(db_t *db);

const char * db_error(db_t *db);

bool db_connect(db_t *db, const char *host, const char *user, const char *password, const char *database, unsigned int port);
void db_disconnect(db_t *db);

bool db_query(db_t *db, const char *query, unsigned int len);
bool db_queryf(db_t *db, const char *fmt, ...);

db_result_t * db_select(db_t *db, const char *query, unsigned int len);
db_result_t * db_selectf(db_t *db, const char *fmt, ...);

char * db_escape(db_t *db, const char *str);

void db_result_free(db_result_t *result);

bool db_result_next(db_result_t *result);

const char * db_result_str(db_result_t *result, unsigned int index);
