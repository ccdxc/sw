/*
 * Implements the TSO stage of the TxDMA P4+ pipeline
 */

#include "tcp-constants.h"
#include "tcp-phv.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "defines.h"

struct phv_ p    ;
struct tcp_tx_tso_k k    ;
struct tcp_tx_tso_tso_d d    ;


%%
    .align
    .param          tcp_tx_stats_stage5_start
    
tcp_tso_process_start:
    /* check SESQ for pending data to be transmitted */
    or              r1, k.to_s4_pending_tso_data, k.to_s4_pending_tso_retx
    or              r1, r1, k.to_s4_pending_ack_send
    sne             c1, r1, r0
    bal.c1          r7, tcp_write_xmit
    nop
    b               flow_tso_process_done
    nop

tcp_write_xmit:
    /* Get the point where we are supposed to send next from */
    seq             c1, d.retx_xmit_cursor, r0
    /* If the retx was all cleaned up , then reinit the xmit
     * cursor to snd_una cursor which is the head of data that
         * can be sent
     */
    
    tblwr.c1        d.retx_xmit_cursor, d.retx_snd_una_cursor
    nop
    /* Even after all this retx_xmit_cursor has no data, then
     * there is no data to send
     */
    seq             c1, d.retx_xmit_cursor, r0
    bcf             [c1], tcp_write_xmit_done
    nop

    seq             c1, k.to_s4_xmit_cursor_addr, r0
    bcf             [c1], tcp_write_xmit_done
    nop

    /* Write the eth header */
//eth_start:
//    phvwr        p.h_dest, d.h_dest
//    phvwr        p.h_source, d.h_source
//    phvwri        p.h_proto, ETH_P_IP
    /* Write the ip header */
//ip_start:    
//    phvwri        p.ihl, 5
//    phvwri        p.version, 4
//    phvwri        p.ip_dsfield, 0
//    phvwri        p.id, 0
//    phvwri        p.frag_off,0
//    phvwri        p.ttl, 255
//    phvwri        p.protocol, IPPROTO_TCP,
//    phvwr        p.saddr, d.saddr
//    phvwr        p.daddr, d.daddr
    /* Write the tcp header */
tcp_start:
    //phvwr        p.fid, d.fid
    //phvwr        p.source,d.source
    //phvwr        p.dest, d.dest
    //phvwr        p.seq, d.retx_xmit_seq
    //phvwr        p.ack_seq, k.rcv_nxt
    //phvwri        p.d_off, 5
    //phvwri        p.res1, 0
    //phvwri        p.ack,1
    //phvwr        p.window, k.rcv_wnd
    //phvwri        p.urg_ptr, 0

dma_cmd_intrinsic:    
    phvwri          p.p4_intr_global_tm_iport, 9
    phvwri          p.p4_intr_global_tm_oport, 11
    phvwri          p.p4_intr_global_tm_oq, 0
    // TODO: P4 pipeline is currently deriving vrf (input_properties.asm) with
    // source_lif as one of the keys. Since for TCP proxy we change the source
    // lif to SERVICE LIF, currently the input_properties table is not being
    // hit. This hack to set LIF to 0 is temporary to derive the right VRF.
    phvwri          p.p4_intr_global_lif, 0 // TODO: fix this hack
    phvwri          p.p4_txdma_intr_dma_cmd_ptr, TCP_PHV_TXDMA_COMMANDS_START

    phvwri          p.intrinsic_dma_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_PKT
    phvwr           p.intrinsic_dma_dma_cmd_phv_start_addr, TCP_PHV_INTRINSIC_START
    phvwr           p.intrinsic_dma_dma_cmd_phv_end_addr, TCP_PHV_INTRINSIC_END
