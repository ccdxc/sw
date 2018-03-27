#include "capri.h"
#include "types.h"
#include "resp_rx.h"
#include "rqcb.h"
#include "common_phv.h"

struct resp_rx_phv_t p;
struct resp_rx_s2_t0_k k;
struct rqwqe_base_t d;

%%

.align
resp_rx_rqwqe_wrid_process:

    //phv_p->cqwqe.id.wrid = wqe_p->wrid;
    phvwr.e       p.cqwqe.id.wrid, d.wrid
    CAPRI_SET_TABLE_0_VALID(0)  //Exit Slot
