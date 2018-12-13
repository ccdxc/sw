#include "tcp-constants.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "tcp_common.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s3_t0_ooq_tcp_tx_k.h"

struct phv_ p;
struct s3_t0_ooq_tcp_tx_k_ k;
struct s3_t0_ooq_tcp_tx_ooq_tcp_txdma_load_one_descr_d d;

%%
    .align
    .param tcp_ooo_txdma_load_qstate_and_update
tcp_ooo_txdma_load_one_descr_addr:
    //write tcp flags from descr scratch to phv
    add r1, k.common_phv_qstate_addr, r0 
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_DIS, tcp_ooo_txdma_load_qstate_and_update, r1, TABLE_SIZE_512_BITS) 
    nop.e
    nop
