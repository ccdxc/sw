#ifndef __TCP_PHV_H__
#define __TCP_PHV_H__

#include "tcp-shared-state.h"

// PHV Layout
// Total 8 flits. Each flit is 512b (i.e., 64B). Total PHV is 4Kb (i.e., 512B)

// Flit 7:  4095b - 3584b   (DMA Instructions 4-7)  
// Flit 6:  3583b - 3072b   (DMA Instructions 0-3)
// Flit 5:  3071b - 2560b   (Headers - 64B)    
// Flit 4:  2559b - 2048b   (Headers - 32B)
// Flit 3:  2047b - 1536b
// Flit 2:  1535b - 1024b
// Flit 1:  1023b - 512b
// Flit 0:  511b  - 0b (INTRINSIC DATA)

#define TCP_PHV_DB_DATA_START       CAPRI_PHV_START_OFFSET(db_data_pid)
#define TCP_PHV_DB_DATA_END         CAPRI_PHV_END_OFFSET(db_data_index)
#define TCP_PHV_DB_DATA_SIZE        8

#define TCP_PHV_AOL_START           512
#define TCP_PHV_AOL_END             640
#define TCP_PHV_AOL_SIZE            64

#define TCP_PHV_AOL_DESC_START      CAPRI_PHV_START_OFFSET(aol_A0)
#define TCP_PHV_AOL_DESC_END        CAPRI_PHV_END_OFFSET(aol_next_pkt)

#define TCP_PHV_RING_ENTRY_DESC_ADDR_START CAPRI_PHV_START_OFFSET(ring_entry_descr_addr)
#define TCP_PHV_RING_ENTRY_DESC_ADDR_END CAPRI_PHV_END_OFFSET(ring_entry_descr_addr)

#define TCP_PHV_ETH_SIZE            112  //14B
#define TCP_PHV_IP_SIZE             160  //20B
#define TCP_PHV_TCP_SIZE            512  //32B
#define TCP_PHV_ALL_HDR_SIZE        (TCP_PHV_ETH_SIZE + TCP_PHV_IP_SIZE + TCP_PHV_TCP_SIZE)

#define TCP_PHV_HEADERS_START       2304
#define TCP_PHV_ETH_START           TCP_PHV_HEADERS_START
#define TCP_PHV_IP_START            (TCP_PHV_ETH_START+TCP_PHV_ETH_SIZE-1)
#define TCP_PHV_TCP_START           (TCP_PHV_IP_START+ROCEV2_PHV_IP_SIZE-1)
#define TCP_PHV_HEADERS_END         (TCP_PHV_TCP_START+TCP_PHV_TCP_SIZE-1)

#define TCP_PHV_RX2TX_SHARED_START  CAPRI_PHV_START_OFFSET(rx2tx_snd_una)
#define TCP_PHV_RX2TX_SHARED_END    CAPRI_PHV_END_OFFSET(rx2tx_pad1_rx2tx)

#define TCP_PHV_RX2TX_SHARED_EXTRA_START  CAPRI_PHV_START_OFFSET(rx2tx_extra_ato_deadline)
#define TCP_PHV_RX2TX_SHARED_EXTRA_END    CAPRI_PHV_END_OFFSET(rx2tx_extra__padding)



#define TCP_PHV_TX2RX_SHARED_START  CAPRI_PHV_START_OFFSET(tx2rx_prr_out)
#define TCP_PHV_TX2RX_SHARED_SIZE   512
#define TCP_PHV_TX2RX_SHARED_END    CAPRI_PHV_END_OFFSET(tx2rx_pad2_tx2rx)

//#define TCP_PHV_HEADERS_END      3088
#define TCP_PHV_TXDMA_COMMANDS_START    (CAPRI_PHV_START_OFFSET(intrinsic_dma_dma_cmd_type) / 16)
#define TCP_PHV_RXDMA_COMMANDS_START    (CAPRI_PHV_START_OFFSET(pkt_dma_dma_cmd_type) / 16)
#define TCP_PHV_DMA_COMMANDS_END        6144
#define TCP_PHV_DMA_COMMAND_ADDR_OFFSET 0
#define TCP_PHV_DMA_COMMAND_ADDR_LEN    64
#define TCP_PHV_DMA_COMMAND_PAD_OFFSET  (TCP_PHV_DMA_COMMAND_ADDR_OFFSET+TCP_PHV_DMA_COMMAND_ADDR_LEN)
#define TCP_PHV_DMA_COMMAND_PAD_LEN     40
#define TCP_PHV_DMA_COMMAND_SIZE_OFFSET (TCP_PHV_DMA_COMMAND_PAD_OFFSET+TCP_PHV_DMA_COMMAND_PAD_LEN)
#define TCP_PHV_DMA_COMMAND_SIZE_LEN    16
#define TCP_PHV_DMA_COMMAND_CMD_OFFESET (TCP_PHV_DMA_COMMAND_SIZE_OFFSET+TCP_PHV_DMA_COMMAND_SIZE_LEN)
#define TCP_PHV_DMA_COMMAND_CMD_LEN     8
#define TCP_PHV_DMA_COMMAND_TOTAL_LEN   (TCP_PHV_DMA_COMMAND_CMD_OFFESET+TCP_PHV_DMA_COMMAND_CMD_LEN)

#define TCP_PHV_INTRINSIC_START         CAPRI_PHV_START_OFFSET(p4_intr_global_tm_iport)
#define TCP_PHV_INTRINSIC_END           CAPRI_PHV_END_OFFSET(p4_intr_global_tm_instance_type)

