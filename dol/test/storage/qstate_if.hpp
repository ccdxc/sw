#ifndef _QSTATE_IF_HPP_
#define _QSTATE_IF_HPP_

#include <stdint.h>

namespace qstate_if {

void write_bit_fields(void *ptr, unsigned start_bit_offset,
                      unsigned size_in_bits, uint64_t value);

int setup_q_state(int src_lif, int src_qtype, int src_qid, char *pgm_bin,
                  uint8_t total_rings, uint8_t host_rings, uint16_t num_entries,
                  uint64_t base_addr, uint64_t entry_size, bool dst_valid, 
                  uint16_t dst_lif, uint8_t dst_qtype, uint32_t dst_qid, 
                  uint16_t vf_id, uint16_t sq_id, uint64_t ssd_bm_addr, 
                  uint16_t ssd_q_num, uint16_t ssd_q_size, uint64_t ssd_ci_addr);

int setup_pci_q_state(int src_lif, int src_qtype, int src_qid,
                      uint8_t total_rings, uint8_t host_rings, 
                      uint16_t num_entries, uint64_t base_addr, 
                      uint64_t entry_size, uint64_t push_addr,
                      uint64_t intr_addr, uint32_t intr_data, uint8_t intr_en);

int setup_pri_q_state(int src_lif, int src_qtype, int src_qid, char *pgm_bin,
                      uint8_t total_rings, uint8_t host_rings, uint16_t num_entries,
                      uint64_t base_addr, uint64_t entry_size, bool dst_valid, 
                      uint16_t dst_lif, uint8_t dst_qtype, uint32_t dst_qid, 
                      uint16_t vf_id, uint16_t sq_id, uint64_t ssd_bm_addr);

int update_pri_q_state(int src_lif, int src_qtype, int src_qid,
                       uint8_t lo_weight, uint8_t med_weight, uint8_t hi_weight,
                       uint8_t lo_running, uint8_t med_running, uint8_t hi_running,
                       uint8_t max_cmds, uint8_t num_running);

int get_qstate_addr(int lif, int qtype, int qid, uint64_t *qaddr);

int setup_roce_sq_state(int src_lif, int src_qtype, int src_qid, char *pgm_bin,
                        uint8_t total_rings, uint8_t host_rings, uint16_t num_entries,
                        uint64_t base_addr, uint64_t entry_size, bool rrq_valid, 
                        uint16_t rrq_lif, uint8_t rrq_qtype, uint32_t rrq_qid, 
                        uint16_t rsq_lif, uint8_t rsq_qtype, uint32_t rsq_qid);

int setup_roce_cq_state(int src_lif, int src_qtype, int src_qid, char *pgm_bin,
                        uint8_t total_rings, uint8_t host_rings, uint16_t num_entries,
                        uint64_t base_addr, uint64_t entry_size, uint64_t xlate_addr,
                        uint16_t rcq_lif, uint8_t rcq_qtype, uint32_t rcq_qid);


int update_xlate_entry(int lif, int qtype, int qid, uint64_t hbm_addr, char *pgm_bin);

}  // namespace qstate_if

#endif
