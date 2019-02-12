#include "apollo_rxdma.h"
#include "INGRESS_p.h"
#include "ingress.h"

struct phv_                 p;
struct sacl_ipv4_lpm_s1_k   k;
struct sacl_ipv4_lpm_s1_d   d;

#define prog_name           sacl_ipv4_lpm_s1
#define keys(a)             d.sacl_ipv4_lpm_s1_d.key ## a
#define key                 k.sacl_metadata_ip
#define next_addr           p.sacl_metadata_ipv4_table_addr
#define s2_offset           k.sacl_metadata_ipv4_lpm_s2_offset
#define curr_addr           k.{sacl_metadata_ipv4_table_addr_sbit0_ebit1,\
                               sacl_metadata_ipv4_table_addr_sbit2_ebit33}

#define LPM_KEY_SIZE        4
#define LPM_DATA_SIZE       2
#define LPM_S0_ENTRY_PAD    4
#define LPM_S1_ENTRY_PAD    4
#define LPM_S2_ENTRY_PAD    20

#include "../include/lpm1.h"

/*****************************************************************************/
/* error function                                                            */
/*****************************************************************************/
.align
.assert $ < ASM_INSTRUCTION_OFFSET_MAX
sacl_ipv4_lpm_s1_error:
    phvwr.e         p.capri_intr_drop, 1
    nop


