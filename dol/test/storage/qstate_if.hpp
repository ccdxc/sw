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
                  uint16_t ssd_q_num, uint16_t ssd_q_size);

int setup_pri_q_state(int src_lif, int src_qtype, int src_qid, char *pgm_bin,
                      uint8_t total_rings, uint8_t host_rings, uint16_t num_entries,
                      uint64_t base_addr, uint64_t entry_size, bool dst_valid, 
                      uint16_t dst_lif, uint8_t dst_qtype, uint32_t dst_qid, 
                      uint16_t vf_id, uint16_t sq_id, uint64_t ssd_bm_addr);

int update_pri_q_state(int src_lif, int src_qtype, int src_qid,
                       uint8_t lo_weight, uint8_t med_weight, uint8_t hi_weight,
                       uint8_t lo_running, uint8_t med_running, uint8_t hi_running,
                       uint8_t max_cmds, uint8_t num_running);

}  // namespace qstate_if

#endif