dma_cmd_p4plus_to_p4_app_header:
    phvwr           p.tcp_app_header_p4plus_app_id, 1 // TODO: P4PLUS_APP_P4PLUS_APP_TCP_PROXY_ID
    phvwrmi         p.tcp_app_header_flags, P4PLUS_TO_P4_FLAGS_LKP_INST, P4PLUS_TO_P4_FLAGS_LKP_INST

    phvwri          p.app_header_dma_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_PKT
    phvwr           p.app_header_dma_dma_cmd_phv_start_addr, TCP_PHV_TX_APP_HDR_START
    phvwr           p.app_header_dma_dma_cmd_phv_end_addr, TCP_PHV_TX_APP_HDR_END
dma_cmd_hdr:    
    add             r5, k.common_phv_qstate_addr, TCP_TCB_HEADER_TEMPLATE_OFFSET

    phvwri          p.l2l3_header_dma_dma_cmd_type, CAPRI_DMA_COMMAND_MEM_TO_PKT
    phvwr           p.l2l3_header_dma_dma_cmd_addr, r5
    phvwri          p.l2l3_header_dma_dma_cmd_size, ETH_IP_VLAN_HDR_SIZE
dma_cmd_tcp_header:
    phvwr           p.tcp_header_source_port, d.source_port
    phvwr           p.tcp_header_dest_port, d.dest_port
    phvwr           p.tcp_header_seq_no, d.retx_snd_nxt // TODO : where is this updated
    // TODO: snd_nxt is hardcoded here so we can verify phv2mem. Derive it
    // correctly and check the appopriate value in the dol test case.
    phvwri          p.tx2rx_snd_nxt, 0xefeff044 // TODO: fix this hack
    phvwr           p.tcp_header_ack_no, k.common_phv_rcv_nxt // TODO : is this right?
    phvwr           p.tcp_header_data_ofs, 5
    phvwr           p.tcp_header_window, k.t0_s2s_snd_wnd

    phvwri          p.tcp_header_dma_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_PKT
    phvwr           p.tcp_header_dma_dma_cmd_phv_start_addr, TCP_PHV_TX_TCP_HDR_START
    phvwr           p.tcp_header_dma_dma_cmd_phv_end_addr, TCP_PHV_TX_TCP_HDR_END

dma_cmd_data:
    seq             c2, k.to_s4_xmit_cursor_addr, r0
    /* r6 has tcp data len being sent */
    addi            r6, r0, 0
    /* We can end up taking this branch if we ended up here
     * to send pure ack and there is really no data in retx queue
     * to send
     */
    bcf             [c2], tcp_write_xmit_done
    nop

    /* Write A = xmit_cursor_addr + xmit_cursor_offset */

    add             r2, k.to_s4_xmit_cursor_addr, k.to_s4_xmit_cursor_offset

    /* Write L = min(mss, descriptor entry len) */
    slt             c1, k.to_s4_rcv_mss, k.to_s4_xmit_cursor_len
    add.c1          r6, k.to_s4_rcv_mss, r0
    add.!c1         r6, k.to_s4_xmit_cursor_len, r0

    phvwri          p.data_dma_dma_cmd_type, CAPRI_DMA_COMMAND_MEM_TO_PKT
    phvwr           p.data_dma_dma_cmd_addr, r2
    phvwr           p.data_dma_dma_cmd_size, r6
    phvwri          p.data_dma_dma_pkt_eop, 1
bytes_sent_stats_update_start:
    addi            r6, r0, ETH_IP_TCP_HDR_SIZE
    CAPRI_STATS_INC(bytes_sent, 16, r6, d.bytes_sent)
bytes_sent_stats_update:
    CAPRI_STATS_INC_UPDATE(r1, d.bytes_sent, p.to_s5_bytes_sent)
bytes_sent_stats_update_end:

pkts_sent_stats_update_start:
    CAPRI_STATS_INC(pkts_sent, 16, 1, d.pkts_sent)
pkts_sent_stats_update:
    CAPRI_STATS_INC_UPDATE(r1, d.pkts_sent, p.to_s5_pkts_sent)
pkts_sent_stats_update_end:

