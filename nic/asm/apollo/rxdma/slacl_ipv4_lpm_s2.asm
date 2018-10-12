#include "apollo_rxdma.h"
#include "INGRESS_p.h"
#include "ingress.h"

struct phv_                 p;
struct slacl_ipv4_lpm_s2_k  k;
struct slacl_ipv4_lpm_s2_d  d;

#define prog_name       slacl_ipv4_ipm_s2_func
#define prog_name_ext   slacl_ipv4_lpm_s2_ext
#define key             r1
#define keys(a)         d.slacl_ipv4_lpm_s2_d.key ## a
#define data(a)         d.slacl_ipv4_lpm_s2_d.data ## a

#define LPM_KEY_SIZE   4
#define LPM_DATA_SIZE  2

%%

slacl_ipv4_lpm_s2:
    add             r1, k.slacl_metadata_ip_sbit120_ebit127, \
                        k.slacl_metadata_ip_sbit0_ebit119[23:0], 8

#include "../include/lpm2.h"

slacl_ipv4_lpm_s2_ext:
    add             r1, r0, k.{p4_to_rxdma_header_slacl_base_addr_sbit0_ebit1... \
                        p4_to_rxdma_header_slacl_base_addr_sbit2_ebit33}
    add             r2, r1, SLACL_P1_TABLE_OFFSET
    add             r2, r2, k.slacl_metadata_sport_class_id, 10
    add.e           r2, r2, r7
    phvwr           p.slacl_metadata_p1_table_addr, r2

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
slacl_ipv4_lpm_s2_error:
    nop.e
    nop
