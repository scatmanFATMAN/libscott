#pragma once

/**
 * @file buffer.c
 * @author scott.c
 *
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct buffer_t buffer_t;

/**
 * Allocates and initializes a buffer.
 *
 * @return A buffer, or <tt>NULL</tt> if not enough memory was available.
 */
buffer_t * buffer_init();

/**
 * Allocates and initializes a buffer with some extra flags.
 *
 * @param[in] capacity An initial capacity to allocate room for.
 * @return A buffer, or <tt>NULL</tt> if not enough memory was available.
 */
buffer_t * buffer_init_ex(size_t capacity);

/**
 * Deallocates a buffer which was allocated with buffer_init().
 *
 * @param[in] buffer The buffer.
 */
void buffer_free(buffer_t *buffer);

/**
 * Sets whether or not the buffer should be considered secure. A secure
 * buffer will always zero out memory that's not used and also lock memory so
 * that it cannot be swapped. If a buffer is set to secure, any data currently
 * in the buffer will be secured.
 *
 * @param[in] buffer The buffer.
 * @param[in] value <tt>true</tt> to turn on, or <tt>false</tt> to turn off.
 * @return <tt>true</tt> if the operation was successful, or <tt>false</tt> if
 * the memory couldn't be secured.
 */
bool buffer_set_secure(buffer_t *buffer, bool value);

/**
 * Sets whether or not the buffer should free memory there's unused memory. For
 * example, when off, a call to buffer_clear() will not free the memory and
 * set the buffer's length to 0. By default, this is off.
 *
 * @param[in] buffer The buffer.
 * @param[in] value <tt>true</tt> to turn on, or <tt>false</tt> to turn off.
 */
void buffer_set_free_memory(buffer_t *buffer, bool value);

/**
 * Returns the length of the buffer.
 *
 * @param[in] buffer The buffer.
 * @return The size of the buffer.
 */
size_t buffer_length(buffer_t *buffer);

/**
 * Writes <tt>len</tt> bytes of data from the pointer pointing to
 * <tt>data</tt> to the buffer.
 *
 * @param[in] buffer The buffer.
 * @param[in] data A pointer to data to write.
 * @param[in] len The number of bytes to write.
 * @return true if the write was successful, otherwise false if not enough
 * memory was available.
 */
bool buffer_write(buffer_t *buffer, unsigned char *data, size_t len);

/**
 * Writes an 8 bit unsigned integer to the buffer.
 *
 * @param[in] buffer The buffer.
 * @param[in] data The 8 bit unsigned integer.
 * @return true if the write was successful, otherwise false if not enough
 * memory was available.
 */
bool buffer_write_uint8(buffer_t *buffer, uint8_t data);

/**
 * Writes a 16 bit unsigned integer to the buffer.
 *
 * @param[in] buffer The buffer.
 * @param[in] data The 16 bit unsigned integer.
 * @return true if the write was successful, otherwise false if not enough
 * memory was available.
 */
bool buffer_write_uint16(buffer_t *buffer, uint16_t data);

/**
 * Writes a 32 bit unsigned integer to the buffer.
 *
 * @param[in] buffer The buffer.
 * @param[in] data The 32 bit unsigned integer.
 * @return true if the write was successful, otherwise false if not enough
 * memory was available.
 */
bool buffer_write_uint32(buffer_t *buffer, uint32_t data);

/**
 * Writes a 64 bit unsigned integer to the buffer.
 *
 * @param[in] buffer The buffer.
 * @param[in] data The 64 bit unsigned integer.
 * @return true if the write was successful, otherwise false if not enough
 * memory was available.
 */
bool buffer_write_uint64(buffer_t *buffer, uint64_t data);

/**
 * Writes an 8 bit signed integer to the buffer.
 *
 * @param[in] buffer The buffer.
 * @param[in] data The 8 bit signed integer.
 * @return true if the write was successful, otherwise false if not enough
 * memory was available.
 */
bool buffer_write_int8(buffer_t *buffer, int8_t data);

/**
 * Writes a 16 bit signed integer to the buffer.
 *
 * @param[in] buffer The buffer.
 * @param[in] data The 16 bit signed integer.
 * @return true if the write was successful, otherwise false if not enough
 * memory was available.
 */
bool buffer_write_int16(buffer_t *buffer, int16_t data);

/**
 * Writes a 32 bit signed integer to the buffer.
 *
 * @param[in] buffer The buffer.
 * @param[in] data The 32 bit signed integer.
 * @return true if the write was successful, otherwise false if not enough
 * memory was available.
 */
bool buffer_write_int32(buffer_t *buffer, int32_t data);

/**
 * Writes an 64 bit signed integer to the buffer.
 *
 * @param[in] buffer The buffer.
 * @param[in] data The 64 bit signed integer.
 * @return true if the write was successful, otherwise false if not enough
 * memory was available.
 */
bool buffer_write_int64(buffer_t *buffer, int64_t data);

/**
 * Writes a char to the buffer.
 *
 * @param[in] buffer The buffer.
 * @param[in] data The char.
 * @return true if the write was successful, otherwise false if not enough
 * memory was available.
 */
bool buffer_write_char(buffer_t *buffer, char data);

/**
 * Writes a float to the buffer.
 *
 * @param[in] buffer The buffer.
 * @param[in] data The float.
 * @return true if the write was successful, otherwise false if not enough
 * memory was available.
 */
bool buffer_write_float(buffer_t *buffer, float data);

/**
 * Writes a double to the buffer.
 *
 * @param[in] buffer The buffer.
 * @param[in] data The double.
 * @return true if the write was successful, otherwise false if not enough
 * memory was available.
 */
bool buffer_write_double(buffer_t *buffer, double data);

/**
 * Returns a pointer to the buffer's data.
 *
 * @param[in] buffer The buffer.
 * @return A pointer to the buffer's data.
 */
const unsigned char * buffer_data(buffer_t *buffer);

/**
 * Removes <tt>len</tt> bytes from the beginning from the buffer.
 *
 * @param[in] buffer The buffer.
 * @param[in] len The number of bytes to remove.
 * @return The number of bytes removed.
 */
size_t buffer_remove(buffer_t *buffer, size_t len);

/**
 * Clears the buffer.
 *
 * @param[in] buffer The buffer.
 */
void buffer_clear(buffer_t *buffer);
