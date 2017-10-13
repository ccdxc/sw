#ifndef _R2N_HPP_
#define _R2N_HPP_

#include <stdint.h>
#include "dol/test/storage/nvme.hpp"

namespace r2n {

typedef struct r2n_buf_post_ {
  union {
    uint8_t bytes[64];
  };
} r2n_buf_post_t;

typedef struct r2n_prp_list_ {
  union {
    uint8_t bytes[128];
  };
} r2n_prp_list_t;

typedef struct nvme_be_sta_ {
  union {
    uint8_t bytes[64];
  };
} nvme_be_sta_t;

typedef struct r2n_sta_req_ {
  union {
    uint8_t bytes[64];
  };
} r2n_sta_req_t;

typedef struct nvme_be_cmd_ {
  union {
    uint8_t bytes[128];
    struct {
      uint32_t src_queue_id;
      uint16_t ssd_handle;
      uint8_t io_priority;
      uint8_t is_read;
      uint64_t r2n_buf_handle;
      uint8_t is_local;
      uint8_t rsvd[47];
      struct NvmeCmd nvme_cmd;
    } __attribute__((packed)) s;
  };
} nvme_be_cmd_t;

typedef struct r2n_write_req_ {
  union {
    uint8_t bytes[64];
  };
} r2n_write_req_t;

typedef struct r2n_wqe_ {
  uint64_t handle;
  uint32_t data_size;
  uint32_t opcode;
  uint32_t status;
} r2n_wqe_t;

void *r2n_buf_alloc();

void r2n_nvme_be_cmd_init(void *r2n_buf, uint32_t src_queue_id, uint16_t ssd_handle,
                          uint8_t io_priority, uint8_t is_read, uint8_t is_local);

uint8_t *r2n_nvme_cmd_ptr(void *r2n_buf);

void r2n_wqe_init(void *r2n_wqe_buf, void *r2n_buf);

void r2n_wqe_db_update(void *r2n_wqe_buf, uint16_t lif, uint8_t qtype, 
                       uint32_t qid, uint16_t index);

}  // namespace r2n

#endif   // _R2N_HPP_
