#include "tls-shared-state.h"

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

#define TLS_PHV_DB_DATA_START       432
#define TLS_PHV_DB_DATA_SIZE        64
#define TLS_PHV_AOL_START           512
#define TLS_PHV_AOL1_START          520
#define TLS_PHV_SEQE_START          640
#define TLS_PHV_BRQ_REQ_START       652
#define TLS_PHV_CUR_TLS_DATA_LEN_START 680
#define TLS_PHV_CUR_TLS_DATA_LEN_SIZE 4

#define TLS_PHV_TLSH_START          700

#define TLS_PHV_ETH_SIZE            112  //14B
#define TLS_PHV_IP_SIZE             160  //20B
#define TLS_PHV_TLS_SIZE            512  //32B
#define TLS_PHV_ALL_HDR_SIZE        (TLS_PHV_ETH_SIZE + TLS_PHV_IP_SIZE + TLS_PHV_TLS_SIZE)

#define TLS_PHV_HEADERS_START       2304
#define TLS_PHV_ETH_START           TLS_PHV_HEADERS_START
#define TLS_PHV_IP_START            (TLS_PHV_ETH_START+TLS_PHV_ETH_SIZE-1)
#define TLS_PHV_TLS_START           (TLS_PHV_IP_START+ROCEV2_PHV_IP_SIZE-1)
#define TLS_PHV_HEADERS_END         (TLS_PHV_TLS_START+TLS_PHV_TLS_SIZE-1)

#define TLS_HDR_SIZE                5
//#define TLS_PHV_HEADERS_END      3088
#define TLS_PHV_DMA_COMMAND_ADDR_OFFSET 0
#define TLS_PHV_DMA_COMMAND_ADDR_LEN    64
#define TLS_PHV_DMA_COMMAND_PAD_OFFSET  (TLS_PHV_DMA_COMMAND_ADDR_OFFSET+TLS_PHV_DMA_COMMAND_ADDR_LEN)
#define TLS_PHV_DMA_COMMAND_PAD_LEN     40
#define TLS_PHV_DMA_COMMAND_SIZE_OFFSET (TLS_PHV_DMA_COMMAND_PAD_OFFSET+TLS_PHV_DMA_COMMAND_PAD_LEN)
#define TLS_PHV_DMA_COMMAND_SIZE_LEN    16
#define TLS_PHV_DMA_COMMAND_CMD_OFFESET (TLS_PHV_DMA_COMMAND_SIZE_OFFSET+TLS_PHV_DMA_COMMAND_SIZE_LEN)
#define TLS_PHV_DMA_COMMAND_CMD_LEN     8
#define TLS_PHV_DMA_COMMAND_TOTAL_LEN   (TLS_PHV_DMA_COMMAND_CMD_OFFESET+TLS_PHV_DMA_COMMAND_CMD_LEN)

#define TLS_PHV_DMA_COMMANDS_START      (CAPRI_PHV_START_OFFSET(dma_cmd0_dma_cmd_type) / 16)

struct p_struct {
        // DMA COMMANDS - END
        dma_cmd0_cmd                     :  8;
        dma_cmd0_size                    :  16;
        dma_cmd0_pad                     :  40;
        dma_cmd0_addr                    :  64;

        pad1                             : 120;

        dma_cmd1_cmd                     :  8;
        dma_cmd1_size                    :  16;
        dma_cmd1_pad                     :  40;
        dma_cmd1_addr                    :  64;


        dma_cmd2_cmd                     :  8;
        dma_cmd2_size                    :  16;
        dma_cmd2_pad                     :  40;
        dma_cmd2_addr                    :  64;
  

  
        dma_cmd3_cmd                     :  8;
        dma_cmd3_size                    :  16;
        dma_cmd3_pad                     :  40;
        dma_cmd3_addr                    :  64;
        pad4                             :  64;
	// DMA COMMANDS - START

	// OTHER PHV - END
        fid                              : 16;
        fid_pad                          : 16;
        ofid                             : 16;
        desc                             : ADDRESS_WIDTH;
        desc_pad                         : ADDRESS_WIDTH;
        odesc                            : ADDRESS_WIDTH;
        sdesc                            : ADDRESS_WIDTH;
        opage                            : ADDRESS_WIDTH;
        pad41                            : ADDRESS_WIDTH;
        desc_scratch                     :  28;
        desc_num_entries                 :  2;
        desc_data_len                    :  18;
        desc_head_idx                    :  2;
        desc_tail_idx                    :  2;
        desc_offset                      :  8;

        desc_aol0_scratch                :  64;
        desc_aol0_free_pending           :  1;
        desc_aol0_valid                  :  1;
        desc_aol0_addr                   :  30;
        desc_aol0_offset                 :  16;
        desc_aol0_len                    :  16;

        desc_aol1_scratch                :  64;
        desc_aol1_free_pending           :  1;
        desc_aol1_valid                  :  1;
        desc_aol1_addr                   :  30;
        desc_aol1_offset                 :  16;
        desc_aol1_len                    :  16;

        seqe_fid                         :  16;
        seqe_desc                        :  ADDRESS_WIDTH;

        brq_idesc                        : ADDRESS_WIDTH;
        brq_odesc                        : ADDRESS_WIDTH;
        brq_key                          : ADDRESS_WIDTH;
        brq_iv                           : ADDRESS_WIDTH;
        brq_pad                          : ADDRESS_WIDTH;
        brq_auth_tag                     : ADDRESS_WIDTH;
  
        brq_cmd_core                     : 3;
        brq_cmd_keysize                  : 3;
        pad5                             : 16;
        brq_cmd_mode                     : 3;
        brq_cmd_hash                     : 3;
        brq_cmd_op                       : 3;
        brq_cmd_rsvd                     : 17;

        brq_payload_offset               : 16;
        brq_status                       : 32;
        pad6                             : 32;
        brq_opaque_tag                   : 32;
        brq_rsvd                         : 64;


        next_tls_hdr_offset              : 16;
        cur_tls_data_len                 : 16;

        brq_pidx                         : 8;
        sesq_pidx                        : 8;
        
        pending_rx_brq                   : 1;
        pending_rx_serq                  : 1;
        pending_queue_brq                : 1;
        enc_flow                         : 1;
        split                            : 1;

        tls_hdr_type                     : 8;
        tls_hdr_version_major            : 8;
        tls_hdr_version_minor            : 8;
        tls_hdr_len                      : 16;

        db_data                          : 64;

        tcp_app_header_table0_valid      : 1;
        tcp_app_header_table1_valid      : 1;
        tcp_app_header_table2_valid      : 1;
        tcp_app_header_table3_valid      : 1;
        common_te0_phv_table_lock_en     : 1; 
        common_te0_phv_table_raw_table_size : 3;
        common_te0_phv_table_pc : 28; 
        common_te0_phv_table_addr : 64; 
        common_te1_phv_table_lock_en     : 1; 
        common_te1_phv_table_raw_table_size : 3;
        common_te1_phv_table_pc : 28; 
        common_te1_phv_table_pc_pad : 28; 
        common_te1_phv_table_addr : 64; 
        common_te2_phv_table_lock_en     : 1; 
        common_te2_phv_table_raw_table_size : 3;
        common_te2_phv_table_pc : 28; 
        common_te2_phv_table_addr : 64; 
        common_te3_phv_table_lock_en     : 1; 
        common_te3_phv_table_raw_table_size : 3;
        common_te3_phv_table_pc : 28; 
        common_te3_phv_table_addr : 64; 
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

