#ifndef __REQ_RX_H
#define __REQ_RX_H
#include "capri.h"
#include "types.h"
#include "req_rx_args.h"
#include "INGRESS_p.h"
#include "ingress.h"
#include "common_phv.h"

#define REQ_RX_CQCB_ADDR_GET(_r, _cqid, _cqcb_base_addr_hi) \
    CQCB_ADDR_GET(_r, _cqid, _cqcb_base_addr_hi);

#define REQ_RX_EQCB_ADDR_GET(_r, _tmp_r, _eqid, _cqcb_base_addr_hi, _log_num_cq_entries) \
    EQCB_ADDR_GET(_r, _tmp_r, _eqid, _cqcb_base_addr_hi, _log_num_cq_entries);

#define REQ_RX_DMA_CMD_START_FLIT_ID       8 // flits 8-11 are used for dma cmds
#define REQ_RX_DMA_CMD_START               0
#define REQ_RX_DMA_CMD_MSN_CREDITS         0
#define REQ_RX_DMA_CMD_FC_DB               1
#define REQ_RX_DMA_CMD_REXMIT_PSN          2
#define REQ_RX_DMA_CMD_BKTRACK_DB          3
#define REQ_RX_DMA_CMD_RNR_TIMEOUT         4
#define REQ_RX_RDMA_PAYLOAD_DMA_CMDS_START 4
#define REQ_RX_MAX_DMA_CMDS                16
#define REQ_RX_DMA_CMD_CQ                  (REQ_RX_MAX_DMA_CMDS - 2)
#define REQ_RX_DMA_CMD_EQ                  (REQ_RX_MAX_DMA_CMDS - 1)
//wakeup dpath and EQ are mutually exclusive
#define REQ_RX_DMA_CMD_WAKEUP_DPATH        REQ_RX_DMA_CMD_EQ
#define REQ_RX_DMA_CMD_EQ_INTR             (REQ_RX_MAX_DMA_CMDS - 1)

// phv 
struct req_rx_phv_t {
    // flit 11-8
    // dma commands

    // scratch
    //
    //flit 7
    wakeup_dpath_data       : 64;
    rexmit_psn              : 24;
    ack_timestamp           : 48;
    err_retry_ctr           : 4;
    rnr_retry_ctr           : 4;
    rnr_timeout             : 8;
    rsvd3                   : (512 - 152);

    //flit 6
    rsvd2                   : 24;
    msn                     : 24;
    rsvd1                   : 3;
    credits                 : 5;
    db_data2                : 64;
    db_data1                : 64;
    eq_int_num              : 16;
    struct eqwqe_t eqwqe;
    struct cqwqe_t cqwqe;
    my_token_id             : 8;

    //flit 0-5
    // common rx
    struct phv_ common;
};

struct req_rx_phv_global_t {
    struct phv_global_common_t common;
};

// stage to stage argument structures

struct req_rx_s0_t {
    lif: 11;
    qtype: 3;
    qid: 24;
    struct p4_2_p4plus_app_hdr_t app_hdr;
};

#endif //__REQ_RX_H
