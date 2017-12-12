#ifndef _RDMA_HPP_
#define _RDMA_HPP_

#include "nic/include/base.h"
#include <memory>
#include <map>
#include "nic/hal/src/interface.hpp"
#include "nic/gen/proto/hal/rdma.pb.h"
// #include "rdma_svc.hpp"
#include "nic/utils/bm_allocator/bm_allocator.hpp"

using rdma::RdmaServiceType;
using rdma::RdmaQpSpec;
using rdma::RdmaQpRequestMsg;
using rdma::RdmaQpResponse;
using rdma::RdmaQpResponseMsg;
using rdma::RdmaQpUpdateSpec;
using rdma::RdmaQpUpdateRequestMsg;
using rdma::RdmaQpUpdateResponse;
using rdma::RdmaQpUpdateResponseMsg;
using rdma::RdmaQpUpdateOper;
using rdma::RdmaMemRegSpec;
using rdma::RdmaMemRegRequestMsg;
using rdma::RdmaMemRegResponse;
using rdma::RdmaMemRegResponseMsg;
using rdma::RdmaCqSpec;
using rdma::RdmaCqRequestMsg;
using rdma::RdmaCqResponse;
using rdma::RdmaCqResponseMsg;
using rdma::RdmaEqSpec;
using rdma::RdmaEqRequestMsg;
using rdma::RdmaEqResponse;
using rdma::RdmaEqResponseMsg;
using rdma::RdmaAhRequestMsg;
using rdma::RdmaAhResponseMsg;
using rdma::RdmaAhSpec;
using rdma::RdmaAhResponse;

namespace hal {

extern hal_ret_t rdma_hal_init();
extern hal_ret_t rdma_lif_init(intf::LifSpec& spec, uint32_t lif_id);
extern  hal_ret_t rdma_qp_create(RdmaQpSpec& spec, RdmaQpResponse *rsp);
extern  hal_ret_t rdma_qp_update(RdmaQpUpdateSpec& spec, RdmaQpUpdateResponse *rsp);
extern hal_ret_t rdma_cq_create (RdmaCqSpec& spec, RdmaCqResponse *rsp);
extern hal_ret_t rdma_eq_create (RdmaEqSpec& spec, RdmaEqResponse *rsp);
extern hal_ret_t rdma_memory_register(RdmaMemRegSpec& spec, RdmaMemRegResponse *rsp);
extern uint64_t rdma_lif_pt_base_addr(uint32_t lif_id);
extern uint64_t rdma_lif_kt_base_addr(uint32_t lif_id);
extern  hal_ret_t rdma_ah_create(RdmaAhSpec& spec, RdmaAhResponse *rsp);



class RDMAManager {
 public:
  RDMAManager();
  int32_t HbmAlloc(uint32_t size);

 protected:

 private:
  uint32_t hbm_base_;
  std::unique_ptr<BMAllocator> hbm_allocator_;

