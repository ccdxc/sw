#ifndef __REQ_RX_H
#define __REQ_RX_H
#include "capri.h"
#include "types.h"
#include "req_rx_args.h"
#include "rxdma_phv.h"
#include "common_phv.h"

// phv 
struct req_rx_phv_t {
    // dma commands

    // scratch

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

struct req_rx_s0_k_t {
    struct capri_intrinsic_s0_k_t intrinsic;
    struct req_rx_s0_t args;
};

#endif //__REQ_RX_H
