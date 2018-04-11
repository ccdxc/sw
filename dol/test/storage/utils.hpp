#ifndef _LOG_HPP_
#define _LOG_HPP_

#include <stdint.h>
#include <string.h>
#include <stdio.h>

const static uint32_t kMinHostMemAllocSize = 64;

namespace utils {

extern const uint32_t kUtilsPageSize;

void write_bit_fields(void *ptr, unsigned start_bit_offset,
                      unsigned size_in_bits, uint64_t value);

void dump(uint8_t *buf, int size=64);

void dump1K(uint8_t *buf);

int hbm_buf_init();

int hbm_addr_alloc(uint32_t size, uint64_t *alloc_ptr);

int hbm_addr_alloc_page_aligned(uint32_t size, uint64_t *alloc_ptr);
int hbm_addr_alloc_spec_aligned(uint32_t size, uint64_t *alloc_ptr, uint32_t spec_align_size);
uint32_t roundup_to_pow_2(uint32_t x);

}  // namespace utils


#endif  // _LOG_HPP_
