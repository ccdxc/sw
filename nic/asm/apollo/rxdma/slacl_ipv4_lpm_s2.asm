#include "apollo_rxdma.h"
#include "INGRESS_p.h"
#include "ingress.h"

struct phv_                 p;
struct slacl_ipv4_lpm_s2_k  k;
struct slacl_ipv4_lpm_s2_d  d;

#define prog_name       slacl_ipv4_ipm_s2
#define prog_name_ext   slacl_ipv4_lpm_s2_ext
#define key             k.slacl_metadata_ip
#define keys(a)         d.slacl_ipv4_lpm_s2_d.key ## a
#define data(a)         d.slacl_ipv4_lpm_s2_d.data ## a

#define LPM_KEY_SIZE   4
#define LPM_DATA_SIZE  2

%%

#include "../include/lpm2.h"

slacl_ipv4_lpm_s2_ext:
    or              r2, r7, k.slacl_metadata_sport_class_id, 10
    div             r6, r2, 51
    phvwr           p.slacl_metadata_ip_sport_class_id, r2
    add             r1, r0, k.{p4_to_rxdma_header_slacl_base_addr_sbit0_ebit1, \
                        p4_to_rxdma_header_slacl_base_addr_sbit2_ebit33}
    add             r1, r1, SLACL_P1_TABLE_OFFSET
    add.e           r1, r1, r6, 6
    phvwr           p.slacl_metadata_p1_table_addr, r1

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
slacl_ipv4_lpm_s2_error:
    nop.e
    nop
