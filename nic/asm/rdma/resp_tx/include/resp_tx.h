#ifndef __RESP_TX_H
#define __RESP_TX_H
#include "capri.h"
#include "types.h"
#include "resp_tx_args.h"
#include "INGRESS_p.h"
#include "common_phv.h"

#define RESP_TX_MAX_DMA_CMDS        16
#define RESP_TX_DMA_CMD_RSQ_DB      0

#define RESP_TX_DMA_CMD_START_FLIT_ID   8 // flits 8-11 are used for dma cmds

// phv 
struct resp_tx_phv_t {
    // dma commands (flit 8 - 11)

    // scratch (flit 6 - 7)
    db_data: 32;

    // common tx (flit 0 - 5)
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
    struct resp_tx_to_stage_t to_stage;
    struct phv_global_common_t global;
};

#endif //__RESP_TX_H