#define TCP_PHV_TXDMA_INTRINSIC_START   CAPRI_PHV_START_OFFSET(p4_txdma_intr_qid)
#define TCP_PHV_TXDMA_INTRINSIC_END     CAPRI_PHV_END_OFFSET(p4_txdma_intr_txdma_rsv)

#define TCP_PHV_TX_APP_HDR_START        CAPRI_PHV_START_OFFSET(app_header_app_type)
#define TCP_PHV_TX_APP_HDR_END          (CAPRI_PHV_START_OFFSET(app_header_app_type) + 9) // TODO: end offset not working on unionized struct due to padding issue

#define TCP_PHV_TX_TCP_HDR_START        CAPRI_PHV_START_OFFSET(tcp_header_source_port)
#define TCP_PHV_TX_TCP_HDR_END          CAPRI_PHV_END_OFFSET(tcp_header_urg)

struct p_struct {
        // DMA COMMANDS - END
        dma_cmd0_cmd                     :  8;
        dma_cmd0_size                    :  16;
        pad0                             :  16;
        dma_cmd0_pad                     :  40;
        dma_cmd0_addr                    :  64;

        pad1                             : 120;

        dma_cmd1_cmd                     :  8;
        dma_cmd1_size                    :  16;
        dma_cmd1_pad                     :  40;
        dma_cmd1_addr                    :  64;
        pad11                            :  64;
        dma_cmd2_cmd                     :  8;
        dma_cmd2_size                    :  16;
        dma_cmd2_pad                     :  40;
        dma_cmd2_addr                    :  64;

  
        dma_cmd3_cmd                     :  8;
        dma_cmd3_size                    :  16;
        dma_cmd3_pad                     :  40;
        dma_cmd3_addr                    :  64;

        dma_cmd4_cmd                     :  8;
        dma_cmd4_size                    :  16;
        dma_cmd4_pad                     :  40;
        dma_cmd4_addr                    :  64;

        dma_cmd5_cmd                     :  8;
        dma_cmd5_size                    :  16;
        dma_cmd5_pad                     :  40;
        dma_cmd5_addr                    :  64;

	// DMA COMMANDS - START

        TCB_RX2TX_SHARED_STATE

        TCB_RX2TX_SHARED_EXTRA_STATE

        TCB_TX2RX_SHARED_STATE

	// OTHER PHV - END

        source                          : 16;
        dest                            : 16;
        pad21                           : 16;
        seq                             : 32;
        ack_seq                         : 32;
        res1                            : 4;
        d_off                           : 4;
        fin                             : 1;
        syn                             : 1;
        rst                             : 1;
        psh                             : 1;
        ack                             : 1;
        urg                             : 1;
        ece                             : 1;
        cwr                             : 1;
        window                          : 16;
        pad22                           : 16;
        check                           : 16;
        urg_ptr                         : 16;

	ooo_rcv				: 1  ;
	rsvd				: 4 ;
	ca_event			: 4 ;
	sack_off			: 8 ;

	ts_off				: 8 ;
	ip_dsfield			: 8 ;
	pkts_acked			: 8  ;


	process_ack_flag		: 16  ;
	descr_idx			: 8 ;
	page_idx			: 8 ;
	descr				: 16 ;
	page				: 16 ;

	timer_event			: 1  ;
	asq_event			: 1  ;
	txq_event			: 1  ;
        is_dupack                       : 1  ;

	xmit_cursor_flags		: 8  ;



        serq_pidx                       : 7  ;
        write_serq                      : 1  ;
	fid				: 16 ;
	flags				: 8  ;
	addr				: 16 ;
	offset				: 16 ;
	len				: 16 ;
	sesq_event			: 1  ;

        rcv_cursor_scratch              : 64;
        rcv_cursor_free_pending         : 1;
        rcv_cursor_valid                : 1;
        rcv_cursor_addr		        : 16 ;
        pad3                            : 16 ;
	rcv_cursor_offset		: 16 ;
	rcv_cursor_len			: 16 ;

        aol_zero_scratch                : 1;
        aol_page_alloc                  : 1;
        aol_desc                        : 16;
        aol_page                        : 16;
        aol_scratch                     : 64;
        pad4                            : 64;
        aol_free_pending                : 1;
        aol_valid                       : 1;
        aol_addr                        : 30;
        aol_offset                      : 16;
        aol_len                         : 16;

	xmit_cursor_addr		: 8;
	xmit_cursor_offset		: 8 ;
	xmit_cursor_len			: 8 ;
        tot_len                         : 8;

        RNMDR_alloc_idx                 : 16;
        RNMPR_alloc_idx                 : 16;
        SERQ_pidx                       : 16;
        SESQ_cidx                       : 16;
        db_data                         : 64;
        seqe_fid                        : 16;
        seqe_desc                       : ADDRESS_WIDTH;
	// OTHER PHV - START


	// INTRINSIC - END
        table_sel                       : 4; // RAW, NONE, or configured table ID
        table_addr                      : 64;
        table_size                      : 6;
        table_key_phv                   : 8;
        table_mpu_entry_raw             : 26;
	// pointer to first DMA command
        dma_cmd_ptr                     : 8;
        sched_id                        : 17;

        type                            : 4;
        vf                              : 11;
        drop                            : 1;
        error                           : 1;
        resubmit                        : 1;
        pkt_type                        : 4;
        tm_oport                        : 8;
        tm_oq                           : 5;
        tm_iq                           : 5;
        tm_rep_en                       : 1;
        tm_rep                          : 12;
        timestamp                       : 32;
        tm_q_depth                      : 16;
	// INTRINSIC - START
};

#endif /* #ifndef __TCP_PHV_H__ */