  // Track allocation size which are needed when we
  // free memory.
  std::map<uint32_t, uint32_t> allocation_sizes_;
};

#define HOSTMEM_PAGE_SIZE  (1 << 12)  //4096 Bytes
//#define MAX_LIFS           32
#define MAX_KEYS_PER_LIF   1024
#define R_KEY_BASE         (MAX_KEYS_PER_LIF / 2)

#define HBM_NUM_PT_ENTRIES_PER_CACHE_LINE 8
#define HBM_CACHE_LINE_SIZE 64
#define HBM_CACHE_LINE_MASK (HBM_CACHE_LINE_SIZE - 1)

#define MAX_KEYS_PER_LIF 1024
#define R_KEY_BASE (MAX_KEYS_PER_LIF / 2)
#define MAX_PT_ENTRIES_PER_LIF 1024

#define MAX_PMTU 1024

#define MAX_PKT_SIZE (10*1024)
#define INT_TABLE_SIZE (4 * 1024)

#define CRQ_PT_SIZE 12

#define PACKED __attribute__((__packed__))

#define NUM_SGES_IN_BASE 2
#define HBM_NUM_SGES_PER_CACHELINE 4
#define NUM_SGE_BYTES 16
#define MAX_SGE_PER_PASS 2

#define HBM_PAGE_SIZE 4096
#define HBM_PAGE_SIZE_SHIFT 12

typedef struct sram_lif_entry_s {
    uint32_t rdma_en_qtype_mask:8;
    uint32_t pt_base_addr_page_id:20;
    uint32_t log_num_pt_entries:7;

    uint32_t cqcb_base_addr_page_id:20;
    uint32_t log_num_cq_entries:5;

    uint32_t prefetch_pool_base_addr_page_id:20;
    uint32_t log_num_prefetch_pool_entries:5;
} PACKED sram_lif_entry_t;

typedef struct lif_init_attr_s {
    uint32_t max_qps;
    //uint32_t max_srqs;
    uint32_t max_cqs;
    uint32_t max_eqs;
    uint32_t max_pt_entries;
    uint32_t max_keys;
} lif_init_attr_t;

typedef enum q_type_s {
    Q_TYPE_SQ,
    Q_TYPE_RQ,
    Q_TYPE_TXQ,
    Q_TYPE_ADMINQ,
    Q_TYPE_RXQ,
    Q_TYPE_CQ,
    Q_TYPE_EQ,
    Q_TYPE_MAX
} q_type_t;



#define ACC_CTRL_LOCAL_WRITE        0x1
#define ACC_CTRL_REMOTE_WRITE       0x2
#define ACC_CTRL_REMOTE_READ        0x4
#define ACC_CTRL_REMOTE_ATOMIC      0x8

#define INVALID_KEY                 0xFFFFFF //24b

typedef enum key_state_e {
    KEY_STATE_INVALID,
    KEY_STATE_FREE,
    KEY_STATE_VALID,
} key_state_t;

#define KEY_INDEX_MASK      0x00ffffff
#define KEY_INDEX_SHIFT     0
#define KEY_USER_KEY_MASK   0xff000000
#define KEY_USER_KEY_SHIFT  24

typedef enum mr_type_s {
    MR_TYPE_MR_INVALID = 0,
    MR_TYPE_MR = 1,
    MR_TYPE_MW_TYPE_1 = 2,
    MR_TYPE_MW_TYPE_2A = 3,
    MR_TYPE_MW_TYPE_2B = 4,
} mr_type_t;

#define MR_FLAG_MW_EN   0x1 // is memory window enabled ?
#define MR_FLAG_INV_EN  0x2 // is memory invalidation enabled ?
#define MR_FLAG_ZBVA    0x4 //is it a zbva ?
#define MR_FLAG_UKEY_EN 0x8 //user-key is enabled

typedef struct qpcb_ring_s {
    uint16_t  c_index;
    uint16_t  p_index;
} qpcb_ring_t;

typedef struct qpcb_intrinsic_s {
    uint16_t  pid;
    uint8_t   total_rings:4;
    uint8_t   host_rings:4;
    uint8_t   eval_last;
    uint8_t   cos_selector;
    uint8_t   cosB:4;
    uint8_t   cosA:4;
    uint8_t   rsvd;
    uint8_t   pc;
} PACKED qpcb_intrinsic_base_t;

typedef struct p4_to_p4plus_rdma_hdr_s {
    uint16_t         flags; //roce_opcode_flags_t
    uint8_t          v4:1;
    union {
        uint32_t         v4;
        uint32_t         v6[4];
    }dip;
    uint16_t         payload_size;
} p4_to_p4plus_rdma_hdr_t;

typedef struct key_entry_s {
    uint32_t         qp: 24; //qp which bound the MW ?
    uint8_t          flags;
    uint32_t         rsvd: 19;
    uint32_t         override_lif: 12;
    uint32_t         override_lif_vld: 1;
    uint32_t         pd;
    uint32_t         pt_base;
    uint64_t         base_va;
    uint32_t         len;
    uint8_t          log_page_size;
    uint8_t          acc_ctrl;
    uint8_t          type  : 4; //mr_type_t
    uint8_t          state : 4;
    uint8_t          user_key;
} PACKED key_entry_t;

   
/*====================  TYPES.H ===================*/


#define BYTES_TO_BITS(__B) ((__B)*8)
#define BITS_TO_BYTES(__b) ((__b)/8)

#define TEST_RC_NO_ERROR		1
#define TEST_RC_FAILED_ATTEMPTS		2

#define IS_ACK_PACKET(phv_p) \
    (((phv_p)->aeth.syndrome & 0x60) == 0x0)

#define IS_RNR_PACKET(phv_p) \
    (((phv_p)->aeth.syndrome & 0x60) == 0x20)

#define IS_NAK_PACKET(phv_p) \
    (((phv_p)->aeth.syndrome & 0x60) == 0x60)

#define IS_PSN_SEQ_ERR_NAK(syndrome) \
     ((syndrome & 0x1f) == 0x0)

extern uint8_t error_code;

typedef enum qp_state_e {
    QP_STATE_RESET,
    QP_STATE_INIT,
    QP_STATE_RTR,
    QP_STATE_RTS,
    QP_STATE_SQD,
    QP_STATE_SQERR,
    QP_STATE_ERR,
} qp_state_t;

#if 0
typedef struct qpcb_s {
    uint8_t  state;
    uint16_t mtu;
    uint8_t  rnr_retry_time;
    uint8_t  rnr_retry_count;
    uint8_t  rnr_curr_retry_count;
    uint8_t  ack_retry_time;
    uint8_t  ack_retry_count;
    uint8_t  ack_curr_retry_count;
    uint8_t  busy: 1;
    uint8_t  in_progress: 1;
    uint8_t  disable_e2e_fc: 1;
    uint32_t pd;
    uint32_t xrcd;
    uint32_t rdd; 
    uint32_t tx_psn;
    uint32_t ack_psn;
    uint32_t rx_epsn;
    uint32_t rx_ack_psn;
    uint32_t ssn;
    uint32_t rx_msn;
    uint32_t rx_credits;
    uint32_t lsn;
    uint8_t  num_rdma_reads;
    uint32_t tx_cq;
    uint32_t rx_cq;
    uint64_t sq_base;
    uint32_t tx_cindex;
    uint32_t ack_cindex;
    uint64_t rq_base;
    uint32_t rx_cindex;
} PACKED qpcb_t;
#endif

typedef enum optype_s {
    OP_TYPE_SEND,
    OP_TYPE_SEND_INV,
    OP_TYPE_SEND_IMM,
    OP_TYPE_READ,
    OP_TYPE_WRITE,
    OP_TYPE_WRITE_IMM,
    OP_TYPE_CMP_N_SWAP,
    OP_TYPE_FETCH_N_ADD,
    OP_TYPE_FRPMR,
    OP_TYPE_LOCAL_INV,
    OP_TYPE_BIND_MW,
    OP_TYPE_SEND_INV_IMM, //vendor specific - needed for storage folks
    // Performance related changes:
    // TODO: Add a way to specify data as part of WQE itself
    // Add a way to specify WQE as part of doorbell itself
    //cq optypes
    OP_TYPE_RDMA_OPER_WITH_IMM = 16,
    OP_TYPE_SEND_RCVD,
    OP_TYPE_INVALID,
} optype_t;

typedef enum nak_code_e {
    NAK_CODE_SEQ_ERR = 0,
    NAK_CODE_INV_REQ = 1,
    NAK_CODE_REM_ACC_ERR = 2,
    NAK_CODE_REM_OP_ERR = 3,
    NAK_CODE_INV_RD_REQ = 4,
    NAK_CODE_INVALID = 31,
} nak_code_t;

typedef enum aeth_code_e {
    AETH_CODE_ACK = 0,
    AETH_CODE_RNR = 1,
    AETH_CODE_RSVD = 2,
    AETH_CODE_NAK = 3,
} aeth_code_t;

#define SYNDROME_GET(__a, __b) (((__a) << 5) | (__b))

typedef struct sge_s {
    uint32_t l_key;
    uint32_t len;
    uint64_t va;
} sge_t;

#define RDMA_ATOMIC_DATA_SIZE 8

typedef struct op_atomic_s {
    uint32_t     r_key;
    uint64_t     va;
    uint64_t     cmp_data;
    uint64_t     swap_or_add_data;
    sge_t   sge;
    uint64_t     rsvd;
} PACKED op_atomic_t;

typedef struct op_bind_mem_win_s {
    uint32_t len;
    uint64_t va;
    uint32_t l_key;
    uint32_t r_key;
    uint8_t  new_r_key_key;
    uint8_t  access_ctrl;
    uint16_t rsvd1;
    uint32_t rsvd2;
    uint64_t rsvd3[3];
} PACKED op_bind_mem_win_t;

typedef struct op_fast_reg_s {
    uint32_t len;
    uint64_t va;
    uint32_t l_key;
    uint32_t r_key;
    uint8_t  key;
    uint8_t  access_ctrl;
    uint16_t rsvd;
    uint32_t num_pages;
    uint64_t pages[0];
} PACKED op_fast_reg_t;

typedef struct op_local_inv_s {
    uint32_t l_key;
    uint32_t r_key;
    uint32_t rsvd1;
    uint64_t rsvd2[5];
    //uint8_t  fence: 1; // do we need separate fence bit for local invalidate ?
} PACKED op_local_inv_t;

typedef struct op_send_s {
    uint32_t     imm_data_or_inv_key;
    uint32_t     length;
    uint32_t     rsvd[3];    // for now
    uint32_t     sge_len_arr[8];
    sge_t   sge_list[0];
} PACKED op_send_t;

typedef struct op_write_s {
    uint32_t     imm_data;
    uint64_t     va;
    uint32_t     len;
    uint32_t     r_key;
    uint32_t     sge_len_arr[8];
    sge_t   sge_list[0];
} PACKED op_write_t;

typedef struct op_read_s {
    uint32_t     rsvd1;
    uint64_t     va;
    uint32_t     len;
    uint32_t     r_key;
    uint32_t     sge_len_arr[8];
    sge_t   sge_list[0];
} PACKED op_read_t;

typedef struct sqwqe_s {
    uint64_t     wrid; // other option to let driver take care of this
    uint8_t      op_type: 4;
    uint8_t      complete_notify: 1;
    uint8_t      fence: 1;
    uint8_t      solicited_event: 1;
    uint8_t      rsvd1:1;
    uint8_t      num_sges;
    uint16_t     rsvd2;

    union {
        op_send_t           send;
        op_write_t          write;
        op_read_t           read;
        op_atomic_t         atomic;
        op_bind_mem_win_t   bind;
        op_fast_reg_t       fast_reg;
        op_local_inv_t      local_inv;
    } op;
} PACKED sqwqe_t;

typedef struct sqwqe_base_s {
    uint64_t     wrid; // other option to let driver take care of this
    uint8_t      op_type: 4;
    uint8_t      complete_notify: 1;
    uint8_t      fence: 1;
    uint8_t      solicited_event: 1;
    uint8_t      rsvd1:1;
    uint8_t      num_sges;
    uint16_t     rsvd2;
} PACKED sqwqe_base_t;

typedef struct sge_short_s {
    uint64_t va;
    uint32_t l_key;
} PACKED sge_short_t;

typedef struct sge_short_zbva_s {
    uint32_t va;
    uint32_t l_lkey;
} PACKED sge_short_zbva_t;

typedef struct sge_list2_s {
    uint32_t         len[2];
    sge_short_t sge_list[2];
} PACKED sge_list2_t;

typedef struct sge_list14_s {
    uint32_t         len[14];
    sge_short_t sge_list[14];
} PACKED sge_list14_t;

typedef struct sge_list30_s {
    uint32_t         len[30];
    sge_short_t sge_list[30];
} PACKED sge_list30_t;

typedef struct sge_list_zbva_2_s {
    uint64_t                 rsvd;
    uint32_t                 len[2];
    sge_short_zbva_t    sge_list[2];
} PACKED sge_list_zbva_2_t;

typedef struct sge_list_zbva_8_s {
    uint32_t                 len[8];
    sge_short_zbva_t    sge_list[8];
} PACKED sge_list_zbva_8_t;

typedef struct sge_list_zbva_16_s {
    uint64_t                 rsvd[4];
    uint32_t                 len[16];
    sge_short_zbva_t    sge_list[16];
} PACKED sge_list_zbva_16_t;

// send
typedef struct sqwqe_send_s {
    uint32_t     imm_data;
    uint32_t     inv_key;
    uint32_t     length;
    uint32_t     rsvd[2];    // for now
} PACKED sqwqe_send_t;

typedef struct sqwqe_write_s {
    uint32_t     imm_data;
    uint64_t     va;
    uint32_t     len;
    uint32_t     r_key;
} PACKED sqwqe_write_t;

typedef struct sqwqe_read_s {
    uint32_t     rsvd1;
    uint64_t     va;
    uint32_t     len;
    uint32_t     r_key;
} PACKED sqwqe_read_t;

typedef struct sqwqe_atomic_s {
    sge_t   sge;
    uint32_t     r_key;
    uint64_t     va;
    uint64_t     cmp_data;
    uint64_t     swap_or_add_data;
    uint64_t     rsvd;
} PACKED sqwqe_atomic_t;

typedef struct sqwqe_bind_mw_s {
    uint32_t len;
    uint64_t va;
    uint32_t l_key;
    uint32_t r_key;
    uint8_t  new_r_key_key;
    uint8_t  access_ctrl;
    uint16_t zbva: 1;
    uint16_t rsvd1 : 15;
    uint32_t rsvd2;
    uint64_t rsvd3[3];
} PACKED sqwqe_bind_mw_t;

typedef struct sqwqe_fast_reg_s {
    uint32_t len;
    uint64_t va;
    uint32_t l_key;
    uint32_t r_key;
    uint8_t  key;
    uint8_t  access_ctrl;
    uint16_t rsvd;
    uint32_t num_pages;
    uint64_t page_table_ptr; // pointer to page table in host memory which can be 
                        // DMA'd directly. it is the driver's responsiblity to
                        // allocate physically contiguous pages for this 
                        // page table.
    // TODO: instead of page_table_ptr, accommodate minimal number of addresses
    //       directly as part of WQE itself
    uint64_t rsvd2[2];
} PACKED sqwqe_fast_reg_t;

typedef struct sqwqe_local_inv_s {
    uint32_t l_key;
    uint32_t r_key;
    uint32_t rsvd1;
    uint64_t rsvd2[5];
    //uint8_t  fence: 1; // do we need separate fence bit for local invalidate ?
} PACKED sqwqe_local_inv_t;

// NEW FORMATS END

typedef struct rrqwqe_read_s {
    uint32_t     len;
    uint64_t     wqe_sge_list_ptr;
    uint8_t      rsvd1[44];
} PACKED rrqwqe_read_t;

typedef struct rrqwqe_atomic_s {
    sge_t  sge;
    uint32_t    op_type:4;
    uint32_t    rsvd:28;
    uint8_t     rsvd1[40];
} PACKED rrqwqe_atomic_t;

typedef struct rrqwqe_s {
    uint32_t read_resp_or_atomic: 1;
    uint32_t num_sges: 7;
    uint32_t psn: 24;
    uint32_t msn:24;
    uint32_t rsvd:8;
    union {
        rrqwqe_read_t   read;
        rrqwqe_atomic_t atomic;
    } op;
} PACKED rrqwqe_t;

typedef enum rdma_serv_type_e {
    RDMA_SERV_TYPE_RC,
    RDMA_SERV_TYPE_UC,
    RDMA_SERV_TYPE_RD,
    RDMA_SERV_TYPE_UD,
    RDMA_SERV_TYPE_CNP,
    RDMA_SERV_TYPE_XRC,
    RDMA_SERV_TYPE_VEND_1,
    RDMA_SERV_TYPE_VEND_2,
} rdma_serv_type_t;

#if 0
typedef struct rqwqe_s {
    uint64_t     wrid;
    uint8_t      num_sges;
    uint8_t      rsvd[3];
    uint32_t     rsvd2;
    uint32_t     sge_len_arr[8];
    sge_t   sge_list[0];
} PACKED rqwqe_t;
#endif

typedef struct rqwqe_base_s {
    uint64_t     rsvd2[2];
    uint8_t      rsvd1[7];
    uint8_t      num_sges;
    uint64_t     wrid;
} PACKED rqwqe_base_t;

typedef struct rqwqe_s {
    sge_t        sge1;
    sge_t        sge0;
    rqwqe_base_t base;
} PACKED rqwqe_t;

typedef struct rsqwqe_read_s {
    uint32_t r_key;
    uint64_t va;
    uint32_t len;
    uint32_t offset;
} PACKED rsqwqe_read_t;

typedef struct rsqwqe_atomic_s {
    uint32_t r_key;
    uint64_t va;
    uint64_t orig_data;
} PACKED rsqwqe_atomic_t;

#define RSQ_OP_TYPE_READ 0
#define RSQ_OP_TYPE_ATOMIC 1

typedef struct rsqwqe_s {
    uint32_t read_or_atomic: 1;
    uint32_t ready:1;
    uint32_t rsvd: 6;
    uint32_t psn: 24;
    uint64_t rsvd2; // to keep RSQWQE 32 bytes
    union {
        rsqwqe_read_t   read;
        rsqwqe_atomic_t atomic;
    } op;
} PACKED rsqwqe_t;

typedef rqwqe_base_t srqwqe_base_t;

typedef enum cq_status_e {
    CQ_STATUS_SUCCESS,
    CQ_STATUS_LOCAL_LEN_ERR,
    CQ_STATUS_LOCAL_QP_OPER_ERR,
    CQ_STATUS_LOCAL_PROT_ERR,
    CQ_STATUS_WQE_FLUSHED_ERR,
    CQ_STATUS_MEM_MGMT_OPER_ERR,
    CQ_STATUS_BAD_RESP_ERR,
    CQ_STATUS_LOCAL_ACC_ERR,
    CQ_STATUS_REMOTE_INV_REQ_ERR,
    CQ_STATUS_REMOTE_ACC_ERR,
    CQ_STATUS_REMOTE_OPER_ERR,
    CQ_STATUS_RETRY_EXCEEDED,
    CQ_STATUS_RNR_RETRY_EXCEEDED,
    CQ_STATUS_XRC_VIO_ERR,
} cq_status_t;

typedef struct rdma_bth_s {
    uint32_t opcode  : 8;
    uint32_t se      : 1;
    uint32_t m       : 1;
    uint32_t pad     : 2;
    uint32_t tver    : 4;
    uint32_t pkey    : 16;
    uint32_t f       : 1;
    uint32_t b       : 1;
    uint32_t rsvd1   : 6;
    uint32_t dst_qp  : 24;
    uint32_t a       : 1;
    uint32_t rsvd2   : 7;
    uint32_t psn     : 24;
} PACKED rdma_bth_t;

typedef struct rdma_rdeth_s {
    uint32_t rsvd    : 8;
    uint32_t ee_ctxt : 24;
} PACKED rdma_rdeth_t;

typedef struct rdma_deth_s {
    uint64_t q_key   : 32;
    uint64_t rsvd    : 8;
    uint64_t src_qp  : 24;
} PACKED rdma_deth_t;

typedef struct rdma_reth_s {
    uint64_t va      : 64;
    uint64_t r_key   : 32;
    uint64_t dma_len : 32;
} PACKED rdma_reth_t;

typedef struct rdma_ieth_s {
    uint32_t rkey    : 32;
} PACKED rdma_ieth_t;

typedef struct rdma_atomiceth_s {
    uint64_t va                 : 64;
    uint64_t r_key              : 32;
    uint64_t swap_or_add_data   : 64;
    uint64_t cmp_data           : 64;
} PACKED rdma_atomiceth_t;

typedef struct rdma_aeth_s {
    uint32_t syndrome    : 8;
    uint32_t msn         : 24;
} PACKED rdma_aeth_t;

typedef struct rdma_atomicaeth_s {
    uint64_t orig_data;
} PACKED rdma_atomicaeth_t;

typedef struct raeth_atom_aeth_s {
    rdma_aeth_t        aeth;
    rdma_atomicaeth_t  atom_aeth;
} PACKED raeth_atom_aeth_t;

typedef struct rdma_immeth_s {
    uint32_t data;
} PACKED rdma_immeth_t;

typedef struct rdma_xrceth_s {
    uint32_t rsvd    : 8;
    uint32_t xrcsrq  : 24;
} PACKED rdma_xrceth_t;

typedef union rdma_ext_s {

    rdma_immeth_t immeth;

    rdma_reth_t   reth;

    struct {
        rdma_reth_t   reth;
        rdma_immeth_t immeth;
    } reth_immeth;

    rdma_aeth_t   aeth;

    rdma_atomicaeth_t atom_aeth;
        
    rdma_atomiceth_t  atom_eth;

    raeth_atom_aeth_t aeth_atom_aeth;
    
    rdma_ieth_t       ieth;

    rdma_rdeth_t      rdeth;

    struct {
        rdma_rdeth_t  rdeth;
        rdma_deth_t   deth;
    } rdeth_deth;

    struct {
        rdma_rdeth_t  rdeth;
        rdma_deth_t   deth;
        rdma_immeth_t immeth;
    } rdeth_deth_immeth;

    struct {
        rdma_rdeth_t  rdeth;
        rdma_deth_t   deth;
        rdma_reth_t   reth;
    } rdeth_deth_reth;

    struct {
        rdma_rdeth_t  rdeth;
        rdma_deth_t   deth;
        rdma_reth_t   reth;
        rdma_immeth_t immeth;
    } rdeth_deth_reth_immeth;

    struct {
        rdma_rdeth_t  rdeth;
        rdma_aeth_t   aeth;
    } rdeth_aeth;

    struct {
        rdma_rdeth_t  rdeth;
        rdma_aeth_t   aeth;
        rdma_atomiceth_t  atom_eth;
    } rdeth_aeth_atom_eth;

    struct {
        rdma_rdeth_t  rdeth;
        rdma_atomiceth_t  atom_eth;
    } rdeth_atom_eth;

    rdma_deth_t       deth;

    struct {
        rdma_deth_t   deth;
        rdma_immeth_t immeth;
    } deth_immeth;

} PACKED rdma_ext_t;
    
typedef struct rdma_hdr_s {
    rdma_bth_t bth;
    rdma_ext_t ext;
} PACKED rdma_hdr_t;

typedef struct udphdr_s {
    uint16_t   csum;
    uint16_t   length;
    uint16_t   dport;
    uint16_t   sport;
} PACKED udphdr_t;

typedef struct iphdr_s {
    ipv4_addr_t daddr;
    ipv4_addr_t saddr;
    uint16_t   check;
    uint8_t    protocol;
    uint8_t    ttl;
    uint16_t   frag_off;
    uint16_t   id;
    uint16_t   tot_len;
    uint8_t    tos;
    uint8_t    ihl:4;
    uint8_t    version:4;
/*The options start here. */
} PACKED iphdr_t;

typedef struct ipv6hdr_s {
    ipv6_addr_t  daddr;
    ipv6_addr_t  saddr;
    uint32_t   hop_limit:8;
    uint32_t   nh:8;
    uint32_t   payload_len:16;
    uint32_t   flow_label:20;
    uint32_t   tc:8;
    uint32_t   version:4;;
} PACKED ipv6hdr_t;


#define MAC_SIZE 6

typedef struct ethhdr_s {
    uint16_t  ethertype;
    uint8_t   smac[MAC_SIZE];
    uint8_t   dmac[MAC_SIZE];
} PACKED ethhdr_t;

typedef struct vlanhdr_s {
    uint16_t ethertype;
    uint16_t vlan: 12;
    uint16_t cfi: 1;
    uint16_t pri: 3;
} PACKED vlanhdr_t;

typedef struct rocev2_pkt_s {
    ethhdr_t eth;
    vlanhdr_t vlan;
    iphdr_t  ip;
    udphdr_t udp;
    rdma_bth_t bth;
    uint8_t         rest[0];
} PACKED rocev2_pkt_t;

typedef enum rdma_pkt_opc_e {
    RDMA_PKT_OPC_SEND_FIRST = 0x0,
    RDMA_PKT_OPC_SEND_MIDDLE = 0x1,
    RDMA_PKT_OPC_SEND_LAST = 0x2,
    RDMA_PKT_OPC_SEND_LAST_WITH_IMM = 0x3,
    RDMA_PKT_OPC_SEND_ONLY = 0x4,
    RDMA_PKT_OPC_SEND_ONLY_WITH_IMM = 0x5,
    RDMA_PKT_OPC_RDMA_WRITE_FIRST = 0x6,
    RDMA_PKT_OPC_RDMA_WRITE_MIDDLE = 0x7,
    RDMA_PKT_OPC_RDMA_WRITE_LAST = 0x8,
    RDMA_PKT_OPC_RDMA_WRITE_LAST_WITH_IMM = 0x9,
    RDMA_PKT_OPC_RDMA_WRITE_ONLY = 0xa,
    RDMA_PKT_OPC_RDMA_WRITE_ONLY_WITH_IMM = 0xb,
    RDMA_PKT_OPC_RDMA_READ_REQ = 0xc,
    RDMA_PKT_OPC_RDMA_READ_RESP_FIRST = 0xd,
    RDMA_PKT_OPC_RDMA_READ_RESP_MID = 0xe,
    RDMA_PKT_OPC_RDMA_READ_RESP_LAST = 0xf,
    RDMA_PKT_OPC_RDMA_READ_RESP_ONLY = 0x10,
    RDMA_PKT_OPC_ACK = 0x11,
    RDMA_PKT_OPC_ATOMIC_ACK = 0x12,
    RDMA_PKT_OPC_CMP_SWAP = 0x13,
    RDMA_PKT_OPC_FETCH_ADD = 0x14,
    RDMA_PKT_OPC_RESYNC = 0x15,
    RDMA_PKT_OPC_SEND_LAST_WITH_INV = 0x16,
    RDMA_PKT_OPC_SEND_ONLY_WITH_INV = 0x17,
}rdma_rc_opc_t;
 
/*====================  TYPES.H ===================*/

//RRQ_RING is not actually monitored by scheduler
#define MAX_SQ_RINGS 6

#define SQ_RING_ID      RING_ID_0
#define FC_RING_ID      RING_ID_1
#define REXMIT_RING_ID  RING_ID_2
#define RRQ_RING_ID     (MAX_SQ_RINGS - 1)

typedef struct sqcb0_s {
    uint8_t  rsvd2;
    uint16_t spec_sq_cindex;
    uint8_t  cb1_byte;
    uint8_t  busy: 1;        //tx
    uint8_t  congestion_mgmt_enable: 1;
    uint8_t  rsvd0: 6;
    uint32_t pd;
    uint8_t  bktrack_in_progress:1;
    uint8_t  retry_timer_on:1;
    uint8_t  li_fence:1;
    uint8_t  fence:1;
    uint8_t  fast_reg_enable: 1; //tx
    uint8_t  disable_e2e_fc: 1;//tx
    uint8_t  signalled_completion: 1;//rx
    uint8_t  in_progress: 1; //tx
    uint8_t  num_sges;
    uint8_t  current_sge_id;
    uint32_t  current_sge_offset;
    uint64_t curr_wqe_ptr;
    uint8_t  curr_op_type;
    uint32_t service: 4;
    uint32_t log_num_wqes: 5;
    uint32_t log_wqe_size: 5;
    uint32_t log_sq_page_size:5;
    uint32_t log_pmtu:5;           //tx
    uint32_t pt_base_addr;          //common
    qpcb_ring_t           rings[MAX_SQ_RINGS];
    // intrinsic
    qpcb_intrinsic_base_t ring_header;
} PACKED sqcb0_t;

typedef struct sqcb1_s {
    uint8_t rsvd4[6];
    uint8_t header_template_size;
    uint8_t p4plus_to_p4_flags;
    uint32_t rsvd3:2;
    uint32_t err_retry_ctr:3;
    uint32_t nak_retry_ctr:3;
    uint32_t wqe_start_psn:24;
    uint32_t rexmit_psn:24;
    uint32_t e_rsp_psn:24;
    uint8_t  token_id;
    uint8_t  nxt_to_go_token_id;
    union {
        uint32_t header_template_addr;
        uint32_t q_key;
    };
    uint32_t rrqwqe_cur_sge_offset;
    uint8_t  rrqwqe_cur_sge_id;
    uint8_t  rrqwqe_num_sges;
    uint32_t rsvd2:5;
    uint32_t local_ack_timeout:5;
    uint32_t timer_active:1;
    uint32_t service:4;
    uint32_t in_progress:1;
    uint32_t credits:5;
    uint32_t rsvd1:3;
    uint32_t msn:24;
    uint32_t lsn:24;
    uint32_t ssn:24;
    uint32_t tx_psn:24;      //tx
    uint32_t dst_qp:24;      //tx
    uint32_t cq_id:24;       //rx
    uint8_t  log_rrq_size;
    uint32_t inv_key;
    uint32_t imm_data;
    uint32_t rrq_base_addr;  //common
    // transport related
} PACKED sqcb1_t;

typedef struct sqcb_s {
    sqcb0_t sqcb0; // 0-63 bytes
    sqcb1_t sqcb1; // 63-127 bytes
} PACKED sqcb_t;

typedef struct header_template_v4_s {
    udphdr_t    udp;
    iphdr_t     ip;
    vlanhdr_t   vlan;
    ethhdr_t    eth;
} PACKED header_template_v4_t;

typedef struct header_template_v6_s {
    udphdr_t    udp;
    ipv6hdr_t   ip;
    vlanhdr_t   vlan;
    ethhdr_t    eth;
} PACKED header_template_v6_t;

typedef union header_template_s {
    header_template_v4_t v4;
    header_template_v6_t v6;
} PACKED header_template_t;

#define RQ_RING_ID            RING_ID_0
#define RSQ_RING_ID           RING_ID_1
#define ACK_NAK_RING_ID       RING_ID_2
#define RSQ_BACKTRACK_RING_ID RING_ID_3 
#define CNP_RING_ID           RING_ID_4
#define MAX_RQ_RINGS          6

typedef struct rqcb0_s {
    uint16_t spec_cindex;

    uint16_t proxy_cindex;
    uint32_t pd;

    uint32_t msn:24;

    uint8_t  adjust_rsq_c_index;
    uint32_t e_psn: 24;

    uint32_t  cache: 1;
    uint32_t  rsq_quiesce: 1;
    uint32_t  adjust_rsq_c_index_in_progress: 1;
    uint32_t  rsvd1:3;
    uint32_t  in_progress: 1;
    uint32_t  busy: 1;
    uint32_t  srq_enabled: 1;
    uint32_t  serv_type: 3;
    uint32_t  log_num_wqes: 5;
    uint32_t  log_wqe_size: 5;
    uint32_t  log_rq_page_size: 5;
    uint32_t  log_pmtu: 5;

    uint16_t rsq_pindex_prime;
    uint8_t  nxt_to_go_token_id;
    uint8_t  token_id;
    uint8_t  log_rsq_size: 5;
    uint8_t  rsvd0:1;
    uint8_t  congestion_mgmt_enable:1;
    uint8_t  immdt_as_dbell:1;

    union {
        uint32_t rsq_base_addr;
        uint32_t q_key;
    };

    uint32_t pt_base_addr;

    qpcb_ring_t           rings[MAX_RQ_RINGS];

    // intrinsic
    qpcb_intrinsic_base_t ring_header;
} PACKED rqcb0_t;

//rqcb1_t is the 2nd 64B of rqcb
typedef struct rqcb1_s {
    uint8_t header_template_size;
    uint8_t p4plus_to_p4_flags;
    uint32_t current_sge_offset;
    uint8_t  num_sges;
    uint8_t  current_sge_id;
    uint64_t curr_wqe_ptr;
    uint32_t dst_qp:24;
    uint32_t header_template_addr;
    uint32_t last_ack_nak_psn:24;
    rdma_aeth_t aeth;
    uint32_t ack_nak_psn:24;
    uint32_t cur_read_rsp_psn:24;
    uint8_t dummy1:6;
    uint8_t read_rsp_lock:1;
    uint8_t read_rsp_in_progress:1;
    uint32_t cq_id:24;
    uint64_t wrid;
    //cached {va, len, r_key} from first packet of write req
    uint32_t r_key;
    uint32_t len;
    uint64_t va; 
} PACKED rqcb1_t;

//rqcb2_t is the 3rd 64B of rqcb
typedef struct rqcb2_s {
    qpcb_ring_t         cache_rq_ring;
    uint16_t            num_rqwqes_per_cpage;
    uint16_t            rqwqe_size;
    uint16_t            new_page_add_retry:2;
    uint16_t            page_chkout_retry:2;
    uint16_t            rsvd:12;
    uint32_t            cache_pt[CRQ_PT_SIZE];   
    uint8_t             rsvd1[6];
} PACKED rqcb2_t;


typedef struct rqcb4_s {
    uint8_t  rsvd[28];
    uint16_t max_pkts_in_any_msg;
    uint16_t num_pkts_in_cur_msg;
    uint16_t num_ring_dbell;
    uint16_t num_ack_requested;
    uint16_t num_write_msgs_imm_data;
    uint16_t num_send_msgs_imm_data;
    uint16_t num_send_msgs_inv_rkey;
    uint16_t num_atomic_cswap_msgs;
    uint16_t num_atomic_fna_msgs;
    uint16_t num_read_req_msgs;
    uint16_t num_write_msgs;
    uint16_t num_send_msgs;
    uint32_t num_pkts;
    uint64_t num_bytes;
} rqcb4_t;

typedef struct rqcb_s {
    rqcb0_t  rqcb0; // 0-63 bytes
    rqcb1_t  rqcb1; // 63-127 bytes
    rqcb2_t  rqcb2; // 63-127 bytes
    uint8_t  rsvd[64];
    rqcb4_t  rqcb4; // RESP_RX stats
} PACKED rqcb_t;

//dcqcn_cb_t dynamically allocated to store dcqcn related info in HBM
typedef struct dcqcn_cb_s {
    uint8_t             rsvd[31];
    uint64_t            cur_timestamp: 48; // For model testing only. Will be removed.
    uint8_t             num_sched_drop; // For model testing only. Number of times packet was scheduled
                                        // and dropped due to insufficient tokens. 
    uint64_t            cur_avail_tokens;
    uint64_t            last_sched_timestamp: 48;
    uint32_t            rate_enforced;
    uint16_t            partition_key;
    uint64_t            last_cnp_timestamp: 48;
} PACKED dcqcn_cb_t;


#define MAX_SRQ_RINGS 6

typedef struct srqcb_s {
    uint64_t base_addr;

    qpcb_ring_t           rings[MAX_SRQ_RINGS];

    // intrinsic
    qpcb_intrinsic_base_t ring_header;
} PACKED srqcb_t;

#define CQ_RING_ID  RING_ID_0
#define MAX_CQ_RINGS 1

typedef struct cqcb_s {
    uint8_t rsvd4[4];

    uint32_t rsvd3:24;
    uint32_t rsvd2:6;
    uint32_t color:1;
    uint32_t arm:1;

    uint32_t eq_id:24;
    uint32_t cq_num:24;
    uint32_t rsvd1:1;
    uint32_t log_num_wqes:5;
    uint32_t log_wqe_size:5;
    uint32_t log_cq_page_size:5;

    uint32_t pt_base_addr;

    qpcb_ring_t           rings[MAX_CQ_RINGS];

    // intrinsic
    qpcb_intrinsic_base_t ring_header;
} PACKED cqcb_t;

#define EQ_RING_ID  RING_ID_0
#define MAX_EQ_RINGS 1

typedef struct eqcb_s {
    uint32_t  rsvd: 28;
    uint32_t  color: 1;
    uint32_t  int_enabled: 1;
    uint32_t  log_wqe_size: 5;
    uint32_t  log_num_wqes: 5;
    uint32_t  eq_id:24;
    uint32_t  int_num;
    uint64_t  eqe_base_addr;

    qpcb_ring_t           rings[MAX_EQ_RINGS];
    // intrinsic
    qpcb_intrinsic_base_t ring_header;
} PACKED eqcb_t;

typedef struct mr_attr_s {
    uint32_t     pd;
    uint64_t     va;
    uint32_t     len;
    uint8_t      acc_ctrl;
    uint8_t      flags;
    uint32_t     lkey; //OUT
    uint32_t     rkey; //OUT
} mr_attr_t;

typedef struct mr_handle_s {
    void    *lkey_entry_p;
    void    *rkey_entry_p;
#if 0
    uint32_t     pd;
    uint64_t     va;
    uint32_t     len;
    uint8_t      acc_ctrl;
    uint8_t      flags;
    uint8_t      type;
    uint8_t      rsvd;
    uint32_t     lkey;
    uint32_t     rkey;
#endif
} mr_handle_t;

typedef struct mw_handle_s {
    uint32_t     pd;
} mw_handle_t;

typedef struct eq_handle_s {
    uint32_t                 eq; 
    uint32_t                 pd; 
    uint16_t                 lif;
    eqcb_t                   *eqcb_p;
    void                     *eq_p;
    uint32_t                 eqwqe_size;
} eq_handle_t;

typedef struct cq_handle_s {
    uint32_t                 cq; 
    uint32_t                 eq; 
    uint32_t                 pd; 
    uint16_t                 lif;
    cqcb_t                   *cqcb_p;
    void                     *cq_p;
    uint32_t                 cqwqe_size;
    eq_handle_t              *eq_handle_p;
} cq_handle_t;

static inline int 
get_cqid(cq_handle_t *handle_p)
{
    assert(handle_p != NULL);
    if (handle_p == NULL) {
        return (-1);
    }   
    return (handle_p->cq);
}

typedef struct qp_attr_s {
    uint8_t      num_sq_sges;
    uint8_t      num_rq_sges;
    uint8_t      fast_reg_enabled: 1;
    uint8_t      atomic_enabled: 1;
    uint8_t      srq_enabled: 1;
    uint8_t      signalled_completion_enabled: 1;
    uint8_t      service;
    uint32_t     pd; 
    uint32_t     srq;
    cq_handle_t *sq_cq;
    cq_handle_t *rq_cq;
    uint32_t     num_sq_wqes;
    uint32_t     num_rq_wqes;
    uint8_t      num_rrq_wqes;
    uint8_t      num_rsq_wqes;
    uint32_t     qp; //OUT
} qp_attr_t;

typedef struct cq_attr_s {
    uint32_t     pd;
    uint32_t     num_cq_entries;
    uint8_t      eq_enabled: 1;
    uint8_t      arm: 1;
    uint32_t     eq;
    eq_handle_t  *eq_handle_p;
    uint32_t     cq; //OUT
} cq_attr_t;

typedef struct eq_attr_s {
    uint32_t     pd;
    uint32_t     num_eq_entries;
    //uint32_t     int_num;
    uint32_t     eq; //OUT
} eq_attr_t;

typedef struct srq_attr_s {
    uint32_t     pd;
    uint32_t     num_wqes;
    uint8_t      num_sges;
    uint32_t     srq; //OUT
} srq_attr_t;

typedef struct modify_qp_attr_s {
    uint8_t      state;
    uint32_t     p_key;
    uint32_t     q_key;
    uint32_t     tx_psn;
    uint32_t     rx_psn;
    uint8_t      rx_rdma_read_enable: 1;
    uint8_t      rx_rdma_write_enable: 1;
    uint8_t      rx_atomic_enable: 1;
    uint32_t     dst_qp;
    uint8_t      rrq_size;
    uint8_t      rsq_size;
    uint8_t      rnr_nak_time;
    uint8_t      service_level;
    uint32_t     pmtu;
    uint32_t     sip;
    uint32_t     dip;
    uint8_t      retry_count;
    uint8_t      rnr_retry_count;
    uint8_t      local_ack_timeout;
} modify_qp_attr_t;

typedef struct alloc_lkey_attr_s {
    uint32_t     pd;
    uint32_t     num_pt_entries;
    uint8_t      need_rkey; // should be set to TRUE if rkey
                       // is needed
    uint32_t     lkey;   //OUT
    uint32_t     rkey;   //OUT
    uint32_t     actual_num_pt_entries; //OUT
} alloc_lkey_attr_t;

typedef enum mw_type_s {
    MW_TYPE_INVALID,
    MW_TYPE_1,
    MW_TYPE_2A,
    MW_TYPE_2B,
} mw_type_t;

typedef struct alloc_mw_attr_s {
    uint32_t     pd;
    uint8_t      type;
    uint32_t     rkey; //OUT
} alloc_mw_attr_t;

typedef struct qp_handle_s {
    uint32_t                 qp;
    uint32_t                 pd;
    uint16_t                 lif;
    //TODO: Remove these pointers, as they are stack varaiable during init code
    sqcb_t              *sqcb_p;
    rqcb_t              *rqcb_p;
    void                *sq_p;
    void                *rq_p;
    uint32_t            sqwqe_size;
    uint32_t            rqwqe_size;
    uint32_t            header_template_addr;
} qp_handle_t;


typedef enum ring_id_e {
    RING_ID_0,
    RING_ID_1,
    RING_ID_2,
    RING_ID_3,
    RING_ID_4,
    RING_ID_5,
    RING_ID_6,
    RING_ID_7,
} ring_id_t;

#define RING_P_INDEX_GET(__cb_p, __r_id) \
({\
    qpcb_intrinsic_base_t *i_p = (qpcb_intrinsic_base_t *)(__cb_p);\
    ((qpcb_ring_t*)(i_p+1))[(__r_id)].p_index;                     \
})

#define RING_C_INDEX_GET(__cb_p, __r_id) \
({\
    qpcb_intrinsic_base_t *i_p = (qpcb_intrinsic_base_t *)(__cb_p);\
    ((qpcb_ring_t*)(i_p+1))[(__r_id)].c_index;                     \
})

#define RING_C_INDEX_INCREMENT(__cb_p, __r_id) \
({\
    qpcb_intrinsic_base_t *i_p = (qpcb_intrinsic_base_t *)(__cb_p);\
    ((qpcb_ring_t*)(i_p+1))[(__r_id)].c_index++;                   \
})

#define RING_C_INDEX_SET(__cb_p, __r_id, __c) \
({\
    qpcb_intrinsic_base_t *i_p = (qpcb_intrinsic_base_t *)(__cb_p);\
    ((qpcb_ring_t*)(i_p+1))[(__r_id)].c_index = (__c);             \
})

#define RING_P_INDEX_SET(__cb_p, __r_id, __p) \
({\
    qpcb_intrinsic_base_t *i_p = (qpcb_intrinsic_base_t *)(__cb_p);\
    ((qpcb_ring_t*)(i_p+1))[(__r_id)].p_index = (__p);             \
})

#define RING_P_INDEX_INCREMENT(__cb_p, __r_id) \
({\
    qpcb_intrinsic_base_t *i_p = (qpcb_intrinsic_base_t *)(__cb_p);\
    ((qpcb_ring_t*)(i_p+1))[(__r_id)].p_index++;                   \
})



/*====================  VERBS.H ===================*/
#define QP_HEADER_TEMPLATE_GET(__qp_h_p) \
    (((qp_handle_t *)(__qp_h_p))->header_template_addr)
/*====================  VERBS.H ===================*/



}  // namespace hal

#endif // _RDMA_HPP_

