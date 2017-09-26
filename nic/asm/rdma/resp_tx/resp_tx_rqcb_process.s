#include "capri.h"
#include "resp_tx.h"
#include "rqcb.h"
#include "types.h"
#include "common_phv.h"
#include "ingress.h"

struct resp_tx_phv_t p;
struct rqcb0_t d;
struct rdma_stage0_table_k k;

#define RQCB_TO_RQCB1_T struct resp_tx_rqcb_to_rqcb1_info_t

#define RSQWQE_P            r1
#define RQCB1_P             r2
#define RAW_TABLE_PC        r3
#define NEW_RSQ_C_INDEX     r5

//r7 is pre-loaded with Qstate-ring-not-empty 8-bit flags, one for each ring
#define RING_NOT_EMPTY r7 

%%
    .param      resp_tx_rqcb1_process

resp_tx_rqcb_process:
    // copy intrinsic to global
    add            r1, r0, offsetof(struct phv_, common_global_global_data)

    // enable below code after spr_tbladdr special purpose register is available in capsim
    #mfspr         r1, spr_tbladdr
    #srl           r1, r1, SQCB_ADDR_SHIFT
    #CAPRI_SET_FIELD(r3, PHV_GLOBAL_COMMON_T, cb_addr, r1)
    add            r2, r0, CAPRI_TXDMA_INTRINSIC_QSTATE_ADDR
    srl            r2, r2, RQCB_ADDR_SHIFT

    CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, cb_addr, r2)
    CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, lif, CAPRI_TXDMA_INTRINSIC_LIF)
    CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, qtype, CAPRI_TXDMA_INTRINSIC_QTYPE)
    CAPRI_SET_FIELD(r1, PHV_GLOBAL_COMMON_T, qid, CAPRI_TXDMA_INTRINSIC_QID)

    // set DMA cmd ptr
    TXDMA_DMA_CMD_PTR_SET(RESP_TX_DMA_CMD_START_FLIT_ID)

    seq         c1, RSQ_C_INDEX, RSQ_P_INDEX
    bcf         [c1], exit
    nop

rsq:
    add         RQCB1_P, r2, CB_UNIT_SIZE_BYTES
    add         RSQWQE_P, d.rsq_base_addr, RSQ_C_INDEX, LOG_SIZEOF_RSQWQE_T

    add         NEW_RSQ_C_INDEX, r0, RSQ_C_INDEX
    mincr       NEW_RSQ_C_INDEX, d.log_rsq_size, 1

    CAPRI_GET_TABLE_0_ARG(resp_tx_phv_t, r4)
    CAPRI_SET_FIELD(r4, RQCB_TO_RQCB1_T, rsqwqe_addr, RSQWQE_P)
    CAPRI_SET_FIELD(r4, RQCB_TO_RQCB1_T, serv_type, d.serv_type)
    CAPRI_SET_FIELD(r4, RQCB_TO_RQCB1_T, log_pmtu, d.log_pmtu)
    CAPRI_SET_FIELD(r4, RQCB_TO_RQCB1_T, new_rsq_c_index, NEW_RSQ_C_INDEX)

    CAPRI_GET_TABLE_0_K(resp_tx_phv_t, r4)
    CAPRI_SET_RAW_TABLE_PC(RAW_TABLE_PC, resp_tx_rqcb1_process)
    CAPRI_NEXT_TABLE_I_READ(r4, CAPRI_TABLE_LOCK_DIS, CAPRI_TABLE_SIZE_512_BITS, RAW_TABLE_PC, RQCB1_P)

    phvwr       p.aeth.msn, d.msn
    
    //compute credits
    // TODO: optimize below instruction set.
    // p_index += (1 << log_rsq_size)
    // credits = (p_index - cindex)%(1 << log_rsq_size)
    add         r1, r0, d.log_rsq_size
    sllv        r1, 1, r1
    add         r2, RSQ_P_INDEX, r1
    sub         r2, r2, RSQ_C_INDEX
    mincr       r2, d.log_rsq_size, r0
    phvwr       p.aeth.syndrome, r2
    
exit:
    nop.e
    nop
