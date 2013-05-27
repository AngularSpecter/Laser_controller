/*
 * buffer.h
 *
 *	Provide support for a circular buffer.
 *
 *  Created on: May 19, 2013
 *      Author: Collin
 */

#include <stdint.h>

#ifndef BUFFER_H_
#define BUFFER_H_

#define BUFFER_LEN 32

typedef struct
{
	char buffer[BUFFER_LEN];		//buffer data store.
	uint8_t rx_index;				//current read index.
	uint8_t tx_index;				//curent write index.

}circular_buffer;

void buffer_init(circular_buffer* buf);
uint8_t buffer_write(circular_buffer* buf, char* data, uint8_t len);
uint8_t buffer_read(circular_buffer* buf, char* data, uint8_t len);
uint8_t buffer_peek(circular_buffer* buf, char* data, uint8_t len);

#endif /* BUFFER_H_ */
