#include "apollo_rxdma.h"
#include "INGRESS_p.h"
#include "ingress.h"

struct phv_                         p;
struct slacl_proto_dport_lpm_s2_k   k;
struct slacl_proto_dport_lpm_s2_d   d;

#define prog_name       slacl_proto_dport_ipm_s2
#define prog_name_ext   slacl_proto_dport_lpm_s2_ext
#define key             k.slacl_metadata_proto_dport
#define keys(a)         d.slacl_proto_dport_lpm_s2_d.key ## a
#define data(a)         d.slacl_proto_dport_lpm_s2_d.data ## a

#define LPM_KEY_SIZE    3
#define LPM_DATA_SIZE   2

%%

#include "../include/lpm2.h"

slacl_proto_dport_lpm_s2_ext:
    add             r1, r0, k.{p4_to_rxdma_header_slacl_base_addr_sbit0_ebit1, \
                        p4_to_rxdma_header_slacl_base_addr_sbit2_ebit33}
    add             r1, r1, SLACL_P2_TABLE_OFFSET
    add             r1, r1, k.{slacl_metadata_p1_class_id_sbit0_ebit7, \
                               slacl_metadata_p1_class_id_sbit8_ebit9}, 8
    phvwr.e         p.slacl_metadata_p2_table_addr, r1
    phvwr           p.slacl_metadata_proto_dport_class_id, r7

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
slacl_proto_dport_lpm_s2_error:
    nop.e
    nop
