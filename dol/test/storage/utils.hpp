#ifndef _LOG_HPP_
#define _LOG_HPP_

#include <stdint.h>
#include <string.h>
#include <stdio.h>

namespace utils {

void write_bit_fields(void *ptr, unsigned start_bit_offset,
                      unsigned size_in_bits, uint64_t value);

void dump(uint8_t *buf);

int hbm_buf_init();

int hbm_addr_alloc(uint32_t size, uint64_t *alloc_ptr);

}  // namespace utils


#endif  // _LOG_HPP_
