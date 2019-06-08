#include "artemis.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_vnic_mapping_k.h"

struct vnic_mapping_k_ k;
struct vnic_mapping_d  d;
struct phv_ p;

%%

vnic_mapping_info:
    nop.!c1.e
    phvwr.c1        p.control_metadata_epoch, d.vnic_mapping_info_d.epoch
    phvwr.e         p.vnic_metadata_vnic_id, d.vnic_mapping_info_d.vnic_id
    phvwr.f         p.vnic_metadata_vpc_id, d.vnic_mapping_info_d.vpc_id

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
vnic_mapping_error:
    phvwr.e         p.capri_intrinsic_drop, 1
    nop
