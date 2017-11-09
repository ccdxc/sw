#include "dol/test/storage/utils.hpp"
#include "dol/test/storage/hal_if.hpp"

namespace utils {

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

void dump(uint8_t *buf) {
  int i;

  for (i = 0; i < 64; i++) {
    printf("%2.2x ", buf[i]);
    if ((i & 7) == 7) {
      printf(" ");
    }
    if ((i & 0xf) == 0xf) {
      printf("\n");
    }
  }
}

void dump1K(uint8_t *buf) {
  int i;

  printf("Dumping 1K of %p \n", buf);
  for (i = 0; i < 1024; i++) {
    printf("%2.2x ", buf[i]);
    if ((i & 7) == 7) {
      printf(" ");
    }
    if ((i & 0xf) == 0xf) {
      printf("\n");
    }
  }
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

}  // namespace utils

