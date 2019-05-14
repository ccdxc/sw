//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef _NVME_DPATH_H
#define _NVME_DPATH_H

#include "nic/sdk/platform/capri/capri_common.hpp"

#define HOSTMEM_PAGE_SIZE  (1 << 12)  //4096 Bytes
#define MAX_LIFS           2048

#define PACKED __attribute__((__packed__))

#define HBM_PAGE_SIZE 4096
#define HBM_PAGE_SIZE_SHIFT 12
#define HBM_PAGE_ALIGN(x) (((x) + (HBM_PAGE_SIZE - 1)) & \
                           ~(uint64_t)(HBM_PAGE_SIZE - 1))

typedef struct lif_init_attr_s {
    uint32_t max_sq;
    uint32_t max_cq;
    uint32_t max_ns;
    uint32_t max_sess;
} lif_init_attr_t;

//4 Bytes
typedef struct qstate_ring_s {
    uint16_t  c_index;
    uint16_t  p_index;
} qstate_ring_t;

//8 Bytes
typedef struct qstate_intrinsic_s {
    uint16_t  pid;
    uint8_t   host_rings:4; // Swapping host/total nibbles since it is accessed by doorbell-module in Little-Endian.
    uint8_t   total_rings:4;
    uint8_t   eval_last;
    uint8_t   cos_selector;
    uint8_t   cosA:4; // Swapping nibbles cosA/cosB since it is accessed by doorbell-module in Little-Endian.
    uint8_t   cosB:4;
    uint8_t   rsvd;
    uint8_t   pc;
} PACKED qstate_intrinsic_base_t;

/*====================  TYPES.H ===================*/

#define BYTES_TO_BITS(__B) ((__B)*8)
#define BITS_TO_BYTES(__b) ((__b)/8)

#define SQ_RING_ID      RING_ID_0

#define MAX_SQ_RINGS 1
#define MAX_SQ_HOST_RINGS 1

//64 Bytes
typedef struct nvme_sqcb_s {

    uint8_t  pad[37];

    uint32_t lif_ns_start: 16;
    uint32_t cq_id:16;
    uint32_t rsvd0: 7;
    uint32_t busy: 1;
    uint32_t ring_empty_sched_eval_done: 1;
    uint32_t log_num_wqes:5;
    uint32_t log_wqe_size:5;
    uint32_t log_host_page_size:5;

    uint64_t base_addr;

    qstate_ring_t           rings[MAX_SQ_RINGS];

    // intrinsic
    qstate_intrinsic_base_t ring_header;
} PACKED nvme_sqcb_t;


#define CQ_RING_ID      RING_ID_0

#define MAX_CQ_RINGS 1
#define MAX_CQ_HOST_RINGS 1

//64 Bytes
typedef struct nvme_cqcb_s {

    uint8_t  pad2[28];

    uint64_t pad: 51;
    uint64_t cq_full:1;
    uint64_t cq_full_hint:1;
    uint64_t color:1;
    uint64_t log_num_wqes:5;
    uint64_t log_wqe_size:5;

    uint64_t int_assert_addr;
    uint64_t base_addr;

    qstate_ring_t           rings[MAX_CQ_RINGS];

    // intrinsic
    qstate_intrinsic_base_t ring_header;
} PACKED nvme_cqcb_t;

typedef struct nvme_nscb_s {
    uint8_t valid_session_bitmap[32];
    uint8_t  pad[13];

    uint64_t sess_prodcb_start: 16;

    uint64_t num_outstanding_req:11;

    uint64_t rr_session_id_served:10;
    uint64_t num_sessions:11;

    uint64_t backend_ns_id:32;

    uint64_t log_lba_size:5;
    uint64_t rsvd0:1;
    uint64_t ns_active:1;
    uint64_t ns_valid:1;
    uint64_t ns_size;
} PACKED nvme_nscb_t;

static_assert(sizeof(nvme_nscb_t) == 64);

typedef struct nvme_txsessprodcb_s {

    uint8_t pad[28];

    uint64_t tcp_q_ci : 16;
    uint64_t tcp_q_pi : 16;

    uint64_t dgst_q_ci : 16;
    uint64_t dgst_q_pi : 16;

    uint64_t xts_q_ci : 16;
    uint64_t xts_q_pi : 16;

    uint64_t rsvd5: 16;
    uint64_t tcp_q_choke_counter: 8;
    uint64_t rsvd4: 1;
    uint64_t log_num_tcp_q_entries: 5;
    uint64_t tcp_q_base_addr: 34;

    uint64_t dgst_qid: 16;
    uint64_t dgst_q_choke_counter: 8;
    uint64_t rsvd2: 1;
    uint64_t log_num_dgst_q_entries: 5;
    uint64_t dgst_q_base_addr: 34;

    uint64_t xts_qid: 16;
    uint64_t xts_q_choke_counter: 8;
    uint64_t rsvd0: 1;
    uint64_t log_num_xts_q_entries: 5;
    uint64_t xts_q_base_addr: 34;

} PACKED nvme_txsessprodcb_t;

