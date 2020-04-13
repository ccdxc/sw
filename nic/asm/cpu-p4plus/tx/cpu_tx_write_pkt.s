#include "INGRESS_p.h"
#include "INGRESS_cpu_tx_write_pkt_k.h"
#include "ingress.h"
#include "cpu-table.h"
#include "cpu-macros.h"

struct phv_ p;
struct cpu_tx_write_pkt_k_ k;
struct cpu_tx_write_pkt_d d;

#define c_upd_vlan_tag      c1
#define c_rem_vlan_tag      c2
#define c_add_qs_trlr       c3
#define c_free_buffer       c4
#define c_vlan_tag_exists   c5
#define c_qs_or_free_buffer c6
#define c_temp              c7

#define r_addr              r1
#define r_len               r2
#define r_temp              r3

#define ASCQ_FREE_REQ_OFFSET 56

%%
    .align
    .param cpu_tx_sem_full_drop_error
cpu_tx_write_pkt_start:
    CAPRI_CLEAR_TABLE_VALID(0)

dma_cmd_global_intrinsic:
#ifdef ELBA
    phvwri  p.p4_intr_global_tm_iport, 5
    phvwri  p.p4_intr_global_tm_oport, 7
#else
    phvwri  p.p4_intr_global_tm_iport, 9
    phvwri  p.p4_intr_global_tm_oport, 11
#endif
    phvwr   p.p4_intr_global_tm_oq, k.to_s4_tm_oq
    phvwr   p.p4_intr_global_lif, k.to_s4_src_lif

    addi    r_addr, r0, CAPRI_PHV_START_OFFSET(dma_cmd0_dma_cmd_type) / 16
    phvwr   p.p4_txdma_intr_dma_cmd_ptr, r_addr

    phvwri  p.dma_cmd0_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_PKT
    phvwr   p.dma_cmd0_dma_cmd_phv_start_addr, CPU_PHV_INTRINSIC_START
    phvwr   p.dma_cmd0_dma_cmd_phv_end_addr, CPU_PHV_INTRINSIC_END

dma_cmd_txdma_intrinsic:
    phvwri  p.dma_cmd1_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_PKT
    phvwr   p.dma_cmd1_dma_cmd_phv_start_addr, CPU_PHV_TXDMA_INTRINSIC_START
    phvwr   p.dma_cmd1_dma_cmd_phv_end_addr, CPU_PHV_TXDMA_INTRINSIC_END

cpu_tx_check_flags:
    sne     c_upd_vlan_tag, k.common_phv_write_vlan_tag, r0
    sne     c_rem_vlan_tag, k.common_phv_rem_vlan_tag, r0
    sne     c_add_qs_trlr, k.common_phv_add_qs_trlr, r0
    sne     c_free_buffer, k.common_phv_free_buffer, r0
    setcf   c_qs_or_free_buffer, [c_add_qs_trlr | c_free_buffer]

cpu_tx_check_vlan_rewrite:
    sne     c_vlan_tag_exists, k.to_s4_vlan_tag_exists, r0
    setcf   c_temp, [c_rem_vlan_tag & c_vlan_tag_exists]
    bcf     [c_upd_vlan_tag | c_temp], dma_cmd_vlan_rewrite_header
    nop

dma_cmd_data:
    // Calculate offsets after removing CPU HDR
    add     r_addr, k.to_s4_page_addr, CPU_TO_P4PLUS_HDR_SIZE
    sub     r_len, k.to_s4_len, CPU_TO_P4PLUS_HDR_SIZE

    phvwri  p.dma_cmd2_dma_cmd_type, CAPRI_DMA_COMMAND_MEM_TO_PKT
    phvwri.!c_add_qs_trlr p.dma_cmd2_dma_cmd_pkt_eop, 1

    phvwr   p.dma_cmd2_dma_cmd_addr, r_addr
    phvwr   p.dma_cmd2_dma_cmd_size, r_len
    b       cpu_tx_write_quiesce_trailer
    phvwr.!c_qs_or_free_buffer p.dma_cmd2_dma_cmd_eop, 1

