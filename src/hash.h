#pragma once

/**
 * @file hash.h
 * @author Scott Newman
 *
 * @brief A simple hash table implementation using either DJB2 or SDBM hashing
 * functions.
 *
 * This hash table uses a very basic hash table implementation using either
 * the DJB2 or SDBM hashing function. The hashing function can only be set at
 * compile time using the <tt>HASH_FUNC</tt> pre-processor definition.
 *
 * The keys in the hash table are strings but are transformed into an integer
 * called a hash code using the hashing function defined. Each corresponding
 * data item of a hash code is a linked list which contains the actual user
 * data. This linked list handles collisions (eg. 2 or more keys produce the
 * same hash code) so when they do occur, the user data is simply appended to
 * the end of the linked list.
 *
 * Keys should be unique. If duplicate keys exist, a collision will occur and
 * the user data will be added to the linked list of items. However, any
 * attempt to retrieve that user data will result in only the first user data
 * item being returned.
 *
 * @see http://www.cse.yorku.ca/~oz/hash.html
 *
 * A hash table will look like this. Let's assume there are only four buckets.
 * Because there are only four buckets, collisions will happen a lot. In the
 * example below, bucket 4 has three collisions. It just so happened that the
 * keys of items 1, 4, and 5 produced the same hash code of 3, so those three
 * items were placed in bucket 4. When searching for item 5, the key will get
 * hashed to produce the hash code 3 and then a linear comparison of every key
 * in bucket 4 will be done until a match is found.
 * @verbatim
                  |----------|-----hash---|-----------|
                  |          |            |           |
               bucket1[0]  bucket2[1]  bucket3[2]  bucket4[3]
                  |                       |           |
                Item3                   Item2       Item1
                                                    Item4
                                                    Item5
 @endverbatim
 *
 * <b>Basic usage:</b>
 * @include hash.c
 */

#include <stdio.h>
#include <stdbool.h>
#include "alist.h"

#define HASH_DJB2 1         //!< Hash function DJBM2
#define HASH_SDBM 2         //!< Hash function SDBM
#define HASH_FUNC HASH_DJB2 //!< Which hash function to use

/** 
 * @brief The hash structure.
 *
 * This structure represnts the hash table. Members of this structure should
 * not be accessed or, more importantly, changed directly.
 */
typedef struct {
    alist_t *buckets;       //!< The list of buckets for the hash.
    unsigned int size;      //!< The current number of items in the hash.
    unsigned int capacity;  //!< The maximum capacity of buckets.
} hash_t;

/**
 * @brief Initializes a hash table.
 *
 * This function must be called before any other hash function is used. If
 * another hash function is called before this, undefined behavior will occur
 * and it's very likely your program will crash.
 *
 * @param[in] hash The hash.
 */
void hash_init(hash_t *hash);

/**
 * @brief Initializes a hash table with the given capacity.
 *
 * This function must be called before any other hash function is used. If
 * another hash function is called before this, undefined behavior will occur
 * and it's very likely your program will crash.
 *
 * @param[in] hash The hash.
 * @param[in] capacity The initial capacity.
 * @return <tt>true</tt>, otherwise <tt>false</tt> if not enough memory is available.
 */
bool hash_init2(hash_t *hash, unsigned int capacity);

/**
 * @brief Frees internal memory used by the hash and reduces the hash size to
 * 0.
 *
 * This function must be called after the hash is done being used. If not,
 * memory leaks will occur. If this function is called and the hash is to be
 * re-used, hash_init() must be called again before using the hash.
 *
 * Calling this function will not automatically free the user data.
 * That is up to the developer. To have the hash automatically free user data,
 * see hash_free_func().
 *
 * @param[in] hash The hash.
 */
void hash_free(hash_t *hash);

/**
 * @brief Fees internal memory used by the hash, calls <tt>free_func</tt> once
 * per item in the hash, and reduces the hash size to 0.
 *
 * This function must be called after the hash is done being used. If not,
 * memory leaks will occur. If this function is called and the hash is to be
 * re-used, hash_init() must be called again before using the hash.
 *
 * For each item stored in the hash, <tt>free_func</tt> is called which is a
 * function that should free the user data item. For example, if the user data
 * item is an integer allocated on the heap, free() is sufficient to used.
 *
 * @param[in] hash The hash.
 * @param[in] free_func The function to call on each item in the hash to free
 * its memory.
 */
