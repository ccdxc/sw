/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#include "pnso_utils.h"

void
__write_bit_(uint8_t *p, unsigned int bit_off, bool val)
{
	unsigned int start_byte = bit_off >> 3;
	uint8_t mask = 1 << (7 - (bit_off & 7));

	if (val)
		p[start_byte] |= mask;
	else
		p[start_byte] &= ~mask;
}

void
write_bit_fields(void *ptr, unsigned int start_bit_offset,
		unsigned int size_in_bits, uint64_t value)
{
	uint8_t *p = (uint8_t *)ptr;
	int bit_no;
	int off;

	for (off = 0, bit_no = (size_in_bits - 1);
			bit_no >= 0; bit_no--, off++) {
		__write_bit_(p, start_bit_offset + off,
				value & (1ull << bit_no));
	}
}
