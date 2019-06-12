#include "artemis_txdma.h"
#include "INGRESS_p.h"
#include "ingress.h"

struct remote_46_mapping_k  k;
struct remote_46_mapping_d  d;
struct phv_       p;

%%

remote_46_mapping:
    // Fill session_info_hint
    phvwr        p.session_info_hint_tx_dst_ip, d.remote_46_info_d.ipv6_tx_da
    phvwr.e      p.session_info_hint_nexthop_idx, d.remote_46_info_d.nh_id
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
remote_46_mapping_error:
    phvwr.e     p.capri_intr_drop, 1
    nop
