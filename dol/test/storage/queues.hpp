#ifndef _QUEUES_HPP_
#define _QUEUES_HPP_

#include <stdint.h>

#define SQ_TYPE		0
#define CQ_TYPE		1
#define HQ_TYPE		2
#define EQ_TYPE		3

namespace queues {

int queues_setup();

void *nvme_sq_consume_entry(uint16_t qid, uint16_t *index);

void *pvm_sq_consume_entry(uint16_t qid, uint16_t *index);

uint16_t get_nvme_lif();

uint16_t get_pvm_lif();

}  // namespace queues

#endif
