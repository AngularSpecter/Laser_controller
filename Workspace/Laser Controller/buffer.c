/*
 * buffer.c
 *
 *  Created on: May 19, 2013
 *      Author: Collin
 */

#include <stdint.h>
#include <stdlib.h>
#include "buffer.h"

/* _isBufferFull()
 *
 * Query a buffer to find out if it's full.
 *
 * Arguments:
 * buf:		The buffer to query.
 *
 * Returns:
 * True, if the buffer is full.
 *
 */
static uint8_t _isBufferFull(circular_buffer* buf)
{
	/* The range of the rx/tx indices is [0:2 * BUFFER_LEN).
	 * If the indices are equal, the buffers are empty, if the
	 * buffer is filled, the indices will differ by BUFFER_LEN.
	 *
	 * To account for wrapping, BUFFER_LEN is set to a power of two,
	 * and the uppermost bit of the indices is compared, if they differ,
	 * but the remainder of the bits match, the indices are separated by BUFFER_LEN.
	 */
	return (buf->rx_index ^ buf->tx_index) == BUFFER_LEN;
}

/* _isBufferEmpty()
 *
 * Query a buffer to find out if it's empty.
 *
 * Arguments:
 * buf:		The buffer to query.
 *
 * Returns:
 * True, if the buffer is empty.
 *
 */
static uint8_t _isBufferEmpty(circular_buffer* buf)
{
	/* If the indices are the same, bit for bit, then the buffer is empty. */
	return (buf->rx_index == buf->tx_index);
}

/* buffer_init()
 *
 * Initializes a circular buffer.
 *
 * Arguments:
 * buf:		The buffer to initialize.
 *
 * Returns:
 * NONE
 *
 */
void buffer_init(circular_buffer* buf)
{
	memset(buf, 0, sizeof(circular_buffer));
}

/* buffer_write()
 *
 * Writes data to a buffer.
 *
 * Arguments:
 * buf:		The buffer to write data into.
 * data:	A pointer to the data to be written.
 * len:		The number of bytes to write into the buffer from the data pointer.
 *
 * Returns:
 * The total number of bytes actually written.
 *
 */
uint8_t buffer_write(circular_buffer* buf, char* data, uint8_t len)
{
	uint8_t k = 0;

	/* Write data until the buffer is full. */
	k = 0;
	for (k = 0; k < len; k++)
	{
		/* If the buffer is full, return the total number of characters read to this point. */
		if (_isBufferFull(buf))
		{
			return k;
		}

		/* Write the data. */
		buf->buffer[buf->tx_index & (BUFFER_LEN - 1)] = data[k]; //only use the lower part of the index.

		/* Increment the tx_index, wrap it once it reaches (2 * BUFFER_LEN - 1). */
		buf->tx_index++;
		if (buf->tx_index >= (2 * BUFFER_LEN))
		{
			buf->tx_index = 0;
		}
	}

	return k;
}

/* buffer_read()
 *
 * Reads data from a buffer.
 *
 * Arguments:
 * buf:		The buffer to read data from.
 * data:	A pointer to the write the data too.
 * len:		The number of bytes to read from the buffer into the data pointer.
 *
 * Returns:
 * The total number of bytes actually read.
 *
 */
uint8_t buffer_read(circular_buffer* buf, char* data, uint8_t len)
{
	uint8_t k = 0;

	/* Read data until the buffer is full. */
	k = 0;
	for (k = 0; k < len; k++)
	{
		/* If the buffer is empty, return the total number of characters read to this point. */
		if (_isBufferEmpty(buf))
		{
			return k;
		}

		/* Read the data. */
		data[k] = buf->buffer[buf->rx_index & (BUFFER_LEN - 1)]; //only use the lower part of the index.

		/* Increment the rx_index, wrap it once it reaches (2 * BUFFER_LEN - 1). */
		buf->rx_index++;
		if (buf->rx_index >= (2 * BUFFER_LEN))
		{
			buf->rx_index = 0;
		}
	}

	return k;
}

