#include <stdio.h>
#include <stdbool.h>
#include "gb_fifo.h"


unsigned char fifo_left(struct fifo * fifo)
{
	if(fifo->w > fifo->r)
		return (fifo->r + fifo->size - fifo->w);
	else
		return (fifo->r - fifo->w);
}

unsigned char fifo_write_byte(struct fifo * fifo, unsigned char data)
{
	if(fifo_full(fifo))
		return 0;
	
	fifo->data[fifo->w] = data;
	fifo->w++;
	fifo->w %= fifo->size;
	
	return 1;
}

unsigned char fifo_read_byte(struct fifo * fifo, unsigned char *data)
{
	if(fifo_empty(fifo))
		return 0;
	
	*data = fifo->data[fifo->r];
	fifo->r++;
	fifo->r %= fifo->size;
	
	return 1;
}

unsigned char fifo_write(struct fifo * fifo, unsigned char * data, unsigned char num)
{
	unsigned size = 0;
	unsigned char *p = data;
	
	while(num-- > 0) {
		if(fifo_write_byte(fifo, *p++))
			size++;
		else
			return size;	
	}
	
	return size;
}

unsigned char fifo_read(struct fifo * fifo, unsigned char * data, unsigned char num)
{
	unsigned char size = 0;
	unsigned char *p = data;
	
	while(num-- > 0) {
		if(fifo_read_byte(fifo, p++))
			size++;
		else
			return size;
	}
	
	return size;
}

