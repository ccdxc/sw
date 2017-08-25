#ifndef __RESP_TX_H
#define __RESP_TX_H
#include "capri.h"
#include "resp_tx_args.h"
#include "txdma_phv.h"
#include "common_phv.h"

// phv 
struct resp_tx_phv_t {
    // dma commands

    // scratch

    // common tx
    struct phv_ common;
};

struct resp_tx_phv_global_t {
    struct phv_global_common_t common;
};

// stage to stage argument structures

struct resp_tx_s0_t {
    lif: 11;
    qtype: 3;
    qid: 24;
};

struct resp_tx_s0_k_t {
    struct capri_intrinsic_s0_k_t intrinsic;
    struct resp_tx_s0_t args;
};

#endif //__RESP_TX_H
