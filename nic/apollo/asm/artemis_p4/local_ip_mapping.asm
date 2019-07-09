#include "artemis.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_local_ip_mapping_k.h"

struct local_ip_mapping_k_ k;
struct local_ip_mapping_d  d;
struct phv_ p;

%%

local_ip_mapping_info:
    nop.!c1.e
    phvwr           p.vnic_metadata_vpc_id2, k.vnic_metadata_vpc_id
    sne             c1, d.local_ip_mapping_info_d.vnic_id, r0
    phvwr.c1        p.vnic_metadata_vnic_id, d.local_ip_mapping_info_d.vnic_id
    phvwr.c1        p.vnic_metadata_vpc_id, d.local_ip_mapping_info_d.vpc_id
    phvwr           p.p4_to_rxdma_service_tag, \
                        d.local_ip_mapping_info_d.service_tag
    phvwr           p.nat_metadata_pa_or_ca_xlate_idx, d.local_ip_mapping_info_d.pa_or_ca_xlate_idx
    phvwr.e         p.nat_metadata_public_xlate_idx, d.local_ip_mapping_info_d.public_xlate_idx
    phvwr.f         p.nat_metadata_ca6_xlate_idx, d.local_ip_mapping_info_d.ca6_xlate_idx

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
local_ip_mapping_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop
