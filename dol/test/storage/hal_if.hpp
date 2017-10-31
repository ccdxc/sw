#ifndef _HAL_IF_HPP_
#define _HAL_IF_HPP_

#include <stdint.h>

#define LIF_MAX_TYPES		8
#define LIF_BASE_QUEUE_SIZE	5

namespace hal_if {

typedef struct lif_params_ {
  uint32_t sw_lif_id;
  struct {
    bool valid;
    uint16_t queue_size; // Log2 value 
    uint16_t num_queues; // Log2 value
    int queue_purpose;
  } type[LIF_MAX_TYPES];

  // RDMA parameters
  bool rdma_enable;
  uint32_t rdma_max_keys;
  uint32_t rdma_max_pt_entries;
} lif_params_t;

void init_hal_if();
int get_pgm_base_addr(const char *prog_name, uint64_t *base_addr);
int get_pgm_label_offset(const char *prog_name,
                         const char *label, uint8_t *off);
int set_lif_qstate(uint32_t lif, uint32_t qtype, uint32_t qid,
                   uint8_t *qstate);
int create_lif(lif_params_t *params, uint64_t *lif_id);
int get_lif_qstate_addr(uint32_t lif, uint32_t qtype, uint32_t qid,
                        uint64_t *qaddr);
int get_lif_qstate(uint32_t lif, uint32_t qtype, uint32_t qid, uint8_t *qstate);
int alloc_hbm_address(uint64_t *addr, uint32_t *size);

}  // namespace hal_if

#endif
