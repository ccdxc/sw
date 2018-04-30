#ifndef _QUEUES_HPP_
#define _QUEUES_HPP_

#include <stdint.h>
#include "dol/test/storage/dp_mem.hpp"

using namespace dp_mem;

#define SQ_TYPE		0
#define CQ_TYPE		1
#define HQ_TYPE		2
#define EQ_TYPE		3

namespace queues {

int nvme_e2e_ssd_handle();

void seq_queue_pdma_num_set(uint64_t& num_pdma_queues);

int resources_init();

int lifs_setup();

int nvme_pvm_queues_setup();

int nvme_dp_queues_setup();

int pvm_queues_setup();

int seq_queues_setup();

void seq_queue_acc_sub_num_set(uint64_t& acc_scale_submissions,
                               uint64_t& acc_scale_chain_replica,
                               uint32_t acc_scale_test_max,
                               uint32_t acc_scale_test_num_true_chaining_tests);
int arm_queues_setup();

int pvm_roce_sq_init(uint16_t roce_lif, uint16_t roce_qtype, 
                     uint32_t roce_qid, dp_mem_t *mem, 
                     uint32_t num_entries, uint32_t entry_size);
int pvm_roce_cq_init(uint16_t roce_lif, uint16_t roce_qtype,
                     uint32_t roce_qid, dp_mem_t *mem,
                     uint32_t num_entries, uint32_t entry_size,
                     uint64_t xlate_addr);

dp_mem_t *nvme_sq_consume_entry(uint16_t qid, uint16_t *index);

dp_mem_t *pvm_sq_consume_entry(uint16_t qid, uint16_t *index);

dp_mem_t *seq_sq_consume_entry(uint16_t qid, uint16_t *index);

dp_mem_t *nvme_cq_consume_entry(uint16_t qid, uint16_t *index);

dp_mem_t *pvm_cq_consume_entry(uint16_t qid, uint16_t *index);

uint16_t get_nvme_lif();

uint16_t get_pvm_lif();

uint16_t get_seq_lif();

uint16_t get_arm_lif();

uint32_t get_nvme_bdf();

uint32_t get_host_nvme_sq(uint32_t offset);

uint32_t get_pvm_nvme_sq(uint32_t offset);

uint32_t get_pvm_r2n_tgt_sq(uint32_t offset);

uint32_t get_pvm_r2n_init_sq(uint32_t offset);

uint32_t get_pvm_r2n_host_sq(uint32_t offset);

uint32_t get_pvm_nvme_be_sq(uint32_t offset);

uint32_t get_pvm_ssd_sq(uint32_t offset);

uint32_t get_seq_pdma_sq(uint32_t offset);

uint32_t get_seq_r2n_sq(uint32_t offset);

uint32_t get_seq_xts_sq(uint32_t offset);

uint32_t get_seq_xts_status_sq(uint32_t offset);

uint32_t get_seq_roce_sq(uint32_t offset);

uint32_t get_seq_comp_sq(uint32_t offset);

uint32_t get_seq_comp_status_sq(uint32_t offset);

uint32_t get_host_nvme_cq(uint32_t offset);

uint32_t get_pvm_nvme_cq(uint32_t offset);

uint32_t get_pvm_r2n_cq(uint32_t offset);

uint32_t get_pvm_nvme_be_cq(uint32_t offset);

void ring_nvme_e2e_ssd();

void get_nvme_doorbell(uint16_t lif, uint8_t qtype, uint32_t qid, 
                       uint8_t ring, uint16_t index, 
                       uint64_t *db_addr, uint64_t *db_data);

void get_host_doorbell(uint16_t lif, uint8_t qtype, uint32_t qid, 
                       uint8_t ring, uint16_t index, 
                       uint64_t *db_addr, uint64_t *db_data);

void get_capri_doorbell(uint16_t lif, uint8_t qtype, uint32_t qid, 
                        uint8_t ring, uint16_t index, 
                        uint64_t *db_addr, uint64_t *db_data);
void get_capri_doorbell_with_pndx_inc(uint16_t lif, uint8_t qtype, uint32_t qid, 
                                      uint8_t ring, uint64_t *db_addr, uint64_t *db_data);

uint64_t get_storage_hbm_addr();

}  // namespace queues

#endif
