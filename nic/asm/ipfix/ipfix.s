#include "ipfix/asm_out/ingress.h"
#include "ipfix/asm_out/INGRESS_p.h"
#include "../common-p4+/include/capri-macros.h"

struct ipfix_start_k k;
struct ipfix_start_d d;
struct phv_ p;

%%

    .param      ipfix_flow_hash

ipfix_start:
    // disable scheduler bit
    addi        r1, r0, DB_ADDR_BASE
    or          r1, r1, 0x2, DB_UPD_SHFT
    or          r1, r1, 1005, DB_LIF_SHFT
    memwr.b     r1, 1

    // save info to global
    phvwr       p.ipfix_t0_metadata_pktaddr, d.u.ipfix_start_d.pktaddr
    phvwr       p.ipfix_t0_metadata_pktsize, d.u.ipfix_start_d.pktsize
    phvwr       p.ipfix_t0_metadata_eindex, d.u.ipfix_start_d.eindex
    phvwr       p.ipfix_metadata_qstate_addr, \
                    k.{p4_txdma_intr_qstate_addr_sbit0_ebit1, \
                       p4_txdma_intr_qstate_addr_sbit2_ebit33}

    // table 0 : lookup flow_hash table
    addi        r1, r0, 0x90000000
    addi        r2, r0, 0xe5a2d
    add         r1, r1, r2, 6
    phvwr       p.common_te0_phv_table_addr, r1
    phvwri      p.common_te0_phv_table_pc, ipfix_flow_hash[33:6]
    phvwr       p.common_te0_phv_table_raw_table_size, 6
    phvwr       p.common_te0_phv_table_lock_en, 0

    // enable lookups in next stage
    phvwr       p.app_header_table0_valid, 1
    phvwr       p.app_header_table1_valid, 0
    phvwr       p.app_header_table2_valid, 0
    phvwr.e     p.app_header_table3_valid, 0
    nop
