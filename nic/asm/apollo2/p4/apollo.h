#include "../../../p4/apollo2/include/defines.h"
#include "../../../p4/apollo2/include/slacl_defines.h"

#define ASM_INSTRUCTION_OFFSET_MAX     (64 * 256)

// Uses c1, c2, r7
#define LOCAL_VNIC_INFO_COMMON_END(local_vnic_tag, vcn_id, skip_src_dst_check, \
                                   resource_group_1, lpm_addr_1, slacl_addr_1, \
                                   epoch1, resource_group_2, lpm_addr_2,       \
                                   slacl_addr_2, epoch2)                       \
    phvwr           p.vnic_metadata_local_vnic_tag, local_vnic_tag;            \
    phvwr           p.vnic_metadata_skip_src_dst_check, skip_src_dst_check;    \
    /* c2 will be set if using epoch1, else will be reset */;                  \
    seq             c1, k.service_header_valid, TRUE;                          \
    seq.c1          c2, k.service_header_epoch, epoch1;                        \
    slt.!c1         c2, epoch1, epoch2;                                        \
    bcf             [!c2], __use_epoch2;                                       \
    phvwr           p.vnic_metadata_vcn_id, vcn_id;                            \
__use_epoch1:;                                                                 \
    phvwr           p.policer_metadata_resource_group, resource_group_1;       \
    phvwr           p.p4_to_txdma_header_lpm_base_addr, lpm_addr_1;            \
    phvwr           p.p4_to_txdma_header_lpm_addr, lpm_addr_1;                 \
    phvwr           p.p4_to_rxdma_header_slacl_base_addr, slacl_addr_1;        \
    phvwr           p.p4_to_rxdma_header_slacl_addr1, slacl_addr_1;            \
    add             r7, slacl_addr_1, SLACL_IP_TABLE_OFFSET;                   \
    phvwr.e         p.p4_to_rxdma_header_slacl_addr2, r7;                      \
    phvwr           p.service_header_epoch, epoch1;                            \
__use_epoch2: ;                                                                \
    phvwr           p.policer_metadata_resource_group, resource_group_2;       \
    phvwr           p.p4_to_txdma_header_lpm_base_addr, lpm_addr_2;            \
    phvwr           p.p4_to_txdma_header_lpm_addr, lpm_addr_2;                 \
    phvwr           p.p4_to_rxdma_header_slacl_base_addr, slacl_addr_2;        \
    phvwr           p.p4_to_rxdma_header_slacl_addr1, slacl_addr_2;            \
    add             r7, slacl_addr_2, SLACL_IP_TABLE_OFFSET;                   \
    phvwr.e         p.p4_to_rxdma_header_slacl_addr2, r7;                      \
    phvwr           p.service_header_epoch, epoch2;
