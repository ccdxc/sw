#ifndef _QUEUES_HPP_
#define _QUEUES_HPP_

#include <stdint.h>

#define SQ_TYPE		0
#define CQ_TYPE		1
#define HQ_TYPE		2
#define EQ_TYPE		3

namespace queues {

int queues_setup();

int pvm_roce_sq_init(uint16_t roce_lif, uint16_t roce_qtype, 
                     uint32_t roce_qid, uint64_t base_addr, 
                     uint32_t num_entries, uint32_t entry_size);

void *nvme_sq_consume_entry(uint16_t qid, uint16_t *index);

void *pvm_sq_consume_entry(uint16_t qid, uint16_t *index);

void *nvme_cq_consume_entry(uint16_t qid, uint16_t *index);

void *pvm_cq_consume_entry(uint16_t qid, uint16_t *index);

uint16_t get_nvme_lif();

uint16_t get_pvm_lif();

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

uint64_t get_storage_hbm_addr();

}  // namespace queues

#endif