dma_cmd_vlan_rewrite_header:
    //  Start offset
    add     r_addr, k.to_s4_page_addr, CPU_TO_P4PLUS_HDR_SIZE
    add     r_len, r0, (P4PLUS_TO_P4_HDR_SIZE + L2HDR_DOT1Q_OFFSET)

    phvwri  p.dma_cmd2_dma_cmd_type, CAPRI_DMA_COMMAND_MEM_TO_PKT
    phvwr   p.dma_cmd2_dma_cmd_addr, r_addr
    phvwr   p.dma_cmd2_dma_cmd_size, r_len

dma_cmd_vlan_header:
    phvwri.c_upd_vlan_tag  p.dma_cmd3_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_PKT
    phvwr.c_upd_vlan_tag   p.dma_cmd3_dma_cmd_phv_start_addr, CPU_PHV_VLAN_TAG_START
    phvwr.c_upd_vlan_tag   p.dma_cmd3_dma_cmd_phv_end_addr, CPU_PHV_VLAN_TAG_END

dma_cmd_trailer:
    //  trailer start =header start + header size
    add     r_addr, r_addr, r_len
    sub     r_len, k.to_s4_len, r_len
    sub     r_len, r_len, CPU_TO_P4PLUS_HDR_SIZE

    // Remove any existing vlan tag from the packet
    add.c_vlan_tag_exists  r_addr, r_addr, VLAN_TAG_HDR_SIZE
    sub.c_vlan_tag_exists  r_len, r_len, VLAN_TAG_HDR_SIZE

    phvwri  p.dma_cmd4_dma_cmd_type, CAPRI_DMA_COMMAND_MEM_TO_PKT
    phvwr   p.dma_cmd4_dma_cmd_addr, r_addr
    phvwr   p.dma_cmd4_dma_cmd_size, r_len

    phvwri.!c_add_qs_trlr       p.dma_cmd4_dma_cmd_pkt_eop, 1
    phvwri.!c_qs_or_free_buffer p.dma_cmd4_dma_cmd_eop, 1

cpu_tx_write_quiesce_trailer:
    bcf     [!c_add_qs_trlr], cpu_tx_write_ascq
    nop

    CAPRI_DMA_CMD_PHV2PKT_SETUP(dma_cmd5_dma_cmd,
                                quiesce_pkt_trlr_timestamp,
                                quiesce_pkt_trlr_timestamp)

    phvwri                p.dma_cmd5_dma_pkt_eop, 1
    phvwri.!c_free_buffer p.dma_cmd5_dma_cmd_eop, 1

cpu_tx_write_ascq:
    bcf     [!c_free_buffer], cpu_tx_write_pkt_done
    nop

    seq     c_temp, d.u.write_pkt_d.ascq_full, 1
    b.c_temp cpu_tx_ascq_full_fatal_error
    nop

    CPU_TX_ASCQ_ENQUEUE(r_temp,
                        k.to_s4_asq_desc_addr,
                        d.{u.write_pkt_d.ascq_pindex}.wx,
                        k.{common_phv_ascq_base},
                        ascq_ring_entry_descr_addr,
                        dma_cmd_ascq_dma_cmd,
                        1,
                        1)
    add r7, k.{common_phv_cpucb_addr}, r0
    CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r7, ASCQ_FREE_REQ_OFFSET, 1)

#if 0

    /*
     * We'll enable this when the code is ready to free CPU-RX-DPR directly from P4+ instead of ARM ASCQ.
     */

    add    r2, k.common_phv_cpu_dpr_sem_cindex, (ASIC_CPU_RX_DPR_RING_SIZE + 1)
	//addi   r2, r0, 1
    phvwr  p.common_phv_cpu_dpr_sem_cindex, r2.wx
    addi   r3, r0, ASIC_SEM_CPU_RX_DPR_ALLOC_CI_RAW_ADDR
    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_cmd_ascq_dma_cmd, r3, common_phv_cpu_dpr_sem_cindex, common_phv_cpu_dpr_sem_cindex)

#endif

cpu_tx_write_pkt_done:
    nop.e
    nop

cpu_tx_ascq_full_fatal_error:
    add r3, r0, k.{common_phv_cpucb_addr}
    CAPRI_NEXT_TABLE_READ(0, TABLE_LOCK_EN, cpu_tx_sem_full_drop_error, r3, TABLE_SIZE_512_BITS)
    CAPRI_CLEAR_TABLE1_VALID
    CAPRI_CLEAR_TABLE2_VALID
    CAPRI_CLEAR_TABLE3_VALID
    nop.e
    nop
