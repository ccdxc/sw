#include "INGRESS_p.h"
#include "ingress.h"
#include "cpu-table.h"
#include "../../../p4/nw/include/defines.h"

struct phv_ p;
struct common_p4plus_stage0_app_header_table_offset_64_k k;
struct common_p4plus_stage0_app_header_table_offset_64_d d;

%%
    .param          cpu_rx_read_desc_pindex_start
    .param          cpu_rx_read_page_pindex_start
    .param          cpu_rx_read_arqrx_pindex_start
    .align
cpu_rx_read_shared_stage0_start:
    CAPRI_CLEAR_TABLE0_VALID
#ifdef CAPRI_IGNORE_TIMESTAMP
    add     r6, r0, r0
#endif
    // Store timestamp in the quisce pkt trailer
    phvwr   p.quiesce_pkt_trlr_timestamp, r6.wx
    
    phvwr   p.common_phv_qstate_addr, k.{p4_rxdma_intr_qstate_addr_sbit0_ebit1...p4_rxdma_intr_qstate_addr_sbit2_ebit33}
    phvwr   p.t0_s2s_payload_len, k.{cpu_app_header_packet_len_sbit0_ebit5...cpu_app_header_packet_len_sbit6_ebit13}
    phvwr   p.common_phv_debug_dol, d.u.cpu_rxdma_initial_action_d.debug_dol
    phvwr   p.common_phv_flags, d.u.cpu_rxdma_initial_action_d.flags

cpu_rx_hash_key:
    // fill Symmetric hash key
    addi    r1, r0, CPU_HASH_KEY_PREFIX 
    sll     r2, r1, CPU_HASH_KEY_SHIFT
    add     r2, r2, r1
    phvwr   p.toeplitz_key0_data[63:0], r2
    phvwr   p.toeplitz_key0_data[127:64], r2
    phvwr   p.toeplitz_key1_data[63:0], r2
    phvwr   p.toeplitz_key1_data[127:64], r2
    phvwr   p.toeplitz_key2_data[127:64], r2
    # store APP Type in the last 4 unused-bits of key2
    phvwr   p.toeplitz_key2_data[3:0], P4PLUS_APPTYPE_CPU

cpu_rx_hash_data:
/* fill data for hash */
    add     r1, r0, k.cpu_app_header_packet_type
    .brbegin
    br      r1[1:0]
    nop
        .brcase 0
            b       table_read_DESC_SEMAPHORE
            nop

        .brcase 1
            /* IPv4 */
            phvwr   p.toeplitz_input0_data[63:32], k.{cpu_app_header_l4_sport...cpu_app_header_l4_dport_sbit8_ebit15}
            phvwr   p.toeplitz_input0_data[127:64], k.cpu_app_header_ip_sa_sbit24_ebit127[87:24]
            b       table_read_DESC_SEMAPHORE
            nop

        .brcase 2
            /* IPv6 */
            phvwr   p.toeplitz_input0_data, k.{cpu_app_header_ip_sa_sbit0_ebit15...cpu_app_header_ip_sa_sbit24_ebit127}
            //phvwr   p.toeplitz_input1_data, k.{cpu_app_header_ip_da_sbit0_ebit87...cpu_app_header_ip_da_sbit88_ebit127}
            phvwrpair p.toeplitz_input1_data[127:120], k.cpu_app_header_ip_da1_sbit0_ebit7,\
                p.toeplitz_input1_data[119:64], k.cpu_app_header_ip_da1_sbit8_ebit63
            phvwr   p.toeplitz_input1_data[63:0], k.cpu_app_header_ip_da2
            phvwr   p.toeplitz_input2_data, k.{cpu_app_header_l4_sport...cpu_app_header_l4_dport_sbit8_ebit15}
            b       table_read_DESC_SEMAPHORE
            nop
       
        .brcase 3
            illegal
    .brend

table_read_DESC_SEMAPHORE:
    addi    r3, r0, RNMDR_ALLOC_IDX 
    CAPRI_NEXT_TABLE_READ(1, TABLE_LOCK_DIS, 
                         cpu_rx_read_desc_pindex_start,
                         r3,
                         TABLE_SIZE_64_BITS)
table_read_PAGE_SEMAPHORE:
    addi    r3, r0, RNMPR_ALLOC_IDX
    CAPRI_NEXT_TABLE_READ(2, TABLE_LOCK_DIS, 
                         cpu_rx_read_page_pindex_start,
                         r3,
                         TABLE_SIZE_64_BITS)
    nop.e
    nop
