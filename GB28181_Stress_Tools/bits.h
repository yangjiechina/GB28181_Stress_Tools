#pragma once

#ifndef BITS_H
#define BITS_H

#include <malloc.h>
#include <cstdint>

struct bits_buffer_t
{
	int     i_size;

	int     i_data;
	uint8_t i_mask;
	uint8_t *p_data;

};
typedef struct bits_buffer_t bits_buffer_t;

static  int bits_initwrite(struct bits_buffer_t *p_buffer, int i_size, void *p_data)
{
	p_buffer->i_size = i_size;
	p_buffer->i_data = 0;
	p_buffer->i_mask = 0x80;
	p_buffer->p_data = (uint8_t *)p_data;
	p_buffer->p_data[0] = 0;
	if (!p_buffer->p_data)
	{
		if (!(p_buffer->p_data = (uint8_t *)malloc(i_size)))
		{
			return(-1);
		}
		else
		{
			return(0);
		}
	}
	else
	{
		return(0);
	}
}

static  void bits_align(struct bits_buffer_t * p_buffer)
{
	if (p_buffer->i_mask != 0x80 && p_buffer->i_data < p_buffer->i_size)
	{
		p_buffer->i_mask = 0x80;
		p_buffer->i_data++;
		p_buffer->p_data[p_buffer->i_data] = 0x00;
	}
}

static  void bits_write(struct bits_buffer_t *p_buffer, int i_count, uint64_t i_bits)
{
	while (i_count > 0)
	{
		i_count--;

		if ((i_bits >> i_count) & 0x01)
		{
			p_buffer->p_data[p_buffer->i_data] |= p_buffer->i_mask;
		}
		else
		{
			p_buffer->p_data[p_buffer->i_data] &= ~p_buffer->i_mask;
		}
		p_buffer->i_mask >>= 1;
		if (p_buffer->i_mask == 0)
		{
			p_buffer->i_data++;
			p_buffer->i_mask = 0x80;
		}
	}
}

#endif  //BITS_H
