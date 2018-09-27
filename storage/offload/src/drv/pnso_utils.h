/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#ifndef __PNSO_UTILS_H__
#define __PNSO_UTILS_H__

#include "osal.h"

void write_bit_fields(void *ptr, unsigned int start_bit_offset,
		      unsigned int size_in_bits, uint64_t value);

#endif  /* __PNSO_UTILS_H__ */