static_assert(sizeof(nvme_txsessprodcb_t) == 64);

#define nvme_rxsessprodcb_t nvme_txsessprodcb_t

#define SESSTX_PREXTS_RING_ID      RING_ID_0
#define SESSTX_POSTXTS_RING_ID      RING_ID_1

#define MAX_SESSXTSTX_RINGS 2
#define MAX_SESSXTSTX_HOST_RINGS 0

typedef struct nvme_sessxtstxcb_s {

    uint8_t pad[28];

    uint64_t sec_key_index: 32;
    uint64_t key_index: 32;

    uint64_t rsvd4: 7;
    uint64_t wb_r1_busy: 1;

    uint64_t rsvd3: 7;
    uint64_t r1_busy: 1;

    uint64_t nxt_lba_offset: 16;

    uint64_t rsvd2: 6;
    uint64_t r0_in_progress: 1;
    uint64_t wb_r0_busy: 1;

    uint64_t rsvd1: 7;
    uint64_t r0_busy: 1;   

    uint64_t rsvd0: 3;
    uint64_t ring_empty_sched_eval_done: 1;
    uint64_t log_lba_size: 5;
    uint64_t log_num_entries: 5;
    uint64_t base_addr: 34;
    
    qstate_ring_t           rings[MAX_SESSXTSTX_RINGS];

    // intrinsic
    qstate_intrinsic_base_t ring_header;
} PACKED nvme_sessxtstxcb_t;

static_assert(sizeof(nvme_sessxtstxcb_t) == 64);

#define MAX_SESSXTSRX_RINGS 2
#define MAX_SESSXTSRX_HOST_RINGS 0

#define nvme_sessxtsrxcb_t nvme_sessxtstxcb_t


#define SESSTX_PREDGST_RING_ID      RING_ID_0
#define SESSTX_POSTDGST_RING_ID     RING_ID_1

#define MAX_SESSDGSTTX_RINGS 2
#define MAX_SESSDGSTTX_HOST_RINGS 0

typedef struct nvme_sessdgsttxcb_s {

    uint8_t pad[38];

    uint64_t rsvd4: 7;
    uint64_t wb_r1_busy: 1;

    uint64_t rsvd3: 7;
    uint64_t r1_busy: 1;

    uint64_t rsvd2: 7;
    uint64_t wb_r0_busy: 1;

    uint64_t rsvd1: 7;
    uint64_t r0_busy: 1;   

    uint64_t rsvd0: 9;
    uint64_t log_num_entries: 5;
    uint64_t base_addr: 34;
    
    qstate_ring_t           rings[MAX_SESSDGSTTX_RINGS];

    // intrinsic
    qstate_intrinsic_base_t ring_header;
} PACKED nvme_sessdgsttxcb_t;

static_assert(sizeof(nvme_sessdgsttxcb_t) == 64);

#define MAX_SESSDGSTRX_RINGS 2
#define MAX_SESSDGSTRX_HOST_RINGS 0

#define nvme_sessdgstrxcb_t nvme_sessdgsttxcb_t

typedef struct nvme_resourcecb_s {

        uint8_t pad[40];

        // ring of free cmdids
        uint64_t cmdid_ring_choke_counter: 8;
        uint64_t cmdid_ring_rsvd: 3;
        uint64_t cmdid_ring_log_sz: 5;
        uint64_t cmdid_ring_ci: 16;
        uint64_t cmdid_ring_proxy_ci: 16;
        uint64_t cmdid_ring_pi: 16;

        // ring of free AOL descriptor pairs
        uint64_t aol_ring_choke_counter: 8;
        uint64_t aol_ring_rsvd: 3;
        uint64_t aol_ring_log_sz: 5;
        uint64_t aol_ring_ci: 16;
        uint64_t aol_ring_proxy_ci: 16;
        uint64_t aol_ring_pi: 16;

        // ring of free data pages
        uint64_t page_ring_choke_counter: 8;
        uint64_t page_ring_rsvd: 3;
        uint64_t page_ring_log_sz: 5;
        uint64_t page_ring_ci: 16;
        uint64_t page_ring_proxy_ci: 16;
        uint64_t page_ring_pi: 16;

} PACKED nvme_resourcecb_t;



