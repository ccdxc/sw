#include "dol/test/storage/utils.hpp"
#include "dol/test/storage/hal_if.hpp"

namespace utils {

const uint32_t kUtilsPageSize	= 4096;
const uint64_t kUtilsPageMask	= 0xFFFFFFFFFFFFF000ULL;

void __write_bit_(uint8_t *p, unsigned bit_off, bool val) {
  unsigned start_byte = bit_off >> 3;
  uint8_t mask = 1 << (7 - (bit_off & 7));
  if (val)
    p[start_byte] |= mask;
  else
    p[start_byte] &= ~mask;
}

void write_bit_fields(void *ptr, unsigned start_bit_offset,
                      unsigned size_in_bits, uint64_t value) {
  uint8_t *p = (uint8_t *)ptr;
  int bit_no;
  int off;

  for (off = 0, bit_no = (size_in_bits - 1); bit_no >= 0; bit_no--, off++) {
    __write_bit_(p, start_bit_offset + off, value & (1ull << bit_no));
  }
}

void dump(uint8_t *buf, int size) {
  int i;

  for (i = 0; i < size; i++) {
    printf("0x%2.2x ", buf[i]);
    if ((i & 7) == 7) {
      printf(" ");
    }
    if ((i & 0xf) == 0xf) {
      printf("\n");
    }
  }
  if (!((i & 0xf) == 0xf)) {
    printf("\n");
  }
}

void dump1K(uint8_t *buf) {
  dump(buf, 1024);
}

uint64_t storage_hbm_addr;
uint32_t storage_hbm_size;
uint32_t storage_hbm_running_size;

int
hbm_buf_init()
{
  // Allocatge HBM address for storage
  if (hal_if::alloc_hbm_address(&storage_hbm_addr, &storage_hbm_size) < 0) {
    printf("can't allocate HBM address for storage \n");
    return -1;
  }
  printf("Storage HBM address %lx size %d KB\n", storage_hbm_addr, storage_hbm_size);
  storage_hbm_size = storage_hbm_size * 1024;
  printf("Storage HBM address %lx (byte) size %d \n", storage_hbm_addr, storage_hbm_size);
  storage_hbm_running_size = 0;
  return 0;
}

int
hbm_addr_alloc(uint32_t size, uint64_t *alloc_ptr)
{
  if (!alloc_ptr) return -1;
  if ((size + storage_hbm_running_size) >= storage_hbm_size) {
    printf("total size %u running size %u requested size %u can't fit \n",
           storage_hbm_size, storage_hbm_running_size, size);
    return -1;
  }
  *alloc_ptr = storage_hbm_addr + storage_hbm_running_size;
  storage_hbm_running_size += size;
  return 0;
}

int
hbm_addr_alloc_spec_aligned(uint32_t size, uint64_t *alloc_ptr, uint32_t spec_align_size)
{
  // spec_align_size must be a power of 2
  if (!spec_align_size || (spec_align_size & (spec_align_size - 1))) return -1;
  if (!alloc_ptr) return -1;
  if (((storage_hbm_addr + storage_hbm_running_size) & (spec_align_size - 1)) == 0)
    return hbm_addr_alloc(size, alloc_ptr);
  uint32_t aligned_size = spec_align_size + size;
  if ((aligned_size + storage_hbm_running_size) >= storage_hbm_size) {
    printf("total size %u running size %u requested size %u aligned size %u can't fit \n",
           storage_hbm_size, storage_hbm_running_size, size, aligned_size);
    return -1;
  }
  *alloc_ptr = (storage_hbm_addr + storage_hbm_running_size + spec_align_size - 1) & ~((uint64_t)spec_align_size - 1);
  storage_hbm_running_size += aligned_size;
  return 0;
}

int
hbm_addr_alloc_page_aligned(uint32_t size, uint64_t *alloc_ptr)
{
  return hbm_addr_alloc_spec_aligned(size, alloc_ptr, kUtilsPageSize);
}
}  // namespace utils

