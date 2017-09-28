#include "capri.h"
#include "resp_tx.h"
#include "rqcb.h"
#include "types.h"
#include "common_phv.h"
#include "ingress.h"

struct resp_tx_phv_t p;
struct resp_tx_rqcb1_write_back_process_k_t k;
struct rqcb1_t d;

#define DB_ADDR             r2
#define DB_DATA             r3
#define CURR_READ_RSP_PSN   r4
#define DMA_CMD_BASE        r1

%%

resp_tx_rqcb1_write_back_process:

add_headers:

    // intrinsic
    DMA_CMD_I_BASE_GET(DMA_CMD_BASE, r3, RESP_TX_DMA_CMD_START_FLIT_ID, RESP_TX_DMA_CMD_INTRINSIC)
    DMA_PHV2PKT_SETUP(DMA_CMD_BASE, common.p4_intr_global_tm_iport, common.p4_intr_global_glb_rsv)
    phvwri          p.common.p4_intr_global_tm_iport, TM_PORT_DMA
    phvwri          p.common.p4_intr_global_tm_oport, TM_PORT_INGRESS
    phvwri          p.common.p4_intr_global_tm_oq, 0

    // common-p4+
    DMA_CMD_I_BASE_GET(DMA_CMD_BASE, r3, RESP_TX_DMA_CMD_START_FLIT_ID, RESP_TX_DMA_CMD_COMMON_P4PLUS)
    DMA_PHV2PKT_SETUP(DMA_CMD_BASE, p4plus_to_p4, p4plus_to_p4);
    phvwr          P4PLUS_TO_P4_APP_ID, P4PLUS_APPTYPE_RDMA
    phvwr          P4PLUS_TO_P4_FLAGS, d.p4plus_to_p4_flags
    phvwr          P4PLUS_TO_P4_VLAN_ID, 0

    tblwr       d.read_rsp_in_progress, k.args.read_rsp_in_progress
    seq         c1, k.args.read_rsp_in_progress, 1
    cmov        CURR_READ_RSP_PSN, c1, k.args.curr_read_rsp_psn, 0

    // TODO: ordering rules
    tblwr       d.curr_read_rsp_psn, CURR_READ_RSP_PSN
    tblwr       d.read_rsp_lock, 0
    bcf         [c1], exit
    CAPRI_SET_TABLE_1_VALID(0) //BD Slot

    // TODO: ordering rules
    // ring doorbell to update RSQ_C_INDEX to NEW_RSQ_C_INDEX
    // currently it is done thru memwr. Need to see if it should be done thru DMA
    DOORBELL_WRITE_CINDEX(k.global.lif, k.global.qtype, k.global.qid, RSQ_RING_ID, k.args.new_rsq_c_index, DB_ADDR, DB_DATA) 

exit:
    nop.e
    nop