debug_num_phv_to_pkt_stats_update_start:
    CAPRI_STATS_INC(debug_num_phv_to_pkt, 16, 2, d.debug_num_phv_to_pkt)
debug_num_phv_to_pkt_stats_update:
    CAPRI_STATS_INC_UPDATE(r1, d.debug_num_phv_to_pkt, p.to_s5_debug_num_phv_to_pkt)
debug_num_phv_to_pkt_stats_update_end:

debug_num_mem_to_pkt_stats_update_start:
    CAPRI_STATS_INC(debug_num_mem_to_pkt, 16, 2, d.debug_num_mem_to_pkt)
debug_num_mem_to_pkt_stats_update:
    CAPRI_STATS_INC_UPDATE(r1, d.debug_num_mem_to_pkt, p.to_s5_debug_num_mem_to_pkt)
debug_num_mem_to_pkt_stats_update_end:

    /*
     * if (tcp_in_cwnd_reduction(tp))
     *    tp->cc.prr_out += 1
     */
    and             r1, k.to_s4_ca_state, (TCPF_CA_CWR | TCPF_CA_Recovery)
    seq             c1, r1, r0
    addi.!c1        r1, r0, 1
    tbladd.!c1      d.prr_out, r1

dma_cmd_write_tx2rx_shared:
    /* Set the DMA_WRITE CMD for copying tx2rx shared data from phv to mem */
    add             r5, k.common_phv_qstate_addr, TCP_TCB_TX2RX_SHARED_WRITE_OFFSET

    phvwri          p.tx2rx_dma_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    phvwri          p.tx2rx_dma_dma_cmd_eop, 1
    phvwr           p.tx2rx_dma_dma_cmd_addr, r5
    phvwri          p.tx2rx_dma_dma_cmd_phv_start_addr, TCP_PHV_TX2RX_SHARED_START
    phvwri          p.tx2rx_dma_dma_cmd_phv_end_addr, TCP_PHV_TX2RX_SHARED_END
    
    bcf             [c1], update_xmit_cursor
    nop
move_xmit_cursor:
    tbladd          d.retx_xmit_cursor, NIC_DESC_ENTRY_SIZE
    /* This clearing of xmit_cursor_addr will cause read of retx_xmit_cursor for
     * next pass after tcp-tx stage
     */
    phvwri          p.to_s4_xmit_cursor_addr, 0 // TODO : which stage needs this?
    b               tcp_read_xmit_cursor
    nop


update_xmit_cursor:
    /* Move offset of descriptor entry by xmit len */
    add             r4, k.to_s4_xmit_cursor_offset, r6

    addi            r2, r0, NIC_DESC_ENTRY_OFF_OFFSET    
    add             r1, d.retx_xmit_cursor, r2
    //memwr.h         r1, r4

    /* Decrement length of descriptor entry by xmit len */
    sub             r4, k.to_s4_xmit_cursor_len, r6

    addi            r2, r0, NIC_DESC_ENTRY_LEN_OFFSET    
    add             r1, d.retx_xmit_cursor, r2
    //memwr.h         r1, r4
    
    
tcp_write_xmit_done:
    /* Set the tot_len in ip header */
    addi            r1, r0, TCPIP_HDR_SIZE
    add             r1, r1, r6
    //phvwr         p.tot_len,r1 // TODO
    sne             c4, r7, r0
    jr.c4           r7
    add             r7, r0, r0


tcp_retx:
tcp_retx_done:

tcp_read_xmit_cursor:
#if 0
    /* Read the xmit cursor if we have zero xmit cursor addr */
    add             r1, d.retx_xmit_cursor, r0    

    phvwr           p.table_sel, TABLE_TYPE_RAW
    phvwr           p.table_mpu_entry_raw, flow_read_xmit_cursor
    phvwr           p.table_addr, r1
#endif
    
flow_tso_process_done:
    CAPRI_NEXT_TABLE0_READ_NO_TABLE_LKUP(tcp_tx_stats_stage5_start)
    nop.e
    nop

