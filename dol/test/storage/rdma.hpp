#ifndef _RDMA_HPP_
#define _RDMA_HPP_

typedef struct __attribute__((packed)) sqwqe {
                               // Offsets
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

  uint8_t inline_data[32];     // 256
} sqwqe_t;

int rdma_init();

void rdma_uspace_test();

void rdma_seq_test();

#endif  // _RDMA_HPP_
