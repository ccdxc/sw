#ifndef _RDMA_HPP_
#define _RDMA_HPP_

#include "dol/iris/test/storage/dp_mem.hpp"

using namespace dp_mem;

#define RDMA_OP_TYPE_SEND           0
#define RDMA_OP_TYPE_SEND_INV       1
#define RDMA_OP_TYPE_SEND_IMM       2
#define RDMA_OP_TYPE_READ           3
#define RDMA_OP_TYPE_WRITE          4
#define RDMA_OP_TYPE_WRITE_IMM      5
#define RDMA_OP_TYPE_CMP_N_SWAP     6
#define RDMA_OP_TYPE_FETCH_N_ADD    7
#define RDMA_OP_TYPE_FRPMR          8
#define RDMA_OP_TYPE_LOCAL_INV      9
#define RDMA_OP_TYPE_BIND_MW        10
#define RDMA_OP_TYPE_SEND_INV_IMM   11

#define IO_STATUS_BUF_BE_STATUS_OFFSET          64
#define IO_STATUS_BUF_BE_STATUS_SIZE            64

// Define the desired Send op_type;
// Valid choices are RDMA_OP_TYPE_SEND or RDMA_OP_TYPE_SEND_IMM
const uint8_t  kRdmaSendOpType = RDMA_OP_TYPE_SEND_IMM;

uint32_t rdma_r2n_data_size(void);
uint32_t rdma_r2n_data_offset(void);
uint32_t rdma_r2n_buf_size(void);

int rdma_init(bool dp_init);
void rdma_shutdown();

void rdma_uspace_test();

dp_mem_t *rdma_get_initiator_rcv_buf();
dp_mem_t *rdma_get_target_write_data_buf();

void PostTargetRcvBuf1();
void PostInitiatorRcvBuf1();

void IncrTargetRcvBufPtr();
void IncrInitiatorRcvBufPtr();

int rdma_roce_ini_sq_info(uint16_t *lif, uint8_t *qtype, uint32_t *qid, uint64_t *qaddr);
int rdma_roce_tgt_sq_info(uint16_t *lif, uint8_t *qtype, uint32_t *qid, uint64_t *qaddr);
int rdma_roce_ini_rq_info(uint16_t *lif, uint8_t *qtype, uint32_t *qid, uint64_t *qaddr, uint64_t *base_pa);
int rdma_roce_tgt_rq_info(uint16_t *lif, uint8_t *qtype, uint32_t *qid, uint64_t *qaddr, uint64_t *base_pa);
uint32_t get_rdma_pvm_roce_init_sq();
uint32_t get_rdma_pvm_roce_init_cq();
uint32_t get_rdma_pvm_roce_tgt_sq();
uint32_t get_rdma_pvm_roce_tgt_cq();

void RdmaMemRegister(uint64_t va, uint64_t pa, uint32_t len, uint32_t lkey,
                     uint32_t rkey, bool remote, bool is_host_mem);
void RdmaMemRegister(dp_mem_t *mem, uint32_t lkey, uint32_t rkey, bool remote);


int StartRoceWriteSeq(uint16_t ssd_handle, uint8_t byte_val, dp_mem_t **nvme_cmd_ptr, uint64_t slba, dp_mem_t **rolling_write_buf);
int StartRoceReadSeq(uint32_t seq_pdma_q, uint32_t seq_roce_q, uint16_t ssd_handle,
                     dp_mem_t **nvme_cmd_ptr, dp_mem_t **read_buf_ptr, uint64_t slba, 
                     uint8_t pdma_dst_lif_override, uint16_t pdma_dst_lif, uint32_t bdf);
int StartRoceWritePdmaPrefilled(uint16_t seq_pdma_q,
                                uint16_t seq_pdma_index,
                                uint16_t seq_roce_q,
                                uint16_t seq_roce_index,
                                dp_mem_t *seq_roce_desc,
                                dp_mem_t *r2n_buf);
int StartRoceReadWithNextLifQueue(uint16_t seq_roce_q,
                                  uint16_t seq_roce_index,
                                  dp_mem_t *seq_roce_desc,
                                  dp_mem_t *r2n_send_buf,
                                  dp_mem_t *r2n_write_buf,
                                  uint32_t data_len,
                                  uint32_t next_lif,
                                  uint32_t next_qtype,
                                  uint32_t next_qid);
void rdma_tmr_global_disable(void);

#endif  // _RDMA_HPP_
