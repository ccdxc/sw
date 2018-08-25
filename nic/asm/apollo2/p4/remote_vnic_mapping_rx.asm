#include "apollo.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct remote_vnic_mapping_rx_k k;
struct remote_vnic_mapping_rx_d d;
struct phv_ p;

%%

rvpath_info:
    sne         c1, k.vnic_metadata_vcn_id, d.rvpath_info_d.vcn_id
    phvwr.!c1   p.p4i_apollo_i2e_rvpath_subnet_id, d.rvpath_info_d.subnet_id
    phvwr.!c1.e p.p4i_apollo_i2e_rvpath_overlay_mac, d.rvpath_info_d.overlay_mac
    phvwr.!c1   p.rvpath_metadata_tep_index, d.rvpath_info_d.tep_index
    phvwr.c1.e  p.capri_intrinsic_drop, TRUE
    nop

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
remote_vnic_mapping_rx_error:
    nop.e
    nop
