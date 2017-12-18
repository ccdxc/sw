#include "capri.h"
#include "resp_rx.h"
#include "sqcb.h"
#include "common_phv.h"
#include "common_defines.h"

struct resp_rx_phv_t p;
struct dcqcn_cb_t d;
struct resp_rx_ecn_process_k_t k;

#define DMA_CMD_BASE r1
#define DB_ADDR r5
#define DB_DATA r6

// r4 is pre-loaded with cur timestamp. Use r4 for CUR_TIMESTAMP.
// NOTE: Feeding timestamp from dcqcn_cb for now since model doesn't have timestamps.
#define CUR_TIMESTAMP d.cur_timestamp

%%
    .param rdma_num_clock_ticks_per_cnp

.align
resp_rx_dcqcn_ecn_process:

    sub   r2, CUR_TIMESTAMP, d.last_cnp_timestamp
    blti  r2, rdma_num_clock_ticks_per_cnp, exit
    nop //BD slot 

    // Store cur-timestamp and pkey in cb
    tblwr  d.last_cnp_timestamp, CUR_TIMESTAMP
    tblwr  d.partition_key, k.args.p_key  
    // Trigger local doorbell to TXDMA CNP ring.
    DOORBELL_INC_PINDEX(k.global.lif,  Q_TYPE_RDMA_SQ, k.global.qid, CNP_RING_ID, DB_ADDR, DB_DATA)

exit:
    CAPRI_SET_TABLE_3_VALID(0)
    nop.e
    nop
