#include <stdint.h>
#include <strings.h>
#include <stdio.h>
#include "dol/test/storage/hal_if.hpp"

namespace qstate_if {

void __write_bit_(uint8_t *p, unsigned bit_off, bool val) {
  unsigned start_byte = bit_off >> 3;
  uint8_t mask = 1 << (7 - (bit_off & 7));
  if (val)
    p[start_byte] |= mask;
  else
    p[start_byte] &= ~mask;
}

void write_q_state_field(void *ptr, unsigned start_bit_offset,
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


int setup_q_state(int src_lif, int src_qtype, int src_qid, char *pgm_bin,
                  uint8_t total_rings, uint8_t host_rings, uint16_t num_entries,
                  uint64_t base_addr, uint64_t entry_size, uint16_t dst_lif, 
                  uint8_t dst_qtype, uint32_t dst_qid, uint16_t vf_id, uint16_t sq_id,
                  uint64_t ssd_bm_addr, uint16_t ssd_q_num, uint16_t ssd_q_size) {

  uint8_t q_state[64];
  uint8_t pc_offset;
  uint64_t next_pc;
  uint64_t dst_qaddr = 0;

  bzero(q_state, sizeof(q_state));

  if (!pgm_bin) {
    printf("Input error\n");
    return -1;
  }

  if (hal_if::get_lif_qstate_addr(dst_lif, dst_qtype, dst_qid, &dst_qaddr) < 0) {
    printf("Failed to get lif_qstate addr \n");
    return -1;
  }

  if (hal_if::get_pgm_base_addr(pgm_bin, &next_pc) < 0) {
    printf("Failed to get base addr of %s\n", pgm_bin);
    return -1;
  }
  next_pc = next_pc >> 6;

  if (hal_if::get_pgm_label_offset("txdma_stage0.bin", "storage_tx_stage0", &pc_offset) < 0) {
    printf("Failed to get pc offset for storage tx stage0\n");
    return -1;
  }

  write_q_state_field(q_state, 0, 8, pc_offset);
  write_q_state_field(q_state, 40, 4, total_rings);
  write_q_state_field(q_state, 44, 4, host_rings);
  write_q_state_field(q_state, 112, 16, num_entries);
  write_q_state_field(q_state, 128, 64, base_addr);
  write_q_state_field(q_state, 192, 16, entry_size);
  write_q_state_field(q_state, 208, 28, next_pc);
  write_q_state_field(q_state, 236, 34, dst_qaddr);
  write_q_state_field(q_state, 270, 11, dst_lif);
  write_q_state_field(q_state, 281, 3, dst_qtype);
  write_q_state_field(q_state, 284, 24, dst_qid);
  write_q_state_field(q_state, 340, 34, ssd_bm_addr);
  write_q_state_field(q_state, 374, 16, ssd_q_num);
  write_q_state_field(q_state, 390, 16, ssd_q_size);

  dump(q_state);

  if (hal_if::set_lif_qstate(src_lif, src_qtype, src_qid, q_state) < 0) {
    printf("Failed to set lif_qstate addr \n");
    return -1;
  }

  printf("Q state created\n");
  return 0;
}

}  // namespace qstate_if
