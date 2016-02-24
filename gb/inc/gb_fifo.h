#ifndef __FIFO_H
#define __FIFO_H
#include <stdbool.h>

struct fifo {
	unsigned char *data;
	unsigned char size;
	unsigned char r;
	unsigned char w;
};

bool __inline fifo_empty(struct fifo * fifo)
{
	return (fifo->r == fifo->w);
}

bool __inline fifo_full(struct fifo * fifo)
{
	return ((fifo->w + 1)%(fifo->size) == fifo->r);
}

unsigned char fifo_left(struct fifo * fifo);

unsigned char fifo_write_byte(struct fifo * fifo, unsigned char data);
unsigned char fifo_read_byte(struct fifo * fifo, unsigned char *data);

unsigned char fifo_write(struct fifo * fifo, unsigned char * data, unsigned char num);
unsigned char fifo_read(struct fifo * fifo, unsigned char * data, unsigned char num);

#endif
