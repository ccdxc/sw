#include "capri.h"
#include "req_rx.h"
#include "sqcb.h"
#include "common_phv.h"
#include "nic/p4/common/defines.h"

struct req_rx_phv_t p;
struct rome_receiver_cb_t d;
struct req_rx_s1_t3_k k;

%%

 .align
req_rx_rome_pkt_process:
    tblwr        d.cur_timestamp, 10
    CAPRI_SET_TABLE_3_VALID(0)

exit:
    nop.e
    nop