void hash_free_func(hash_t *hash, void (*free_func)(void *));

/**
 * @brief Returns the size of the hash.
 *
 * Returns how many items are in the hash. This number will be different than
 * the number of buckets in the hash if collisions have occured.
 *
 * @param[in] hash The hash.
 * @return The number of items in the hash.
 */
unsigned int hash_size(hash_t *hash);

/**
 * @brief Adds user data to the hash given a key.
 *
 * Puts the user data <tt>data</tt> into the hash given key <tt>key</tt>. The
 * keys within the hash must be unique or else attempting to retrieve the user
 * data will result in only the first user data item every being returned.
 *
 * @param[in] hash The hash.
 * @param[in] key  The key used to identify the user data.
 * @param[in] data The user data.
 * @return <tt>true</tt> on success, otherwise <tt>false</tt> if memory cannot
 * be allocated.
 */
bool hash_set(hash_t *hash, const char *key, void *data);

/**
 * @brief Determines if the key exists in the hash.
 *
 * Returns true if the key exists in the hash, otherwise false.
 *
 * @param[in] hash The hash.
 * @param[in] key The key to search for.
 * @return <tt>true</tt> if the key exists, otherwise false.
 */
bool hash_contains(hash_t *hash, const char *key);

/**
 * @brief Gets user data from the hash.
 *
 * Returns the user data associated with <tt>key</tt>. If the key does not
 * exist in the hash then <tt>NULL</tt> is returned.
 *
 * @param[in] hash The hash.
 * @param[in] key The key used to identify the user data.
 * @return The user data associated with <tt>key</tt>, otherwise <tt>NULL</tt>
 * if the key doesnt not exist.
 */
void * hash_get(hash_t *hash, const char *key);

/**
 * @brief Delete a key from the hash.
 *
 * Deletes a key from the hash and reduces the hash size by 1. The user data
 * is returned so that the developer may do something else with the memory.
 * That means if the user data is allocated on the heap and not free'd by the
 * developer, then a memory leak will occur. See hash_delete_func() if you wish
 * to free user data with the delete operation. <tt>NULL</tt> is returned if
 * the key is not found.
 *
 * @param[in] hash The hash.
 * @param[in] key  The key to delete.
 * @return The user data, otherwise <tt>NULL</tt> if the key was not found.
 */
void * hash_delete(hash_t *hash, const char *key);

/**
 * @brief Deletes a key from the hash and also calls <tt>free_func</tt> for the
 * user data item.
 *
 * Deletes a key from the hash and reduces the hash size by 1. If found,
 * <tt>free_func</tt> is called on the user data item which should free the memory
 * for the user data.
 *
 * @param[in] hash      The hash.
 * @param[in] key       The key to delete.
 * @param[in] free_func The function called the free the user data.
 * @return <tt>true</tt> if the key was found, otherwise <tt>false</tt>.
 */
bool hash_delete_func(hash_t *hash, const char *key, void (*free_func)(void *));

/**
 * @brief Iterates over each item in the hash and calls a function.
 *
 * Iterates over each item in the hash and calls <tt>iterate_func</tt>. The
 * params to <tt>iterate_func</tt> params are as follows:
 *     <tt>iterate_func(key, item, additional user data)</tt>
 *
 * Return <tt>false</tt> to stop iterating, otherwise return <tt>true</tt>.
 *
 * @param[in] hash         The hash.
 * @param[in] iterate_func The function to be called on each item.
 * @param[in] user_data    Additional user data to pass along to <tt>iterate_func</tt>.
 * @return <tt>true</tt> if the iteration completely finished, otherwise <tt>false</tt>.
 */
bool hash_foreach(hash_t *hash, bool (*iterate_func)(const char *, void *, void *), void *user_data);

/**
 * @brief Prints the hash to the file stream identified by <tt>f</tt>.
 *
 * Iterates over the each item in the hash and prints the key to
 * the file stream identified by <tt>f</tt>. The hash's size and max capacity
 * is also printed to <tt>f</tt>.
 *
 * @param[in] hash The hash.
 * @param[in] f    The file stream.
 */
void hash_print(hash_t *hash, FILE *f);
