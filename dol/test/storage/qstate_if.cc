#include <stdint.h>
#include <strings.h>
#include <stdio.h>
#include "dol/test/storage/log.hpp"
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

void write_bit_fields(void *ptr, unsigned start_bit_offset,
                      unsigned size_in_bits, uint64_t value) {
  uint8_t *p = (uint8_t *)ptr;
  int bit_no;
  int off;

  for (off = 0, bit_no = (size_in_bits - 1); bit_no >= 0; bit_no--, off++) {
    __write_bit_(p, start_bit_offset + off, value & (1ull << bit_no));
  }
}

int setup_q_state(int src_lif, int src_qtype, int src_qid, char *pgm_bin,
                  uint8_t total_rings, uint8_t host_rings, uint16_t num_entries,
                  uint64_t base_addr, uint64_t entry_size, bool dst_valid, 
                  uint16_t dst_lif, uint8_t dst_qtype, uint32_t dst_qid, 
                  uint16_t vf_id, uint16_t sq_id, uint64_t ssd_bm_addr, 
                  uint16_t ssd_q_num, uint16_t ssd_q_size, uint64_t ssd_ci_addr) {

  uint8_t q_state[64];
  uint8_t pc_offset;
  uint64_t next_pc = 0;
  uint64_t dst_qaddr = 0;

  bzero(q_state, sizeof(q_state));

  // Get the dst_qaddr only if destination is used in P4+
  if (dst_valid) {
    if (hal_if::get_lif_qstate_addr(dst_lif, dst_qtype, dst_qid, &dst_qaddr) < 0) {
      printf("Failed to get lif_qstate addr \n");
      return -1;
    }
  }

  // If no program binary name supplied => no need to set next_pc as 
  // it is a host queue
  if (pgm_bin) {
    if (hal_if::get_pgm_base_addr(pgm_bin, &next_pc) < 0) {
      printf("Failed to get base addr of %s\n", pgm_bin);
      return -1;
    }
    next_pc = next_pc >> 6;
  }

  if (hal_if::get_pgm_label_offset("txdma_stage0.bin", "storage_tx_stage0", &pc_offset) < 0) {
    printf("Failed to get pc offset for storage tx stage0\n");
    return -1;
  }

  write_bit_fields(q_state, 0, 8, pc_offset);
  write_bit_fields(q_state, 40, 4, total_rings);
  write_bit_fields(q_state, 44, 4, host_rings);
  write_bit_fields(q_state, 112, 16, num_entries);
  write_bit_fields(q_state, 128, 64, base_addr);
  write_bit_fields(q_state, 192, 16, entry_size);
  write_bit_fields(q_state, 208, 28, next_pc);
  // Program only if destination is used in P4+
  if (dst_valid) {
    write_bit_fields(q_state, 236, 34, dst_qaddr);
    write_bit_fields(q_state, 270, 11, dst_lif);
    write_bit_fields(q_state, 281, 3, dst_qtype);
    write_bit_fields(q_state, 284, 24, dst_qid);
  }
  write_bit_fields(q_state, 340, 34, ssd_bm_addr);
  write_bit_fields(q_state, 374, 16, ssd_q_num);
  write_bit_fields(q_state, 390, 16, ssd_q_size);
  write_bit_fields(q_state, 406, 34, ssd_ci_addr);

  //log::dump(q_state);

  if (hal_if::set_lif_qstate(src_lif, src_qtype, src_qid, q_state) < 0) {
    printf("Failed to set lif_qstate addr \n");
    return -1;
  }

  printf("Q state created with lif %u type %u, qid %u next_pc %lx base_addr %lx\n", 
         src_lif, src_qtype, src_qid, next_pc, base_addr);
  return 0;
}

int setup_pci_q_state(int src_lif, int src_qtype, int src_qid,
                      uint8_t total_rings, uint8_t host_rings, 
                      uint16_t num_entries, uint64_t base_addr, 
                      uint64_t entry_size, uint64_t push_addr,
                      uint64_t intr_addr, uint32_t intr_data, uint8_t intr_en) {

  uint8_t q_state[64];
  uint8_t pc_offset;

  bzero(q_state, sizeof(q_state));

  if (hal_if::get_pgm_label_offset("txdma_stage0.bin", "storage_tx_stage0", &pc_offset) < 0) {
    printf("Failed to get pc offset for storage tx stage0\n");
    return -1;
  }

  write_bit_fields(q_state, 0, 8, pc_offset);
  write_bit_fields(q_state, 40, 4, total_rings);
  write_bit_fields(q_state, 44, 4, host_rings);
  write_bit_fields(q_state, 112, 16, num_entries);
  write_bit_fields(q_state, 128, 64, base_addr);
  write_bit_fields(q_state, 192, 16, entry_size);
  write_bit_fields(q_state, 208, 64, push_addr);
  write_bit_fields(q_state, 272, 64, intr_addr);
  write_bit_fields(q_state, 336, 32, intr_data);
  write_bit_fields(q_state, 368, 1, intr_en);

  //log::dump(q_state);

  if (hal_if::set_lif_qstate(src_lif, src_qtype, src_qid, q_state) < 0) {
    printf("Failed to set lif_qstate addr \n");
    return -1;
  }

  printf("PCI Q state created with lif %u type %u, qid %u base_addr %lx\n", 
         src_lif, src_qtype, src_qid, base_addr);
  return 0;
}

