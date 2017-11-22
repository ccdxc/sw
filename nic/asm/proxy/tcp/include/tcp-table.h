#define TCP_TCB_TABLE_BASE              0xbeef0000

// NOTE : **Change to this size needs to reflect in
// P4PD_HBM_TCP_CB_ENTRY_SIZE in p4pd_tcp_proxy_api.h
#define TCP_TCB_TABLE_ENTRY_SIZE        1024 /* 1024B */
#define TCP_TCB_TABLE_ENTRY_SIZE_SHFT   10  /* 1024B */

// NOTE : ** These offsets need to match the offsets in 
// tcp_proxy_api.h **
#define TCP_TCB_RX2TX_SHARED_OFFSET     0
#define TCP_TCB_RX2TX_SHARED_WRITE_OFFSET \
                    (TCP_TCB_RX2TX_SHARED_OFFSET + 42)  // skip intrinsic part etc
#define TCP_TCB_TX2RX_SHARED_OFFSET     64
#define TCP_TCB_TX2RX_SHARED_WRITE_OFFSET \
                    (TCP_TCB_TX2RX_SHARED_OFFSET + 32)  // skip intrinsic part etc
#define TCP_TCB_RX2TX_SHARED_EXTRA_OFFSET     128
#define TCP_TCB_RX_OFFSET               192
#define TCP_TCB_RTT_OFFSET              256
#define TCP_TCB_FRA_OFFSET              320
#define TCP_TCB_RETX_OFFSET             384
#define TCP_TCB_FC_OFFSET               448
#define TCP_TCB_WRITE_SERQ_OFFSET       512

#define TCP_TCB_CC_AND_XMIT_OFFSET      576
#define TCP_TCB_TSO_OFFSET              640
#define TCP_TCB_HEADER_TEMPLATE_OFFSET  704
#define TCP_TCB_RX_STATS_OFFSET         768
#define TCP_TCB_TX_STATS_OFFSET         896
#define TCP_TCB_WRITE_L7Q_OFFSET        960
#define TCP_TCB_SACK_OFFSET             96

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


#define tcp_write_wakeup               0x000400c0
#define tcp_mtu_probe                  0x00040198
#define tcp_send_probe0                0x00023000
#define tcp_retxq_consume              0x000403c8