typedef struct nvme_cmd_context_s {
    uint8_t pad[2048];
} nvme_cmd_context_t;

typedef struct nvme_cmd_context_ring_entry_s {
    uint16_t id;
} nvme_cmd_context_ring_entry_t;

typedef struct nvme_aol_s {
    uint8_t pad[128];
} nvme_aol_t;

typedef struct nvme_iv_s {
    uint8_t pad[16];
} nvme_iv_t;

typedef struct nvme_aol_ring_entry_s {
    uint16_t id;
} nvme_aol_ring_entry_t;

typedef struct nvme_tx_hwxtscb_s {
 
        uint8_t  pad[56];

        // hw ring towards HW(BARCO) Encrypt XTS Engine
        uint64_t ring_choke_counter: 8;
        uint64_t ring_rsvd: 3;
        uint64_t ring_log_sz: 5;
        uint64_t ring_ci: 16;
        uint64_t ring_proxy_ci: 16;
        uint64_t ring_pi: 16;
   
} nvme_tx_hwxtscb_t;

static_assert(sizeof(nvme_tx_hwxtscb_t) == 64);

// hw ring towards HW(BARCO) Decrypt XTS Engine
#define nvme_rx_hwxtscb_t nvme_tx_hwxtscb_t


typedef struct nvme_tx_hwdgstcb_s {
 
        uint8_t  pad[56];

        // hw ring towards HW(BARCO) Tx CRC32 Digest/Compress Engine
        uint64_t ring_choke_counter: 8;
        uint64_t ring_rsvd: 3;
        uint64_t ring_log_sz: 5;
        uint64_t ring_ci: 16;
        uint64_t ring_proxy_ci: 16;
        uint64_t ring_pi: 16;
   
} nvme_tx_hwdgstcb_t;

static_assert(sizeof(nvme_tx_hwdgstcb_t) == 64);

// hw ring towards HW(BARCO) Rx CRC32 Digest/Compress Engine
#define nvme_rx_hwdgstcb_t nvme_tx_hwdgstcb_t

#define NVME_TX_SESS_XTSQ_DEPTH 64
#define NVME_TX_SESS_XTSQ_ENTRY_SIZE sizeof(nvme_cmd_context_ring_entry_t)
#define NVME_TX_SESS_XTSQ_SIZE NVME_TX_SESS_XTSQ_DEPTH * NVME_TX_SESS_XTSQ_ENTRY_SIZE
#define NVME_TX_SESS_DGSTQ_DEPTH 64
#define NVME_TX_SESS_DGSTQ_ENTRY_SIZE sizeof(nvme_cmd_context_ring_entry_t)
#define NVME_TX_SESS_DGSTQ_SIZE NVME_TX_SESS_DGSTQ_DEPTH * NVME_TX_SESS_DGSTQ_ENTRY_SIZE
#define NVME_RX_SESS_XTSQ_DEPTH 64
#define NVME_RX_SESS_XTSQ_ENTRY_SIZE sizeof(nvme_cmd_context_ring_entry_t)
#define NVME_RX_SESS_XTSQ_SIZE NVME_RX_SESS_XTSQ_DEPTH * NVME_RX_SESS_XTSQ_ENTRY_SIZE
#define NVME_RX_SESS_DGSTQ_DEPTH 64
#define NVME_RX_SESS_DGSTQ_ENTRY_SIZE sizeof(nvme_cmd_context_ring_entry_t)
#define NVME_RX_SESS_DGSTQ_SIZE NVME_RX_SESS_DGSTQ_DEPTH * NVME_RX_SESS_DGSTQ_ENTRY_SIZE

//HBM organization of NVME data path tables

typedef enum nvme_dpath_ds_type_s {
    NVME_TYPE_NSCB = 0,
    NVME_TYPE_CMD_CONTEXT,
    NVME_TYPE_CMD_CONTEXT_RING,
    NVME_TYPE_TX_RESOURCECB,
    NVME_TYPE_RX_RESOURCECB,
    NVME_TYPE_TX_SESSPRODCB,
    NVME_TYPE_RX_SESSPRODCB,
    NVME_TYPE_TX_HWXTSCB,
    NVME_TYPE_RX_HWXTSCB,
    NVME_TYPE_TX_HWDGSTCB,
    NVME_TYPE_RX_HWDGSTCB,
    NVME_TYPE_TX_AOL,
    NVME_TYPE_TX_AOL_RING,
    NVME_TYPE_RX_AOL,
    NVME_TYPE_RX_AOL_RING,
    NVME_TYPE_TX_SESS_XTSQ,
    NVME_TYPE_TX_SESS_DGSTQ,
    NVME_TYPE_RX_SESS_XTSQ,
    NVME_TYPE_RX_SESS_DGSTQ,
    NVME_TYPE_TX_XTS_AOL_ARRAY,
    NVME_TYPE_TX_XTS_IV_ARRAY,
    NVME_TYPE_RX_XTS_AOL_ARRAY,
    NVME_TYPE_RX_XTS_IV_ARRAY,
    NVME_TYPE_MAX
} nvme_dpath_ds_type_t;
     