int setup_pri_q_state(int src_lif, int src_qtype, int src_qid, char *pgm_bin,
                      uint8_t total_rings, uint8_t host_rings, uint16_t num_entries,
                      uint64_t base_addr, uint64_t entry_size, bool dst_valid, 
                      uint16_t dst_lif, uint8_t dst_qtype, uint32_t dst_qid, 
                      uint16_t vf_id, uint16_t sq_id, uint64_t ssd_bm_addr) {

  uint8_t q_state[64];
  uint8_t pc_offset;
  uint64_t next_pc = 0;
  uint64_t dst_qaddr = 0;

  bzero(q_state, sizeof(q_state));

  // Get the dst_qaddr only if destination is used in P4+
  if (dst_valid) {
    if (hal_if::get_lif_qstate_addr(dst_lif, dst_qtype, dst_qid, &dst_qaddr) < 0) {
      printf("Failed to get lif_qstate addr \n");
      return -1;
    }
  }

  // If no program binary name supplied => no need to set next_pc as 
  // it is a host queue
  if (pgm_bin) {
    if (hal_if::get_pgm_base_addr(pgm_bin, &next_pc) < 0) {
      printf("Failed to get base addr of %s\n", pgm_bin);
      return -1;
    }
    next_pc = next_pc >> 6;
  }

  if (hal_if::get_pgm_label_offset("txdma_stage0.bin", "storage_tx_pri_stage0", &pc_offset) < 0) {
    printf("Failed to get pc offset for storage tx stage0\n");
    return -1;
  }

  write_bit_fields(q_state, 0, 8, pc_offset);
  write_bit_fields(q_state, 40, 4, total_rings);
  write_bit_fields(q_state, 44, 4, host_rings);
  write_bit_fields(q_state, 208, 16, num_entries);
  write_bit_fields(q_state, 224, 64, base_addr);
  write_bit_fields(q_state, 288, 16, entry_size);
  write_bit_fields(q_state, 304, 8, 6); // hi weight
  write_bit_fields(q_state, 312, 8, 4); // med weight
  write_bit_fields(q_state, 320, 8, 2); // lo weight
  write_bit_fields(q_state, 360, 8, 63); // max cmds
  write_bit_fields(q_state, 368, 28, next_pc);
  // Program only if destination is used in P4+
  if (dst_valid) {
    write_bit_fields(q_state, 396, 34, dst_qaddr);
    write_bit_fields(q_state, 430, 11, dst_lif);
    write_bit_fields(q_state, 441, 3, dst_qtype);
    write_bit_fields(q_state, 444, 24, dst_qid);
  }
  write_bit_fields(q_state, 468, 34, ssd_bm_addr);

  //log::dump(q_state);

  if (hal_if::set_lif_qstate(src_lif, src_qtype, src_qid, q_state) < 0) {
    printf("Failed to set lif_qstate addr \n");
    return -1;
  }
  return 0;
}

int update_pri_q_state(int src_lif, int src_qtype, int src_qid,
                       uint8_t hi_weight, uint8_t med_weight, uint8_t lo_weight,
                       uint8_t hi_running, uint8_t med_running, uint8_t lo_running,
                       uint8_t num_running, uint8_t max_cmds) {
  uint8_t q_state[64];

  // Get the qstate
  if (hal_if::get_lif_qstate(src_lif, src_qtype, src_qid, q_state) < 0) {
    printf("Pri Q state GET FAILED for lif %u type %u, qid %u \n", 
           src_lif, src_qtype, src_qid);
    return -1;
  } else {
    printf("Pri Q state GET SUCCEEDED for lif %u type %u, qid %u \n", 
           src_lif, src_qtype, src_qid);
    //log::dump(q_state);
  }

  // Update the weights and counters
  write_bit_fields(q_state, 304, 8, hi_weight); // hi weight
  write_bit_fields(q_state, 312, 8, med_weight); // med weight
  write_bit_fields(q_state, 320, 8, lo_weight); // lo weight
  write_bit_fields(q_state, 328, 8, hi_running); // hi running
  write_bit_fields(q_state, 336, 8, med_running); // med running
  write_bit_fields(q_state, 344, 8, lo_running); // lo running
  write_bit_fields(q_state, 352, 8, num_running); // num running
  write_bit_fields(q_state, 360, 8, max_cmds); // max cmds

  // Write the qstate back
  if (hal_if::set_lif_qstate(src_lif, src_qtype, src_qid, q_state) < 0) {
    printf("Failed to set lif_qstate addr \n");
    return -1;
  }
  return 0;
}

int get_qstate_addr(int lif, int qtype, int qid, uint64_t *qaddr) {

  if (!qaddr) return -1;

  if (hal_if::get_lif_qstate_addr(lif, qtype, qid, qaddr) < 0) {
    printf("Failed to get lif_qstate addr \n");
    return -1;
  }
  return 0;
}

}  // namespace qstate_if
