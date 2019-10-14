
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_rx_table_s3_t2_k.h"

#include "defines.h"

struct phv_ p;
struct rx_table_s3_t2_k_ k;
struct rx_table_s3_t2_eth_rx_event_d d;

#define  _r_stats       r1  // Stats
#define  _r_eq_desc     r2  // event desc addr
#define  _r_intr_addr   r3  // intr assert addr
#define  _r_ptr         r4  // dma p-offset
#define  _r_index       r5  // dma index

#define  _c_intr        c1  // Stats

// Essentially the same as eth_tx_event.s

%%

.param eth_rx_stats

.align
eth_rx_event:
    LOAD_STATS(_r_stats) 

    // Error loading the eq state?
    setcf           c7, [c2 | c3 | c7]
    b.c7            eth_rx_event_error
    SET_STAT(_r_stats, c7, event_error) // BD Slot, for branch taken

    // Is the eq disabled?
    seq             c7, d.eq_enable, 0
    b.c7            eth_rx_event_error
    SET_STAT(_r_stats, c7, event_disabled) // BD Slot, for branch taken

    // Compute the event descriptor address
    add             _r_eq_desc, d.{eq_ring_base}.dx, d.{eq_index}.hx, LG2_EQ_DESC_SIZE

    // Advance the event index
    tblmincri       d.{eq_index}.hx, d.eq_ring_size, 1

    // Finalize the event descriptor, all other fields were set prior
    phvwr           p.eq_desc_gen_color, d.eq_gen

    // Advance the color generation when the ring wraps
    seq             c7, d.eq_index, 0
    tblmincri.c7    d.eq_gen, 8, 1

    // Post an event only?  then proceed
    seq             c7, k.eth_rx_global_do_intr, 1

    //SAVE_STATS(_r_stats)

    // If exiting, just prepare the phv for the eth_rx_completion action.
    //
    // This program will exit, but table valid bits are not cleared here:
    // the table valid bits are under control of rx_packet, and that action
    // will clear this table valid bit for the next stage.

    phvwr.!c7       p.eth_rx_t0_s2s_eq_desc_addr, _r_eq_desc
    phvwr.!c7.e     p.eth_rx_t0_s2s_intr_index, d.{intr_index}.hx
    phvwr.!c7       p.eth_rx_global_do_intr, d.{intr_enable}

eth_rx_event_only:
    seq             _c_intr, d.{intr_enable}, 1

    // Set intrinsics
#ifndef GFT
    phvwrpair       p.p4_intr_global_tm_iport, TM_PORT_DMA, p.p4_intr_global_tm_oport, TM_PORT_INGRESS
#else
    phvwrpair       p.p4_intr_global_tm_iport, TM_PORT_DMA, p.p4_intr_global_tm_oport, TM_PORT_EGRESS
#endif

    // Init DMA command pointer
    phvwri          p.p4_rxdma_intr_dma_cmd_ptr, ETH_DMA_CMD_START_OFFSET
    addi            _r_index, r0, (ETH_DMA_CMD_START_FLIT << LOG_NUM_DMA_CMDS_PER_FLIT) | ETH_DMA_CMD_START_INDEX

    // Post Event
    DMA_CMD_PTR(_r_ptr, _r_index, r7)
    DMA_PHV2MEM(_r_ptr, !_c_intr, k.eth_rx_global_host_queue, _r_eq_desc, CAPRI_PHV_START_OFFSET(eq_desc_code), CAPRI_PHV_END_OFFSET(eq_desc_gen_color), r7)
    b.!_c_intr      eth_rx_event_done
    DMA_CMD_NEXT(_r_index) // BD Slot

    // Assert Interrupt
    addi            _r_intr_addr, r0, INTR_ASSERT_BASE
    add             _r_intr_addr, _r_intr_addr, d.{intr_index}.hx, LG2_INTR_ASSERT_STRIDE
    DMA_CMD_PTR(_r_ptr, _r_index, r7)
    DMA_HBM_PHV2MEM_WF(_r_ptr, c0, _r_intr_addr, CAPRI_PHV_START_OFFSET(eq_desc_intr_data), CAPRI_PHV_END_OFFSET(eq_desc_intr_data), r7)
    DMA_CMD_NEXT(_r_index)

eth_rx_event_done:
    //SAVE_STATS(_r_stats)

    // If this is event-only, then there is no rx action.  The table bits are
    // under control of this action, which will launch the stats action.

    // Launch stats action
    phvwri          p.{app_header_table0_valid...app_header_table3_valid}, TABLE_VALID_1

    phvwri.e        p.common_te1_phv_table_pc, eth_rx_stats[38:6]
    phvwri.f        p.common_te1_phv_table_raw_table_size, CAPRI_RAW_TABLE_SIZE_MPU_ONLY

eth_rx_event_error:
    // EQ error or disabled: will result in a missed event!
    // The commit action has disarmed the cq.
    SAVE_STATS(_r_stats)

    // Not event-only: proceed with completion, without an event.
    seq             c7, k.eth_rx_global_do_intr, 1
    phvwri.!c7.e    p.eth_rx_global_do_eq, 0
    phvwri.!c7      p.eth_rx_global_do_intr, 0

    // Event-only: drop this phv
    phvwr           p.p4_intr_global_drop, 1
    phvwr           p.eth_rx_global_drop, 1     // increment pkt drop counters

    // Launch stats action
    phvwri          p.{app_header_table0_valid...app_header_table3_valid}, TABLE_VALID_1

    phvwri.e        p.common_te1_phv_table_pc, eth_rx_stats[38:6]
    phvwri.f        p.common_te1_phv_table_raw_table_size, CAPRI_RAW_TABLE_SIZE_MPU_ONLY
