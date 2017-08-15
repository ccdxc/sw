#define TCP_TCB_TABLE_BASE              0xbeef0000

#define TCP_TCB_TABLE_ENTRY_SIZE        512 /* 512B */
#define TCP_TCB_TABLE_ENTRY_SIZE_SHFT   9  /* 512B */
#define TCP_TCB_TX2RX_SHARED_OFFSET     0
#define TCP_TCB_RX2TX_SHARED_OFFSET     64
#define TCP_TCB_RX_OFFSET               128
#define TCP_TCB_RTT_OFFSET              192
#define TCP_TCB_FRA_OFFSET              256
#define TCP_TCB_CC_OFFSET               320
#define TCP_TCB_FC_OFFSET               384
#define TCP_TCB_WRITE_SERQ_OFFSET       448

#define TCP_TCB_TX_OFFSET               32
#define TCP_TCB_SACK_OFFSET             96
#define TCP_TCB_TSO_OFFSET              352

#define TNMDR_TABLE_BASE                0xbeef1000
#define TNMDR_TABLE_ENTRY_SIZE          8 /* 8B */
#define TNMDR_TABLE_ENTRY_SIZE_SHFT     3 /* 8B */
#define TNMDR_TABLE_SIZE                255
#define TNMDR_TABLE_SIZE_SHFT           8

#define RNMDR_TABLE_BASE                0xa4102000 
#define RNMDR_TABLE_ENTRY_SIZE          8 /* 8B */
#define RNMDR_TABLE_ENTRY_SIZE_SHFT     3 /* 8B */
#define RNMDR_TABLE_SIZE                255
#define RNMDR_TABLE_SIZE_SHFT           8

#define RNMPR_TABLE_BASE                0xa4234000 
#define RNMPR_TABLE_ENTRY_SIZE          8 /* 8B */
#define RNMPR_TABLE_ENTRY_SIZE_SHFT     3 /* 8B */
#define RNMPR_TABLE_SIZE                255
#define RNMPR_TABLE_SIZE_SHFT           8

/* Global registers */
#define SERQ_BASE                      0xbeef4000
#define SERQ_PRODUCER_IDX              0xba00ba00
#define SERQ_CONSUMER_IDX              0xba00ba08
#define SERQ_TABLE_SIZE                255
#define SERQ_TABLE_SIZE_SHFT           8

#define SESQ_BASE                      0xbeef5000
#define SESQ_PRODUCER_IDX              0xba00ba10
#define SESQ_CONSUMER_IDX              0xba00ba18
#define SESQ_PRODUCER_IDX_VAL          0xba11ba10
#define SESQ_CONSUMER_IDX_VAL          0xba11ba18
#define SESQ_ENTRY_SIZE                16 /* 16B */
#define SESQ_ENTRY_SIZE_SHFT           4  /* 16B */
#define SESQ_SIZE                      255
#define SESQ_SIZE_SHFT                 8

#define RNMPR_ALLOC_IDX                0xba00ba20
#define RNMPR_FREE_IDX                 0xba00ba28
#define RNMDR_ALLOC_IDX                0xba00ba30
#define RNMDR_FREE_IDX                 0xba00ba38

#define TNMPR_ALLOC_IDX                0xba00ba40
#define TNMPR_FREE_IDX                 0xba00ba48
#define TNMDR_ALLOC_IDX                0xba00ba50
#define TNMDR_FREE_IDX                 0xba00ba58

#define ARQ_PRODUCER_IDX               0xba00ba60
#define ARQ_CONSUMER_IDX               0xba00ba68
#define ASQ_PRODUCER_IDX               0xba00ba70
#define ASQ_CONSUMER_IDX               0xba00ba78

#define TABLE_TYPE_RAW                 0
#define TABLE_LOCK_DIS                 0
#define TABLE_LOCK_EN                  1

#define TABLE_SIZE_8_BITS              0
#define TABLE_SIZE_16_BITS             1
#define TABLE_SIZE_32_BITS             2
#define TABLE_SIZE_64_BITS             3
#define TABLE_SIZE_128_BITS            4
#define TABLE_SIZE_256_BITS            5
#define TABLE_SIZE_512_BITS            6

#define flow_sack2_process             0x00010000
#define flow_fc_process                0x00010400
#define flow_sack4_process             0x00010800
#define flow_tx_process                0x00010c00
#define flow_rx_process                0x00011000
#define flow_get_obj                   0x00011400
#define flow_aol_page_alloc_process    0x00011800


#define flow_sack1_process             0x00011c00
#define flow_cc_process                0x00012000
#define flow_read_aol_entry_process    0x00012400
#define flow_sack3_process             0x00012800
#define flow_read_se_rcv_aol_entry_process 0x00012c00
#define flow_read_xmit_cursor          0x00013000
#define flow_tso_process               0x00013400
#define flow_tx2_process               0x00013800
#define flow_rdesc_alloc_process       0x00013c00
#define flow_tdesc_alloc_process       0x00014000
#define flow_read_serq_pidx_process    0x00014400
#define flow_read_rnmdr_aidx_process   0x00014800
#define flow_read_rnmpr_aidx_process   0x00014c00
#define flow_read_tnmdr_aidx_process   0x00015000
#define flow_write_serq_process        0x00015400
#define flow_rtt_process               0x00015800
#define flow_rpage_alloc_process       0x00015c00
#define flow_fra_process               0x00016000
#define flow_sesq_read_process         0x00016400
#define flow_queue_sched_process       0x00016800
#define flow_sesq_consume_process      0x00016c00

#define tcp_incr_quickack              0x00040110
#define tcp_ecn_check_ce               0x000404c0
#define tcp_rearm_rto                  0x000e0000
#define tcp_replace_ts_recent          0x000f0000
#define tcp_enter_quickack_mode        0x0001f000
#define tcp_write_wakeup               0x000400c0
#define tcp_mtu_probe                  0x00040198
#define tcp_send_probe0                0x00023000
#define tcp_retxq_consume              0x000403c8
#define tcp_retx_enqueue               0x000402b8
#define tcp_clean_retx_queue           0x00025000
#define tcp_cwnd_test                  0x000401d8
#define tcp_snd_wnd_test               0x00040260
#define tcp_write_xmit                 0x00040038


