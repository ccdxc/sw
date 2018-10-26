#include "apollo_rxdma.h"
#include "INGRESS_p.h"
#include "ingress.h"

struct phv_                         p;
struct slacl_proto_dport_lpm_s1_k   k;
struct slacl_proto_dport_lpm_s1_d   d;

#define prog_name           slacl_proto_dport_lpm_s1
#define keys(a)             d.slacl_proto_dport_lpm_s1_d.key ## a
#define key                 k.slacl_metadata_proto_dport
#define next_addr           p.slacl_metadata_proto_dport_table_addr
#define s2_offset           k.slacl_metadata_proto_dport_lpm_s2_offset
#define curr_addr           k.{slacl_metadata_proto_dport_table_addr_sbit0_ebit1,\
                               slacl_metadata_proto_dport_table_addr_sbit2_ebit33}

#define LPM_KEY_SIZE        3
#define LPM_DATA_SIZE       2
#define LPM_S0_ENTRY_PAD    19
#define LPM_S1_ENTRY_PAD    19
#define LPM_S2_ENTRY_PAD    27

#include "../include/lpm1.h"

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
slacl_proto_dport_lpm_s1_error:
    phvwr.e         p.capri_intr_drop, 1
    nop
