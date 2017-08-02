/* 
 * This contains all the fields shared across TX and RX pipeline stages for TCP.
 * Each field has a unique writing stage
 */

#define SEQ_NUMBER_WIDTH                32
#define TS_WIDTH                        32
#define RING_INDEX_WIDTH                16
#define ADDRESS_WIDTH                   16
#define OFFSET_WIDTH                    16
#define LEN_WIDTH                       16
#define COUNTER32                       32
#define COUNTER16                       16
#define COUNTER8                        8
#define WINDOW_WIDTH                    16
#define MTU_WIDTH                       8


#define TCB_RX2TX_SHARED_STATE \
	snd_una				: SEQ_NUMBER_WIDTH	;\
	rcv_nxt				: SEQ_NUMBER_WIDTH	;\
	snd_wnd				: SEQ_NUMBER_WIDTH	;\
	snd_wl1				: SEQ_NUMBER_WIDTH	;\
	snd_up				: SEQ_NUMBER_WIDTH	;\
        write_seq                       : SEQ_NUMBER_WIDTH      ;\
        tso_seq                         : SEQ_NUMBER_WIDTH      ;\
        snd_cwnd                        : WINDOW_WIDTH          ;\

#define TCB_RX2TX_SHARED_EXTRA_STATE \
	pending_challenge_ack_send	: 1	                ;\
	pending_ack_send		: 1	                ;\
	pending_sync_mss		: 1	                ;\
	pending_tso_keepalive           : 1                     ;\
	pending_tso_pmtu_probe          : 1                     ;\
	pending_tso_data		: 1                     ;\
	pending_tso_probe_data		: 1                     ;\
	pending_tso_probe		: 1                     ;\
	pending_ooo_se_recv		: 1                     ;\
	pending_tso_retx	        : 1                     ;\
	pending_ft_clear	        : 1                     ;\
	pending_ft_reset	        : 1                     ;\
	pending_rexmit		        : 2                     ;\
        pending		                : 3                     ;\
	ack_blocked			: 1	                ;\
	ack_pending			: 3	                ;\
	snd_wscale			: 4	                ;\
	rcv_mss_shft			: 4	                ;\
        rcv_mss                         : 8                     ;\
	rto				: 8	                ;\
	ca_state			: 8	                ;\
	ato_deadline			: TS_WIDTH              ;\
	rto_deadline			: TS_WIDTH	        ;\
	retx_head_ts			: TS_WIDTH	        ;\
	srtt_us				: TS_WIDTH              ;\
	rcv_wnd				: WINDOW_WIDTH          ;\
        rx2tx_shared_extra_pad1         : WINDOW_WIDTH          ;\
	prior_ssthresh			: WINDOW_WIDTH	        ;\
	high_seq			: SEQ_NUMBER_WIDTH	;\
	sacked_out			: COUNTER16	        ;\
	lost_out			: COUNTER8	        ;\
	retrans_out			: COUNTER8	        ;\
	fackets_out			: COUNTER16	        ;\
	ooo_datalen			: COUNTER16 	        ;\
	quick				: 4	                ;\
	pingpong			: 1                     ;\
	backoff				: 4                     ;\
	dsack			        : 1                     ;\
	num_sacks			: 8                     ;\
	reordering			: COUNTER32	        ;\
	undo_marker			: SEQ_NUMBER_WIDTH	;\
	undo_retrans			: SEQ_NUMBER_WIDTH	;\
        snd_ssthresh			: WINDOW_WIDTH	        ;\
	loss_cwnd			: WINDOW_WIDTH	        ;\
	ecn_flags			: 8	                ;

#define TCB_TX2RX_SHARED_STATE \
        prr_out				: COUNTER32	        ;\
	snd_nxt				: SEQ_NUMBER_WIDTH	;\
	ecn_flags_tx			: 8	                ;\
	packets_out			: COUNTER16	        ;

#define TCB_FLOW_TUNABLES

#define TCB_GLOBAL_TUNABLES

#define TCB_TX_SHARED_STATE \
	retx_ci                         : 16		   ;     \
	retx_pi                         : 16		   ;     \
        sched_flag                      : 8		   ;     \
	retx_snd_una			: SEQ_NUMBER_WIDTH ;     \
	retx_snd_nxt			: SEQ_NUMBER_WIDTH ;     \
	retx_head_desc			: ADDRESS_WIDTH ;        \
	retx_snd_una_cursor		: ADDRESS_WIDTH ;        \
	retx_tail_desc			: ADDRESS_WIDTH ;        \
	retx_snd_nxt_cursor		: ADDRESS_WIDTH ;        \
	retx_xmit_cursor		: ADDRESS_WIDTH ;        \
	xmit_cursor_addr		: ADDRESS_WIDTH ;        \
	rcv_wup				: WINDOW_WIDTH ;         \
	pending_ack_tx			: 1  ;                   \
	pending_delayed_ack_tx		: 1  ;                   \
	pending_tso_data		: 1                     ;
