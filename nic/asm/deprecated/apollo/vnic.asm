#include "apollo.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct vnic_k k;
struct vnic_d d;
struct phv_ p;

%%

vnic_mapping:
    phvwr           p.control_metadata_direction, TX_PACKET
    phvwr           p.control_metadata_ingress_vnic, d.vnic_mapping_d.vnic
    phvwr           p.control_metadata_subnet_id, d.vnic_mapping_d.subnet_id
    phvwr           p.lpm_metadata_base_addr, d.vnic_mapping_d.lpm_addr_1
    phvwr           p.lpm_metadata_addr, d.vnic_mapping_d.lpm_addr_1
    phvwr           p.slacl_metadata_base_addr, d.vnic_mapping_d.slacl_addr_1
    phvwr           p.slacl_metadata_addr1, d.vnic_mapping_d.slacl_addr_1
    add.e           r1, d.vnic_mapping_d.slacl_addr_1, SLACL_IP_TABLE_OFFSET
    phvwr           p.slacl_metadata_addr2, r1

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
vnic_error:
    nop.e
    nop
