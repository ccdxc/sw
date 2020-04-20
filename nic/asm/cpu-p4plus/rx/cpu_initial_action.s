#include "INGRESS_p.h"
#include "ingress.h"
#include "cpu-table.h"
#include "nic/p4/common/defines.h"

struct phv_ p;
struct common_p4plus_stage0_app_header_table_offset_64_k k;
struct common_p4plus_stage0_app_header_table_offset_64_d d;

%%
    .param          cpu_rx_read_desc_pindex_start
    .param          cpu_rx_read_arqrx_pindex_start

.align
cpu_rx_packet_len:
    phvwr.e         p.t0_s2s_payload_len, r1
    nop

.align
cpu_rx_read_shared_stage0_start:
    CAPRI_CLEAR_TABLE0_VALID
#ifdef CAPRI_IGNORE_TIMESTAMP
    add     r6, r0, r0
#endif
    // Store timestamp in the quisce pkt trailer
    phvwr   p.quiesce_pkt_trlr_timestamp, r6.wx
    
    phvwr   p.common_phv_qstate_addr, k.{p4_rxdma_intr_qstate_addr_sbit0_ebit1...p4_rxdma_intr_qstate_addr_sbit2_ebit33}
    phvwr   p.common_phv_debug_dol, d.u.cpu_rxdma_initial_action_d.debug_dol
    phvwr   p.common_phv_flags, d.u.cpu_rxdma_initial_action_d.flags
    phvwr   p.common_phv_qstate_addr, k.{p4_rxdma_intr_qstate_addr_sbit0_ebit1...p4_rxdma_intr_qstate_addr_sbit2_ebit33}

cpu_rx_hash_key:

    /*
     * Set up HW toeplitz hash
     */
    TOEPLITZ_KEY_DATA_SETUP(CPU_HASH_KEY_PREFIX,
                            P4PLUS_APPTYPE_CPU,
                            cpu_app_header,
                            r1,
                            r2,
                            table_read_DESC_SEMAPHORE)

table_read_DESC_SEMAPHORE:
    addi    r3, r0, CPU_RX_DPR_ALLOC_IDX
    CAPRI_NEXT_TABLE_READ(1, TABLE_LOCK_DIS, 
                         cpu_rx_read_desc_pindex_start,
                         r3,
                         TABLE_SIZE_64_BITS)
    nop.e
    nop
