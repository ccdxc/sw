#include <stdint.h>
#include <strings.h>
#include <stdio.h>
#include "dol/test/storage/utils.hpp"
#include "dol/test/storage/hal_if.hpp"
#include "nic/model_sim/include/lib_model_client.h"

namespace qstate_if {

int get_qstate_addr(int lif, int qtype, int qid, uint64_t *qaddr) {

  if (!qaddr) return -1;

  if (hal_if::get_lif_qstate_addr(lif, qtype, qid, qaddr) < 0) {
    printf("Failed to get lif_qstate addr \n");
    return -1;
  }
  return 0;
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

  utils::write_bit_fields(q_state, 0, 8, pc_offset);
  utils::write_bit_fields(q_state, 40, 4, total_rings);
  utils::write_bit_fields(q_state, 44, 4, host_rings);
  utils::write_bit_fields(q_state, 112, 16, num_entries);
  utils::write_bit_fields(q_state, 128, 64, base_addr);
  utils::write_bit_fields(q_state, 192, 16, entry_size);
  utils::write_bit_fields(q_state, 208, 28, next_pc);
  // Program only if destination is used in P4+
  if (dst_valid) {
    utils::write_bit_fields(q_state, 236, 11, dst_lif);
    utils::write_bit_fields(q_state, 247, 3, dst_qtype);
    utils::write_bit_fields(q_state, 250, 24, dst_qid);
    utils::write_bit_fields(q_state, 274, 34, dst_qaddr);
  }
  utils::write_bit_fields(q_state, 340, 34, ssd_bm_addr);
  utils::write_bit_fields(q_state, 374, 16, ssd_q_num);
  utils::write_bit_fields(q_state, 390, 16, ssd_q_size);
  utils::write_bit_fields(q_state, 406, 34, ssd_ci_addr);

  //utils::dump(q_state);

  if (hal_if::set_lif_qstate(src_lif, src_qtype, src_qid, q_state) < 0) {
    printf("Failed to set lif_qstate addr \n");
    return -1;
  }

  uint64_t qaddr;
  if (get_qstate_addr(src_lif, src_qtype, src_qid, &qaddr) < 0) {
    printf("Failed to get q state address \n");
    return -1;
  }
  printf("Q state addr %lx created with lif %u type %u, qid %u next_pc %lx base_addr %lx\n", 
         qaddr, src_lif, src_qtype, src_qid, next_pc, base_addr);

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

  utils::write_bit_fields(q_state, 0, 8, pc_offset);
  utils::write_bit_fields(q_state, 40, 4, total_rings);
  utils::write_bit_fields(q_state, 44, 4, host_rings);
  utils::write_bit_fields(q_state, 112, 16, num_entries);
  utils::write_bit_fields(q_state, 128, 64, base_addr);
  utils::write_bit_fields(q_state, 192, 16, entry_size);
  utils::write_bit_fields(q_state, 208, 64, push_addr);
  utils::write_bit_fields(q_state, 272, 64, intr_addr);
  utils::write_bit_fields(q_state, 336, 32, intr_data);
  utils::write_bit_fields(q_state, 368, 1, intr_en);

  //utils::dump(q_state);

  if (hal_if::set_lif_qstate(src_lif, src_qtype, src_qid, q_state) < 0) {
    printf("Failed to set lif_qstate addr \n");
    return -1;
  }

  uint64_t qaddr;
  if (get_qstate_addr(src_lif, src_qtype, src_qid, &qaddr) < 0) {
    printf("Failed to get q state address \n");
    return -1;
  }
  printf("PCI Q state addr %lx created with lif %u type %u, qid %u base_addr %lx\n", 
         qaddr, src_lif, src_qtype, src_qid, base_addr);
  return 0;
}

int setup_pri_q_state(int src_lif, int src_qtype, int src_qid, char *pgm_bin,
                      uint8_t total_rings, uint8_t host_rings, uint16_t num_entries,
                      uint64_t base_addr, uint64_t entry_size, bool dst_valid, 
                      uint16_t dst_lif, uint8_t dst_qtype, uint32_t dst_qid, 
                      uint16_t vf_id, uint16_t sq_id, uint64_t ssd_bm_addr,
                      bool raise_weights) {

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

  utils::write_bit_fields(q_state, 0, 8, pc_offset);
  utils::write_bit_fields(q_state, 40, 4, total_rings);
  utils::write_bit_fields(q_state, 44, 4, host_rings);
  utils::write_bit_fields(q_state, 208, 16, num_entries);
  utils::write_bit_fields(q_state, 224, 64, base_addr);
  utils::write_bit_fields(q_state, 288, 16, entry_size);
  if (raise_weights) {
    utils::write_bit_fields(q_state, 303, 8, 50); // hi weight
    utils::write_bit_fields(q_state, 311, 8, 48); // med weight
    utils::write_bit_fields(q_state, 320, 8, 46); // lo weight
  } else {
    utils::write_bit_fields(q_state, 303, 8, 6); // hi weight
    utils::write_bit_fields(q_state, 311, 8, 4); // med weight
    utils::write_bit_fields(q_state, 320, 8, 2); // lo weight
  }
  utils::write_bit_fields(q_state, 360, 8, 63); // max cmds
  utils::write_bit_fields(q_state, 368, 28, next_pc);
  // Program only if destination is used in P4+
  if (dst_valid) {
    utils::write_bit_fields(q_state, 396, 11, dst_lif);
    utils::write_bit_fields(q_state, 407, 3, dst_qtype);
    utils::write_bit_fields(q_state, 410, 24, dst_qid);
    utils::write_bit_fields(q_state, 434, 34, dst_qaddr);
  }
  utils::write_bit_fields(q_state, 468, 34, ssd_bm_addr);

  //utils::dump(q_state);

  if (hal_if::set_lif_qstate(src_lif, src_qtype, src_qid, q_state) < 0) {
    printf("Failed to set lif_qstate addr \n");
    return -1;
  }

  uint64_t qaddr;
  if (get_qstate_addr(src_lif, src_qtype, src_qid, &qaddr) < 0) {
    printf("Failed to get q state address \n");
    return -1;
  }
  printf("PRI Q state addr %lx created with lif %u type %u, qid %u next_pc %lx base_addr %lx\n", 
         qaddr, src_lif, src_qtype, src_qid, next_pc, base_addr);

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
    //utils::dump(q_state);
  }

