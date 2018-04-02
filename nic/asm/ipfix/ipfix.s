#include "ipfix/asm_out/ingress.h"
#include "ipfix/asm_out/INGRESS_p.h"
#include "../common-p4+/include/capri-macros.h"
#include "ipfix_defines.h"

struct ipfix_start_k k;
struct ipfix_start_d d;
struct phv_          p;

%%

    .param      p4_flow_hash_base
    .param      p4_flow_hash_overflow_base
    .param      ipfix_flow_hash
    .param      ipfix_export_packet

ipfix_start:
    slt         c1, k.p4_txdma_intr_qid, IPFIX_EXPORT_ID_MAX
    bcf         [!c1], ipfix_export_start
    phvwr       p.ipfix_metadata_export_id, k.p4_txdma_intr_qid

    seq         c1, d.{u.ipfix_start_d.flow_hash_table_type}, \
                    IPFIX_FLOW_HASH_TABLE
    bcf         [!c1], ipfix_flow_hash_overflow_scan

ipfix_flow_hash_scan:
    slt         c1, d.{u.ipfix_start_d.flow_hash_index_next}.wx, \
                    d.{u.ipfix_start_d.flow_hash_index_max}.wx
    bcf         [!c1], ipfix_flow_hash_scan_complete
    // set table address in r1
    addi        r1, r0, loword(p4_flow_hash_base)
    addui       r1, r1, hiword(p4_flow_hash_base)
    add         r1, r1, d.{u.ipfix_start_d.flow_hash_index_next}.wx, 6
    b           ipfix_flow_hash_setup
    tbladd      d.{u.ipfix_start_d.flow_hash_index_next}.wx, 1

ipfix_flow_hash_scan_complete:
    // set table type and index for next scan
    tblwr       d.{u.ipfix_start_d.flow_hash_index_next}, 0
    tblwr       d.{u.ipfix_start_d.flow_hash_table_type}, \
                    IPFIX_FLOW_HASH_OVERFLOW_TABLE

ipfix_flow_hash_overflow_scan:
    slt         c1, d.{u.ipfix_start_d.flow_hash_index_next}.wx, \
                    d.{u.ipfix_start_d.flow_hash_overflow_index_max}.wx
    bcf         [!c1], ipfix_flow_hash_overflow_scan_complete
    // set table address in r1
    addi        r1, r0, loword(p4_flow_hash_overflow_base)
    addui       r1, r1, hiword(p4_flow_hash_overflow_base)
    add         r1, r1, d.{u.ipfix_start_d.flow_hash_index_next}.wx, 6
    b           ipfix_flow_hash_setup
    tbladd      d.{u.ipfix_start_d.flow_hash_index_next}.wx, 1

ipfix_flow_hash_overflow_scan_complete:
    phvwr       p.ipfix_metadata_scan_complete, 1
    // set table type and index for next scan
    tblwr       d.{u.ipfix_start_d.flow_hash_index_next}, 0
    tblwr       d.{u.ipfix_start_d.flow_hash_table_type}, \
                    IPFIX_FLOW_HASH_TABLE
    // set table address in r1 for this scan
    addi        r1, r0, loword(p4_flow_hash_base)
    addui       r1, r1, hiword(p4_flow_hash_base)
    add         r1, r1, d.{u.ipfix_start_d.flow_hash_index_next}.wx, 6

ipfix_flow_hash_setup:
    phvwr       p.ipfix_t0_metadata_flow_hash_table_type, \
                    d.{u.ipfix_start_d.flow_hash_table_type}
    phvwr       p.ipfix_t0_metadata_flow_hash_index_next, \
                    d.{u.ipfix_start_d.flow_hash_index_next}.wx

    // save info to global
    phvwr       p.ipfix_metadata_qstate_addr, \
                    k.{p4_txdma_intr_qstate_addr_sbit0_ebit1, \
                       p4_txdma_intr_qstate_addr_sbit2_ebit33}

    // table 0 : lookup flow_hash table
    phvwr       p.common_te0_phv_table_addr, r1
    phvwri      p.common_te0_phv_table_pc, ipfix_flow_hash[33:6]
    phvwr       p.common_te0_phv_table_raw_table_size, 6
    phvwr       p.common_te0_phv_table_lock_en, 0

    // enable table 0 in next stage
    phvwr.e     p.{app_header_table0_valid...app_header_table3_valid}, 0x8
    nop

ipfix_export_start:
    seq         c1, d.u.ipfix_start_d.next_record_offset, IPFIX_SCAN_COMPLETE
    phvwr.c1    p.ipfix_metadata_scan_complete, 1

    // is export complete?
    seq         c1, d.u.ipfix_start_d.next_record_offset, 0
    bcf         [c1], ipfix_exit
    tblwr.!c1   d.u.ipfix_start_d.next_record_offset, 0

    // save info for next stage
    phvwr       p.ipfix_t1_metadata_flow_hash_table_type, \
                    d.u.ipfix_start_d.flow_hash_table_type
    phvwr       p.ipfix_t1_metadata_flow_hash_index_next, \
                    d.{u.ipfix_start_d.flow_hash_index_next}.wx
    add         r1, d.{u.ipfix_start_d.ipfix_hdr_offset}.hx, 16
    phvwr       p.ipfix_t1_metadata_next_record_offset, r1[15:0].hx
    phvwr       p.ipfix_metadata_qstate_addr, \
                    k.{p4_txdma_intr_qstate_addr_sbit0_ebit1, \
                       p4_txdma_intr_qstate_addr_sbit2_ebit33}

    // table 1 : lookup qstate address of expoter's qid (subtract 64B*16)
    sub         r1, k.{p4_txdma_intr_qstate_addr_sbit0_ebit1, \
                       p4_txdma_intr_qstate_addr_sbit2_ebit33}, 0x40, 4
    phvwr       p.common_te1_phv_table_addr, r1
    phvwri      p.common_te1_phv_table_pc, ipfix_export_packet[33:6]
    phvwr       p.common_te1_phv_table_raw_table_size, 6
    phvwr       p.common_te1_phv_table_lock_en, 0

    // enable table 1 in next stage
    phvwr.e     p.{app_header_table0_valid...app_header_table3_valid}, 0x4
    nop

ipfix_exit:
    phvwr.e     p.{app_header_table0_valid...app_header_table3_valid}, 0
    phvwr       p.p4_intr_global_drop, 1
