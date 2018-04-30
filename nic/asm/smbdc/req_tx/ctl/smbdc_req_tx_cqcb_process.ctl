#include "capri.h"
#include "req_tx.h"
#include "cqcb.h"
struct smbdc_req_tx_s4_t0_k k;
struct cqcb_t d;


d = {
    cq_base_addr = 0x450000;
    ring0.pindex = 10;
    ring0.cindex = 10;
    color = 1;
    log_num_wqes = 4;
    log_wqe_size = 5;
};