  // Update the weights and counters
  utils::write_bit_fields(q_state, 304, 8, hi_weight); // hi weight
  utils::write_bit_fields(q_state, 312, 8, med_weight); // med weight
  utils::write_bit_fields(q_state, 320, 8, lo_weight); // lo weight
  utils::write_bit_fields(q_state, 328, 8, hi_running); // hi running
  utils::write_bit_fields(q_state, 336, 8, med_running); // med running
  utils::write_bit_fields(q_state, 344, 8, lo_running); // lo running
  utils::write_bit_fields(q_state, 352, 8, num_running); // num running
  utils::write_bit_fields(q_state, 360, 8, max_cmds); // max cmds

  // Write the qstate back
  if (hal_if::set_lif_qstate(src_lif, src_qtype, src_qid, q_state) < 0) {
    printf("Failed to set lif_qstate addr \n");
    return -1;
  }
  return 0;
}

int setup_roce_sq_state(int src_lif, int src_qtype, int src_qid, char *pgm_bin,
                        uint8_t total_rings, uint8_t host_rings, uint16_t num_entries,
                        uint64_t base_addr, uint64_t entry_size, bool rrq_valid, 
                        uint16_t rrq_lif, uint8_t rrq_qtype, uint32_t rrq_qid, 
                        uint16_t rsq_lif, uint8_t rsq_qtype, uint32_t rsq_qid) {

  uint8_t q_state[64];
  uint8_t pc_offset;
  uint64_t next_pc = 0;
  uint64_t rrq_qaddr = 0;

  bzero(q_state, sizeof(q_state));

  printf("Setting LIF %u, type %u, qid %u \n", rsq_lif, rsq_qtype, rsq_qid);

  // Get the rrq_qaddr only if destination is used in P4+
  if (rrq_valid) {
    if (hal_if::get_lif_qstate_addr(rrq_lif, rrq_qtype, rrq_qid, &rrq_qaddr) < 0) {
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

  if (hal_if::get_pgm_label_offset("txdma_stage0.bin", "storage_tx_rsq_stage0", &pc_offset) < 0) {
    printf("Failed to get pc offset for storage tx stage0\n");
    return -1;
  }

  utils::write_bit_fields(q_state, 0, 8, pc_offset);
  utils::write_bit_fields(q_state, 40, 4, total_rings);
  utils::write_bit_fields(q_state, 44, 4, host_rings);
  utils::write_bit_fields(q_state, 96, 64, base_addr);
  utils::write_bit_fields(q_state, 165, 5, entry_size);
  utils::write_bit_fields(q_state, 170, 5, num_entries);
  utils::write_bit_fields(q_state, 224, 28, next_pc);
  // Program only if rrq is used in P4+
  if (rrq_valid) {
    utils::write_bit_fields(q_state, 252, 11, rrq_lif);
    utils::write_bit_fields(q_state, 263, 3, rrq_qtype);
    utils::write_bit_fields(q_state, 266, 24, rrq_qid);
    utils::write_bit_fields(q_state, 290, 34, rrq_qaddr);
  }
  utils::write_bit_fields(q_state, 324, 11, rsq_lif);
  utils::write_bit_fields(q_state, 335, 3, rsq_qtype);
  utils::write_bit_fields(q_state, 338, 24, rsq_qid);

  //utils::dump(q_state);

  if (hal_if::set_lif_qstate(src_lif, src_qtype, src_qid, q_state) < 0) {
    printf("Failed to set lif_qstate addr \n");
    return -1;
  }

  uint64_t qaddr;
  if (get_qstate_addr(src_lif, src_qtype, src_qid, &qaddr) < 0) {
    printf("Failed to get q state address \n");
    return -1;
  }
  printf("Q state addr %lx created with lif %u type %u, qid %u next_pc %lx base_addr %lx\n", 
         qaddr, src_lif, src_qtype, src_qid, next_pc, base_addr);

  return 0;
}

int setup_roce_cq_state(int src_lif, int src_qtype, int src_qid, char *pgm_bin,
                        uint8_t total_rings, uint8_t host_rings, uint16_t num_entries,
                        uint64_t base_addr, uint64_t entry_size, uint64_t xlate_addr,
                        uint16_t rcq_lif, uint8_t rcq_qtype, uint32_t rcq_qid) {

  uint8_t q_state[64];
  uint8_t pc_offset;
  uint64_t next_pc = 0;

  bzero(q_state, sizeof(q_state));

  printf("Setting LIF %u, type %u, qid %u \n", rcq_lif, rcq_qtype, rcq_qid);

  // If no program binary name supplied => no need to set next_pc as 
  // it is a host queue
  if (pgm_bin) {
    if (hal_if::get_pgm_base_addr(pgm_bin, &next_pc) < 0) {
      printf("Failed to get base addr of %s\n", pgm_bin);
      return -1;
    }
    next_pc = next_pc >> 6;
  }

  if (hal_if::get_pgm_label_offset("txdma_stage0.bin", "storage_tx_rcq_stage0", &pc_offset) < 0) {
    printf("Failed to get pc offset for storage tx stage0\n");
    return -1;
  }

  utils::write_bit_fields(q_state, 0, 8, pc_offset);
  utils::write_bit_fields(q_state, 40, 4, total_rings);
  utils::write_bit_fields(q_state, 44, 4, host_rings);
  utils::write_bit_fields(q_state, 96, 64, base_addr);
  utils::write_bit_fields(q_state, 165, 5, entry_size);
  utils::write_bit_fields(q_state, 170, 5, num_entries);
  utils::write_bit_fields(q_state, 304, 28, next_pc);
  utils::write_bit_fields(q_state, 332, 34, xlate_addr);
  utils::write_bit_fields(q_state, 366, 11, rcq_lif);
  utils::write_bit_fields(q_state, 377, 3, rcq_qtype);
  utils::write_bit_fields(q_state, 380, 24, rcq_qid);

  //utils::dump(q_state);

  if (hal_if::set_lif_qstate(src_lif, src_qtype, src_qid, q_state) < 0) {
    printf("Failed to set lif_qstate addr \n");
    return -1;
  }

  uint64_t qaddr;
  if (get_qstate_addr(src_lif, src_qtype, src_qid, &qaddr) < 0) {
    printf("Failed to get q state address \n");
    return -1;
  }
  printf("Q state addr %lx created with lif %u type %u, qid %u next_pc %lx base_addr %lx\n", 
         qaddr, src_lif, src_qtype, src_qid, next_pc, base_addr);

  return 0;
}

int update_xlate_entry(int lif, int qtype, int qid, uint64_t hbm_addr, char *pgm_bin) {

  uint64_t qaddr;
  uint64_t next_pc = 0;
  uint8_t data[64];

  bzero(data, sizeof(data));

  printf("Updating xlate entry @ hbm_addr %lx with "
         "LIF %u, type %u, qid %u \n", hbm_addr, lif, qtype, qid);

  if (get_qstate_addr(lif, qtype, qid, &qaddr) < 0) {
    printf("Failed to get q state address \n");
    return -1;
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

  utils::write_bit_fields(data, 0, 28, next_pc);
  utils::write_bit_fields(data, 28, 11, lif);
  utils::write_bit_fields(data, 39, 3, qtype);
  utils::write_bit_fields(data, 42, 24, qid);
  utils::write_bit_fields(data, 66, 34, qaddr);

  write_mem(hbm_addr, data, 64);
  
  return 0;
}


}  // namespace qstate_if
