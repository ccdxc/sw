#ifndef _R2N_HPP_
#define _R2N_HPP_

#include <stdint.h>
#include "dol/test/storage/nvme.hpp"
#include "dol/test/storage/dp_mem.hpp"

#define R2N_OPCODE_PROCESS_WQE                  1
#define R2N_OPCODE_BUF_POST                     2


namespace r2n {

typedef struct {
  uint64_t wrid;               // 0
  uint32_t op_type:4,          // 64
           complete_notify:1,  // 68
           fence:1,            // 69
           solicited_event:1,  // 70
           inline_data_valid:1,// 71
           num_sges:8,         // 72
           rsvd1:16;           // 80

  uint32_t imm_data;           // 96
  uint32_t inv_key;            // 128
  uint32_t rsvd2;              // 160
  uint32_t length;             // 192
  uint32_t rsvd3;              // 224

  // SGE 0
  uint64_t va0;                // 256
  uint32_t len0;               // 320
  uint32_t l_key0;             // 352

  // SGE 1
  uint64_t va1;                // 384
  uint32_t len1;               // 448
  uint32_t l_key1;             // 480
} __attribute__((packed)) roce_sq_wqe_t;

typedef struct {
  uint64_t wrid;               // 0
  uint8_t  num_sges;           // 64
  uint8_t  rsvd[23];           // 72

  // SGE 0
  uint64_t va0;                // 256
  uint32_t len0;               // 320
  uint32_t l_key0;             // 352

  // SGE 1
  uint64_t va1;                // 384
  uint32_t len1;               // 448
  uint32_t l_key1;             // 480
} __attribute__((packed)) roce_rq_wqe_t;

typedef struct r2n_buf_post_desc_ {
  union {
    uint8_t bytes[64];
    roce_sq_wqe_t s;
  };
} r2n_buf_post_desc_t;

typedef struct r2n_prp_list_ {
  union {
    uint8_t bytes[128];
    struct {
      uint64_t prps[16];
    } __attribute__((packed)) s;
  };
} r2n_prp_list_t;

typedef struct nvme_be_sta_ {
  union {
    uint8_t bytes[64];
    struct {
      uint32_t time_us;
      uint8_t be_status;
      uint8_t is_q0;
      uint16_t rsvd;
      uint64_t r2n_buf_handle;
    } __attribute__((packed)) s;
  };
} nvme_be_sta_t;

typedef struct r2n_sta_desc_ {
  union {
    uint8_t bytes[64];
    roce_sq_wqe_t s;
  };
} r2n_sta_desc_t;

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

typedef struct r2n_write_desc_ {
  union {
    uint8_t bytes[64];
    roce_sq_wqe_t s;
  };
} r2n_write_desc_t;

typedef struct r2n_buf_ {
  r2n_buf_post_desc_t	buf_post_desc;
  r2n_prp_list_t	prp_buf;
  nvme_be_sta_t		sta_buf;
  r2n_sta_desc_t	sta_desc;
  nvme_be_cmd_t		cmd_buf;
  r2n_write_desc_t	write_desc;
} r2n_buf_t;

typedef struct r2n_wqe_ {
  uint64_t handle;
  uint32_t data_size;
  uint16_t opcode;
  uint16_t status;
} r2n_wqe_t;

dp_mem_t *r2n_buf_alloc();

void r2n_nvme_be_cmd_buf_init(dp_mem_t *nvme_cmd_buf, dp_mem_t *r2n_buf,
                              uint32_t src_queue_id, uint16_t ssd_handle, 
                              uint8_t io_priority, uint8_t is_read, 
                              uint8_t is_local, dp_mem_t **nvme_cmd_ptr);

void r2n_nvme_be_cmd_init(dp_mem_t *r2n_buf, uint32_t src_queue_id, uint16_t ssd_handle,
                          uint8_t io_priority, uint8_t is_read, uint8_t is_local);

dp_mem_t *r2n_nvme_cmd_ptr(dp_mem_t *r2n_buf);

void r2n_wqe_init(dp_mem_t *r2n_wqe_buf, dp_mem_t *r2n_buf, uint16_t opcode);

void r2n_wqe_db_update(dp_mem_t *r2n_wqe_buf, uint16_t lif, uint8_t qtype, 
                       uint32_t qid, uint16_t index);

}  // namespace r2n

#endif   // _R2N_HPP_
