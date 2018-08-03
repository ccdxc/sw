#include "apollo.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct local_vnic_by_vlan_tx_k k;
struct local_vnic_by_vlan_tx_d d;
struct phv_ p;

%%

local_vnic_info_tx:
    phvwr.e         p.vnic_metadata_local_vnic_tag, d.local_vnic_info_tx_d.local_vnic_tag
    phvwr           p.vnic_metadata_skip_src_dst_check, d.local_vnic_info_tx_d.skip_src_dst_check

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
vnic_error:
    nop.e
    nop
