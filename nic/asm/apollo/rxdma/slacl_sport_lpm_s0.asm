#include "apollo_rxdma.h"
#include "INGRESS_p.h"
#include "ingress.h"

struct phv_                 p;
struct slacl_sport_lpm_s0_k k;

%%

slacl_sport_lpm_s0:
    seq             c1, k.p4_to_rxdma_header_direction, TX_FROM_HOST
    phvwr.c1        p.slacl_metadata_ip[127:112], \
                        k.p4_to_rxdma_header_flow_dst_sbit0_ebit15
    phvwr.c1        p.slacl_metadata_ip[111:0], \
                        k.p4_to_rxdma_header_flow_dst_sbit16_ebit127
    phvwr.!c1       p.slacl_metadata_ip, k.p4_to_rxdma_header_flow_src
    add             r1, r0, k.{p4_to_rxdma_header_slacl_base_addr_sbit0_ebit1... \
                        p4_to_rxdma_header_slacl_base_addr_sbit2_ebit33}
    add             r2, r1, SLACL_SPORT_TABLE_OFFSET + (1<<6)
    phvwr           p.slacl_metadata_sport_table_addr, r2
    add             r2, r1, SLACL_IPV4_TABLE_OFFSET
    phvwr           p.slacl_metadata_ipv4_table_addr, r2
    add.e           r2, r1, SLACL_PROTO_DPORT_TABLE_OFFSET
    phvwr           p.slacl_metadata_proto_dport_table_addr, r2

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
slacl_sport_lpm_s0_error:
    nop.e
    nop
