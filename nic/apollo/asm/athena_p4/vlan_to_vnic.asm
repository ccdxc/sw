#include "athena.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_vlan_to_vnic_k.h"

struct vlan_to_vnic_k_  k;
struct vlan_to_vnic_d   d;
struct phv_             p;

%%

vlan_to_vnic:
    add             r1, d.vlan_to_vnic_d.vnic_id, r0
    beq             r1, r0, vlan_to_vnic_failed

    phvwr           p.key_metadata_vnic_id, r1
    phvwr.e         p.control_metadata_vnic_type, d.vlan_to_vnic_d.vnic_type
    nop
    
vlan_to_vnic_failed:
    phvwr           p.control_metadata_flow_miss, TRUE
    phvwr           p.ingress_recirc_header_flow_done, TRUE
    phvwr           p.ingress_recirc_header_dnat_done, TRUE
    phvwr.e         p.control_metadata_skip_dnat_lkp, TRUE
    phvwr           p.control_metadata_skip_flow_lkp, TRUE


/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
vlan_to_vnic_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    phvwr           p.capri_p4_intrinsic_valid, TRUE
