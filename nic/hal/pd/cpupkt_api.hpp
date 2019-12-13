#pragma once

#include "nic/include/base.hpp"
#include "nic/include/pd.hpp"
#include "nic/include/cpupkt_headers.hpp"
#include "nic/hal/src/internal/proxy.hpp"

namespace hal {
namespace pd {

#define MAX_CPU_PKT_QUEUES              (types::WRingType_ARRAYSIZE)
#define MAX_CPU_PKT_QUEUE_INST_INFO     MAX_PD_CPU_PKT_QUEUE_INST

#define CPUPKT_MAX_BATCH_SIZE           512

#define HAL_MAX_CPU_PKT_DESCR_ENTRIES   1024
#define CPU_PKT_DESCR_SIZE              128

#define CPU_PKT_SEM_CI_BATCH_SIZE       1

#define CPU_PKT_VALID_BIT_MASK          ((uint64_t)1 << 63)

#define HAL_MAX_CPU_PKT_PAGE_ENTRIES    1024
#define CPU_PKT_PAGE_SIZE               9216

#define CPU_ASQ_PID                     0
#define CPU_ASQ_QTYPE                   0
#define CPU_ASQ_QID                     0
#define CPU_SCHED_RING_ASQ              0


#define DB_ADDR_BASE                   0x800000
#define DB_ADDR_BASE_HOST              0x8400000
#define DB_UPD_SHFT                    17
#define DB_LIF_SHFT                    6
#define DB_TYPE_SHFT                   3

#define DB_PID_SHFT                    48
#define DB_QID_SHFT                    24
#define DB_RING_SHFT                   16

#define DB_IDX_UPD_NOP                 (0x0 << 2)
#define DB_IDX_UPD_CIDX_SET            (0x1 << 2)
#define DB_IDX_UPD_PIDX_SET            (0x2 << 2)
#define DB_IDX_UPD_PIDX_INC            (0x3 << 2)

#define DB_SCHED_UPD_NOP               (0x0)
#define DB_SCHED_UPD_EVAL              (0x1)
#define DB_SCHED_UPD_CLEAR             (0x2)
#define DB_SCHED_UPD_SET               (0x3)


typedef uint64_t  cpupkt_hw_id_t;
struct cpupkt_queue_info_s;
typedef struct cpupkt_queue_info_s cpupkt_queue_info_t;
typedef struct cpupkt_qinst_ctr_s {
    uint64_t        recv_pkts;
    uint64_t        send_pkts;
    uint64_t        num_errs;
    uint32_t        rx_sem_wr_err;
    uint32_t        rx_slot_value_read_err;
    uint32_t        rx_descr_read_err;
    uint32_t        rx_descr_to_hdr_err;
    uint32_t        rx_descr_free_err;
    uint32_t        tx_descr_free_err;
    uint32_t        tx_page_alloc_err;
    uint32_t        tx_page_copy_err;
    uint32_t        tx_descr_pgm_err;
    uint32_t        tx_send_err;
    uint32_t        tx_db_err;
    uint32_t        rx_descr_addr_oob;
     uint64_t       poll_count;
} __PACK__ cpupkt_qinst_ctr_t;

typedef struct cpupkt_qinst_info_s {
    uint32_t                queue_id;
    cpupkt_hw_id_t          base_addr;
    uint8_t                 *virt_base_addr; //mmap'ed virtual base address of the queue region
    uint32_t                pc_index;    //Producer index in case of TX queue / Consumer index in case of RXQ
    cpupkt_hw_id_t          pc_index_addr;
    uint8_t                 *virt_pc_index_addr; //mmap'ed virtual address of the queue index slot
    uint64_t                valid_bit_value;
    uint16_t                queue_tbl_shift;
    cpupkt_queue_info_t     *queue_info;
    cpupkt_qinst_ctr_t      ctr;
} cpupkt_qinst_info_t;

typedef struct cpupkt_queue_info_s {
    types::WRingType        type;
    pd_wring_meta_t         *wring_meta;
    uint32_t                num_qinst;
    cpupkt_qinst_info_t     *qinst_info[MAX_CPU_PKT_QUEUE_INST_INFO];
} cpupkt_queue_info_t;

typedef struct cpupkt_rx_ctxt_s {
    uint32_t                num_queues;
    cpupkt_queue_info_t     queue[MAX_CPU_PKT_QUEUES];
} __PACK__ cpupkt_rx_ctxt_t;

typedef struct cpupkt_tx_ctxt_s {
    cpupkt_queue_info_t     queue[MAX_CPU_PKT_QUEUES];
} __PACK__ cpupkt_tx_ctxt_t;

typedef struct cpupkt_ctxt_s {
    cpupkt_rx_ctxt_t rx;
    cpupkt_tx_ctxt_t tx;
} __PACK__ cpupkt_ctxt_t;

typedef struct cpupkt_pktinfo_s {
     p4_to_p4plus_cpu_pkt_t *cpu_rxhdr;
     uint8_t                *pkt;
     size_t                 pkt_len;
     bool                   copied_pkt;
} __PACK__ cpupkt_pktinfo_t;

typedef struct cpupkt_pkt_batch_s {
   cpupkt_pktinfo_t pkts[CPUPKT_MAX_BATCH_SIZE];
   uint16_t         pktcount;
} __PACK__ cpupkt_pkt_batch_t;

typedef struct cpupkt_send_pkt_s {
    cpupkt_ctxt_t* ctxt;
    types::WRingType type;
    uint32_t queue_id;
    cpu_to_p4plus_header_t* cpu_header;
    p4plus_to_p4_header_t* p4_header;
    uint8_t* data;
    size_t data_len;
    uint16_t dest_lif;
    uint8_t  qtype;
    uint32_t qid;
    uint8_t  ring_number;
} __PACK__ cpupkt_send_pkt_t;

typedef struct cpupkt_send_pkt_batch_s {
   cpupkt_send_pkt_t pkts[CPUPKT_MAX_BATCH_SIZE];
   uint16_t          pktcount;
} __PACK__ cpupkt_send_pkt_batch_t;

// Stats for cpupkt

typedef struct cpupkt_global_info_s {
    uint32_t       gc_pindex;
    uint32_t       cpu_tx_page_pindex;
    uint32_t       cpu_tx_page_cindex;
    uint32_t       cpu_tx_page_full_err;
    uint32_t       cpu_tx_descr_pindex;
    uint32_t       cpu_tx_descr_cindex;
    uint32_t       cpu_tx_descr_full_err;  
    uint32_t       cpu_rx_dpr_cindex;
    uint32_t       cpu_rx_dpr_sem_cindex;
    uint32_t       cpu_rx_dpr_descr_free_err;
    uint32_t       cpu_rx_dpr_sem_free_err;
    uint32_t       cpu_rx_dpr_descr_invalid_free_err;
} __PACK__ cpupkt_global_info_t;

typedef struct cpupkt_qinst_stats_s {
    cpupkt_qinst_ctr_t  ctr;
    uint32_t            queue_id;
    cpupkt_hw_id_t      base_addr;
    uint32_t            pc_index;
    cpupkt_hw_id_t      pc_index_addr;
} __PACK__ cpupkt_qinst_stats_t;

typedef struct cpupkt_qinfo_stats_s {
    types::WRingType       type;
    cpupkt_qinst_stats_t   inst;    // Today we have only one instance
} __PACK__ cpupkt_qinfo_stats_t;

static inline cpupkt_ctxt_t *
cpupkt_ctxt_init (cpupkt_ctxt_t *ctxt)
{
    if (!ctxt) {
        return NULL;
    }

    memset(ctxt, 0, sizeof(cpupkt_ctxt_t));
    return ctxt;
}

#if 0
cpupkt_ctxt_t* cpupkt_ctxt_alloc_init(void);
hal_ret_t cpupkt_register_rx_queue(cpupkt_ctxt_t* ctxt, types::WRingType type, uint32_t queue_id=0);
hal_ret_t cpupkt_register_tx_queue(cpupkt_ctxt_t* ctxt, types::WRingType type, uint32_t queue_id=0);
hal_ret_t cpupkt_unregister_tx_queue(cpupkt_ctxt_t* ctxt, types::WRingType type, uint32_t queue_id=0);

// receive
hal_ret_t cpupkt_poll_receive(cpupkt_ctxt_t* ctxt,
                              p4_to_p4plus_cpu_pkt_t** flow_miss_hdr,
                              uint8_t** data,
                              size_t* data_len, bool *copied_pkt);
hal_ret_t cpupkt_free(p4_to_p4plus_cpu_pkt_t* flow_miss_hdr, uint8_t* data);

// transmit pkt
hal_ret_t cpupkt_send(cpupkt_ctxt_t* ctxt,
                      types::WRingType type,
                      uint32_t queue_id,
                      cpu_to_p4plus_header_t* cpu_header,
                      p4plus_to_p4_header_t* p4_header,
                      uint8_t* data,
                      size_t data_len,
                      uint16_t dest_lif = HAL_LIF_CPU,
                      uint8_t  qtype = CPU_ASQ_QTYPE,
                      uint32_t qid = CPU_ASQ_QID,
                      uint8_t  ring_number = CPU_SCHED_RING_ASQ);

hal_ret_t cpupkt_page_alloc(cpupkt_hw_id_t* page_addr);
hal_ret_t cpupkt_descr_alloc(cpupkt_hw_id_t* descr_addr);
hal_ret_t cpupkt_descr_free(cpupkt_hw_id_t descr_addr);

hal_ret_t
cpupkt_program_send_ring_doorbell(uint16_t dest_lif,
                                  uint8_t  qtype,
                                  uint32_t qid,
                                  uint8_t  ring_number);

#endif

}    // namespace pd
}    // namespace hal
