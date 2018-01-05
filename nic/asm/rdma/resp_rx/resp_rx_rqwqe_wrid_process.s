#include "capri.h"
#include "types.h"
#include "resp_rx.h"
#include "rqcb.h"
#include "common_phv.h"

struct resp_rx_phv_t p;
struct resp_rx_rqwqe_process_k_t k;
struct rqwqe_base_t d;

%%

.align
resp_rx_rqwqe_wrid_process:

    //MPU GLOBAL
    RQCB1_ADDR_GET(r7)

    //phv_p->cqwqe.id.wrid = wqe_p->wrid;
    phvwr       p.cqwqe.id.wrid, d.wrid
    
    // I don't think below memwr is necessary, as this function is going to
    // get called only when WRITE and IMMDT both flagas are set, which 
    // can happen for either ONLY or LAST packet of write. In both cases,
    // why do we need to remember wrid into rqcb1 ?
    // rqcb1_p->wrid = wqe_p->wrid
    //TODO: make sure wrid is at byte boundary so that below divison works
    //add         r6, r7, BYTE_OFFSETOF(rqcb1_t, wrid)
    //TODO: change to DMA
    //memwr.d     r6, d.wrid

    CAPRI_SET_TABLE_0_VALID(0)

    nop.e
    nop
