#include "apollo_txdma.h"
#include "INGRESS_p.h"
#include "ingress.h"

struct phv_            p;
struct route_lpm_s0_k  k;
struct route_lpm_s0_d  d;

#define prog_name           route_lpm_s0
#define keys(a)             d.route_lpm_s0_d.key ## a
#define key                 k.p4_to_txdma_header_lpm_dst
#define next_addr           p.p4_to_txdma_header_lpm_addr
#define s2_offset           p.txdma_control_lpm_s2_offset
#define curr_addr           k.{p4_to_txdma_header_lpm_addr_sbit0_ebit1...\
                               p4_to_txdma_header_lpm_addr_sbit2_ebit33}

#define LPM_KEY_SIZE        4
#define LPM_DATA_SIZE       2
#define LPM_S0_ENTRY_PAD    4
#define LPM_S1_ENTRY_PAD    4
#define LPM_S2_ENTRY_PAD    20

#include "../include/lpm0.h"
