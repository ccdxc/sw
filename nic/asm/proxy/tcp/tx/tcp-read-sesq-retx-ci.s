/*
 * 	Implements the reading of SESQ entry to queue the desc to TCP tx q
 */

#include "tcp-constants.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s1_t1_tcp_tx_k.h"
	
struct phv_ p;
struct s1_t1_tcp_tx_k_ k;
struct s1_t1_tcp_tx_read_sesq_retx_ci_d d;
	
	
%%
        .align
        .param          tcp_tx_read_descr_start
        .param          tcp_tx_read_tcp_flags_start
        .param          tcp_tx_s2_bubble_start
        .param          TCP_PROXY_STATS
tcp_tx_sesq_read_retx_ci_stage1_start:
        CAPRI_CLEAR_TABLE_VALID(1)

        seq             c1, k.common_phv_pending_asesq, 1

        add.c1          r4, d.{pad1...descr1_addr}.dx, r0
        sub.!c1         r4, d.{descr1_addr}, NIC_DESC_ENTRY_0_OFFSET

        phvwr           p.ring_entry1_descr_addr, r4
        // For asesq packets len is not encoded in the ring entry
        // It will be filled in the next stage (tcp-read-descr.s)
        phvwr.!c1       p.t0_s2s_clean_retx_len1, d.len1

        b.c1            read_asesq_ci_end
        nop
        blti            r4, ASIC_HBM_BASE, tcp_tx_read_sesq_ci_fatal_error

        sub             r4, d.{descr2_addr}, NIC_DESC_ENTRY_0_OFFSET
        phvwr           p.ring_entry2_descr_addr, r4
        phvwr           p.t0_s2s_clean_retx_len2, d.len2
        sub             r4, d.{descr3_addr}, NIC_DESC_ENTRY_0_OFFSET
        phvwr           p.ring_entry3_descr_addr, r4
        phvwr           p.t0_s2s_clean_retx_len3, d.len3
        sub             r4, d.{descr4_addr}, NIC_DESC_ENTRY_0_OFFSET
        phvwr           p.ring_entry4_descr_addr, r4
        phvwr           p.t0_s2s_clean_retx_len4, d.len4
        sub             r4, d.{descr5_addr}, NIC_DESC_ENTRY_0_OFFSET
        phvwr           p.ring_entry5_descr_addr, r4
        phvwr           p.t0_s2s_clean_retx_len5, d.len5
        sub             r4, d.{descr6_addr}, NIC_DESC_ENTRY_0_OFFSET
        phvwr           p.ring_entry6_descr_addr, r4
        phvwr           p.t0_s2s_clean_retx_len6, d.len6
        sub             r4, d.{descr7_addr}, NIC_DESC_ENTRY_0_OFFSET
        phvwr           p.ring_entry7_descr_addr, r4
        phvwr           p.t0_s2s_clean_retx_len7, d.len7
        sub             r4, d.{descr8_addr}, NIC_DESC_ENTRY_0_OFFSET
        phvwr           p.ring_entry8_descr_addr, r4
        phvwr           p.t0_s2s_clean_retx_len8, d.len8

        // HACK to handle descriptors with FIN flag - they will be zero length,
        // but occupy a sequence number so increment by 1. Need to find a
        // better way to free these descriptors
        seq             c1, d.len1, 0
        phvwr.c1        p.t0_s2s_clean_retx_len1, 1
        // Don't need to handle the rest of the descriptors as we don't batch
        // free in CLOSE_WAIT state

read_sesq_ci_end:
        CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP(0, tcp_tx_s2_bubble_start)
        nop.e
        nop

read_asesq_ci_end:
        add             r3, d.{pad1...descr1_addr}.dx, NIC_DESC_ENTRY_0_OFFSET
		CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_DIS,
                        tcp_tx_read_descr_start, r3, TABLE_SIZE_512_BITS)
        nop.e
        nop


tcp_tx_read_sesq_ci_fatal_error:
    addui           r3, r0, hiword(TCP_PROXY_STATS)
    addi            r3, r3, loword(TCP_PROXY_STATS)
    CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r3, TCP_PROXY_STATS_INVALID_RETX_SESQ_DESCR, 1)
    phvwri p.p4_intr_global_drop, 1
    CAPRI_CLEAR_TABLE0_VALID
    CAPRI_CLEAR_TABLE1_VALID
    CAPRI_CLEAR_TABLE2_VALID
    CAPRI_CLEAR_TABLE3_VALID
    nop.e
    nop
