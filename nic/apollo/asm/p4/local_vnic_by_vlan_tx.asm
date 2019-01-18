#include "apollo.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct local_vnic_by_vlan_tx_k k;
struct local_vnic_by_vlan_tx_d d;
struct phv_ p;

%%

local_vnic_info_tx:
    seq             c1, k.ethernet_1_srcAddr, r0
    sne.!c1         c2, k.ethernet_1_srcAddr, d.local_vnic_info_tx_d.overlay_mac
    bcf             [c1|c2], local_vnic_info_tx_src_mac_error
    nop
    phvwr           p.p4i_apollo_i2e_src_slot_id, \
                        d.local_vnic_info_tx_d.src_slot_id
    LOCAL_VNIC_INFO_COMMON_END(d.local_vnic_info_tx_d.local_vnic_tag,
                               d.local_vnic_info_tx_d.vcn_id,
                               d.local_vnic_info_tx_d.skip_src_dst_check,
                               d.local_vnic_info_tx_d.resource_group_1,
                               d.local_vnic_info_tx_d.lpm_addr_1,
                               d.local_vnic_info_tx_d.slacl_addr_1,
                               d.local_vnic_info_tx_d.epoch1,
                               d.local_vnic_info_tx_d.resource_group_2,
                               d.local_vnic_info_tx_d.lpm_addr_2,
                               d.local_vnic_info_tx_d.slacl_addr_2,
                               d.local_vnic_info_tx_d.epoch2)

local_vnic_info_tx_src_mac_error:
    phvwr.c1        p.control_metadata_p4i_drop_reason[DROP_SRC_MAC_ZERO], 1
    phvwr.e         p.capri_intrinsic_drop, TRUE
    phvwr.c2        p.control_metadata_p4i_drop_reason[DROP_SRC_MAC_MISMATCH], 1

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
local_vnic_by_vlan_tx_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop
