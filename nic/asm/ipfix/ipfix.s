#include "ipfix/asm_out/ingress.h"
#include "ipfix/asm_out/INGRESS_p.h"
#include "../common-p4+/include/capri-macros.h"

struct ipfix_start_k k;
struct ipfix_start_d d;
struct phv_ p;

%%

    .param      p4_flow_hash_base
    .param      ipfix_flow_hash
    .param      ipfix_export_packet

ipfix_start:
    seq         c1, k.p4_txdma_intr_qid, 1
    bcf         [c1], ipfix_export_start

    slt         c1, d.u.ipfix_start_d.sindex, d.u.ipfix_start_d.eindex
    // disable scheduler bit if we have processed all entries
    addi        r1, r0, DB_ADDR_BASE
    or          r1, r1, 0x2, DB_UPD_SHFT
    or          r1, r1, 1005, DB_LIF_SHFT
    memwr.!c1.dx    r1, r0

    // ring the export doorbell, if needed
    add         r1, d.{u.ipfix_start_d.rstart}.hx, 16
    slt         c2, r1, d.u.ipfix_start_d.rnext
    setcf       c2, [!c1 & c2]
    phvwr.c2    p.ipfix_metadata_do_export, 1

    // save info to global
    phvwr       p.ipfix_metadata_qstate_addr, \
                    k.{p4_txdma_intr_qstate_addr_sbit0_ebit1, \
                       p4_txdma_intr_qstate_addr_sbit2_ebit33}

    // table 0 : lookup flow_hash table
    addi        r1, r0, loword(p4_flow_hash_base)
    addui       r1, r1, hiword(p4_flow_hash_base)
    add         r1, r1, d.u.ipfix_start_d.sindex, 6
    phvwr       p.common_te0_phv_table_addr, r1
    phvwri      p.common_te0_phv_table_pc, ipfix_flow_hash[33:6]
    phvwr       p.common_te0_phv_table_raw_table_size, 6
    phvwr       p.common_te0_phv_table_lock_en, 0

    // update start index
    tbladd.c1   d.{u.ipfix_start_d.sindex}.wx, 1

    // enable lookups in next stage
    phvwr       p.app_header_table0_valid, 1
    phvwr       p.app_header_table1_valid, 0
    phvwr       p.app_header_table2_valid, 0
    phvwr.e     p.app_header_table3_valid, 0
    nop

ipfix_export_start:
    // disable scheduler bit
    addi        r1, r0, DB_ADDR_BASE
    or          r1, r1, 0x2, DB_UPD_SHFT
    or          r1, r1, 1005, DB_LIF_SHFT
    add         r2, r0, 1, 24
    memwr.dx    r1, r2

    // table 1 : lookup qstate address of qid 0
    sub         r1, k.{p4_txdma_intr_qstate_addr_sbit0_ebit1, \
                       p4_txdma_intr_qstate_addr_sbit2_ebit33}, 0x40
    phvwr       p.common_te1_phv_table_addr, r1
    phvwri      p.common_te1_phv_table_pc, ipfix_export_packet[33:6]
    phvwr       p.common_te1_phv_table_raw_table_size, 6
    phvwr       p.common_te1_phv_table_lock_en, 0

    // enable lookups in next stage
    phvwr       p.app_header_table0_valid, 0
    phvwr       p.app_header_table1_valid, 1
    phvwr       p.app_header_table2_valid, 0
    phvwr.e     p.app_header_table3_valid, 0
    nop
