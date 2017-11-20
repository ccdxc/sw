#include "capri.h"
#include "resp_tx.h"
#include "rqcb.h"
#include "common_phv.h"

struct resp_tx_phv_t p;
struct dcqcn_cb_t d;
struct resp_tx_cnp_process_k_t k;

#define DMA_CMD_BASE r1
#define DB_ADDR r4
#define DB_DATA r5


%%

.align
resp_tx_dcqcn_cnp_process:

    phvwr       p.bth.pkey, d.partition_key

    // ring doorbell to update C_INDEX to P_INDEX 
    // currently it is done thru memwr. Need to see if it should be done thru DMA
    DOORBELL_WRITE_CINDEX(k.global.lif, k.global.qtype, k.global.qid, CNP_RING_ID, k.args.new_c_index, DB_ADDR, DB_DATA)

exit:
    CAPRI_SET_TABLE_0_VALID(0)
    nop.e
    nop

