#include "req_rx.h"
#include "sqcb.h"


%%

.align
req_rx_dummy_drop_phv_process:

    // This program is only invoked when stage0 sets phv-intrinsic-drop-bit.
    // Setting all table-valid bits to 0 in stage0 results in loading eth_rx_rss_indir in stage1
    // from common-rxdma.p4. This is a workaround to avoid loading eth programs for rdma pkts.
    nop.e
    nop