typedef struct nvme_hbm_alloc_info_s {
    int type;
    int size;
    int num_entries;
} nvme_hbm_alloc_info_t;
    
static nvme_hbm_alloc_info_t nvme_hbm_alloc_table[] = {
    {NVME_TYPE_NSCB, 512, sizeof(nvme_nscb_t)},
    {NVME_TYPE_CMD_CONTEXT, 512, sizeof(nvme_cmd_context_t)},
    {NVME_TYPE_CMD_CONTEXT_RING, 512, sizeof(nvme_cmd_context_ring_entry_t)},
    {NVME_TYPE_TX_RESOURCECB, 1, sizeof(nvme_resourcecb_t)},
    {NVME_TYPE_RX_RESOURCECB, 1, sizeof(nvme_resourcecb_t)},
    {NVME_TYPE_TX_SESSPRODCB, 512, sizeof(nvme_txsessprodcb_t)},
    {NVME_TYPE_RX_SESSPRODCB, 512, sizeof(nvme_rxsessprodcb_t)},
    {NVME_TYPE_TX_HWXTSCB, 1, sizeof(nvme_tx_hwxtscb_t)},
    {NVME_TYPE_RX_HWXTSCB, 1, sizeof(nvme_rx_hwxtscb_t)},
    {NVME_TYPE_TX_HWDGSTCB, 1, sizeof(nvme_tx_hwdgstcb_t)},
    {NVME_TYPE_RX_HWDGSTCB, 1, sizeof(nvme_rx_hwdgstcb_t)},
    {NVME_TYPE_TX_AOL, 512, sizeof(nvme_aol_t)},
    {NVME_TYPE_TX_AOL_RING, 512, sizeof(nvme_aol_ring_entry_t)},
    {NVME_TYPE_RX_AOL, 512, sizeof(nvme_aol_t)},
    {NVME_TYPE_RX_AOL_RING, 512, sizeof(nvme_aol_ring_entry_t)},
    {NVME_TYPE_TX_SESS_XTSQ, 512, NVME_TX_SESS_XTSQ_DEPTH * NVME_TX_SESS_XTSQ_ENTRY_SIZE},
    {NVME_TYPE_TX_SESS_DGSTQ, 512, NVME_TX_SESS_DGSTQ_DEPTH * NVME_TX_SESS_DGSTQ_ENTRY_SIZE},
    {NVME_TYPE_RX_SESS_XTSQ, 512, NVME_RX_SESS_XTSQ_DEPTH * NVME_RX_SESS_XTSQ_ENTRY_SIZE},
    {NVME_TYPE_RX_SESS_DGSTQ, 512, NVME_RX_SESS_DGSTQ_DEPTH * NVME_RX_SESS_DGSTQ_ENTRY_SIZE},
    //XXX: Dividing the resources by 16 for now. 
    {NVME_TYPE_TX_XTS_AOL_ARRAY, CAPRI_BARCO_XTS_RING_SLOTS/16, sizeof(nvme_aol_t)},
    {NVME_TYPE_TX_XTS_IV_ARRAY, CAPRI_BARCO_XTS_RING_SLOTS/16, sizeof(nvme_iv_t)},
    {NVME_TYPE_RX_XTS_AOL_ARRAY, CAPRI_BARCO_XTS_RING_SLOTS/16, sizeof(nvme_aol_t)},
    {NVME_TYPE_RX_XTS_IV_ARRAY, CAPRI_BARCO_XTS_RING_SLOTS/16, sizeof(nvme_iv_t)},
};

static inline int nvme_hbm_offset(int type) {
    int i;
    int offset = 0;
    for (i = 0; i < type; i++) {
        assert(i == nvme_hbm_alloc_table[i].type);
        offset += nvme_hbm_alloc_table[i].size * nvme_hbm_alloc_table[i].num_entries;
    }
    return (offset);
}

static inline int nvme_hbm_resource_max(int type) {
    assert(type < NVME_TYPE_MAX);
    return (nvme_hbm_alloc_table[type].size);
}

#endif // _NVME_DPATH_H
