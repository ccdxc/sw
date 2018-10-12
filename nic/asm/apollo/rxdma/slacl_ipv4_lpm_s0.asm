#include "apollo_rxdma.h"
#include "INGRESS_p.h"
#include "ingress.h"

struct phv_                 p;
struct slacl_ipv4_lpm_s0_k  k;
struct slacl_ipv4_lpm_s0_d  d;

#define prog_name      slacl_ipv4_lpm_s0
#define keys(a)        d.slacl_ipv4_lpm_s0_d.key ## a
#define key            k.slacl_metadata_ip
#define next_addr      p.slacl_metadata_ipv4_table_addr
#define s2_offset      p.slacl_metadata_ipv4_lpm_s2_offset
#define curr_addr      k.{slacl_metadata_ipv4_table_addr_sbit0_ebit1...\
                          slacl_metadata_ipv4_table_addr_sbit2_ebit33}
#define LPM_KEY_SIZE   4
#define LPM_DATA_SIZE  2

#include "../include/lpm0.h"

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
slacl_ipv4_lpm_s0_error:
    nop.e
    nop
