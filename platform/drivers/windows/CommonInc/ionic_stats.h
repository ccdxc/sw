#ifndef _IONIC_STATS_H
#define _IONIC_STATS_H

#define MAX_QUEUE_PER_LIF_COUNT		64
#define MAX_LIF_COUNT				32
#define MAX_PORT_COUNT				10

#define LIF_NAME_MAX_SZ				32

struct global_stats {

	LARGE_INTEGER	perf_frequency;

	ULONG			tick_count_interval;

	LONG			interrupt_cnt;

	LONGLONG		interrupt_time;

	LONGLONG		tx_int_time;

	LONG			tx_miss;

	LONGLONG		tx_miss_time;

	LONGLONG		tx_total_time;

	LONGLONG		tx_send_time;

	LONGLONG		tx_send_sub1_time;
	
	LONGLONG		tx_send_sub2_time;
	
	LONGLONG		tx_send_sub3_time;

	LONGLONG		tx_sg_time;

	LONGLONG		tx_sg_sub1_time;
	
	LONGLONG		tx_sg_sub2_time;
	
	LONGLONG		tx_sg_sub3_time;
	
	LONGLONG		tx_sg_sub4_time;

	LONGLONG		tx_sg_sub5_time;

	LONGLONG		tx_sg_sub6_time;

	LONGLONG		tx_sg_sub7_time;

	LONGLONG		tx_sg_sub8_time;

	LONGLONG		tx_sg_sub9_time;

	LONGLONG		tx_sg_sub10_time;

	LONGLONG		tx_sg_sub11_time;

	LONGLONG		tx_sg_sub12_time;

	LONGLONG		tx_sg_sub13_time;

	LONGLONG		tx_bytes;

	LONG			tx_packets;

	LONGLONG		rx_int_time;

	LONGLONG		rx_wait_time;

	LONGLONG		rx_walk_time;

	LONGLONG		rx_walk_sub1_time;

	LONGLONG		rx_walk_sub2_time;

	LONGLONG		rx_walk_sub3_time;

	LONGLONG		rx_walk_sub4_time;

	LONGLONG		rx_walk_sub5_time;

	LONGLONG		rx_walk_sub6_time;
	
	LONGLONG		rx_walk_sub7_time;

	LONGLONG		rx_walk_sub8_time;
	
	LONGLONG		rx_walk_sub9_time;
	
	LONGLONG		rx_walk_sub10_time;
	
	LONGLONG		rx_fill_time;

	LONGLONG		rx_fill_sub1_time;

	LONGLONG		rx_fill_sub2_time;
	
	LONGLONG		rx_fill_sub3_time;
	
	LONGLONG		rx_ind_time;

	LONG			rx_miss;

	LONGLONG		rx_miss_time;

	LONGLONG		rx_total_time;

	LONGLONG		rx_bytes;

	LONG			rx_packets;

};

struct dev_rx_ring_stats {

	__le32		msi_id;

	__le64		poll;
	__le64		arm;
	__le64		completion_count;
	__le64		buffers_posted;

	__le64		directed_bytes;
	__le64		directed_packets;

	__le64		bcast_bytes;
	__le64		bcast_packets;

	__le64		mcast_bytes;
	__le64		mcast_packets;

	__le64		lro_packets;

	__le64		csum_none;
	__le64		csum_complete;
	__le64		csum_verified;
	__le64		csum_ip;
	__le64		csum_ip_bad;
	__le64		csum_udp;
	__le64		csum_udp_bad;
	__le64		csum_tcp;
	__le64		csum_tcp_bad;
	__le64		vlan_stripped;

    // stats for perfmon interface
    __le32      pool_packet_count;
    __le32      pool_sample_count;
};

struct dev_tx_ring_stats {

	__le64		full;
	__le64		wake;
	__le64		no_descs;
	__le64		doorbell_count;
	__le64		comp_count;

	__le64		directed_bytes;
	__le64		directed_packets;

	__le64		bcast_bytes;
	__le64		bcast_packets;

	__le64		mcast_bytes;
	__le64		mcast_packets;

	__le64		tso_packets;
	__le64		tso_bytes;
	__le64		encap_tso_packets;
	__le64		encap_tso_bytes;
	__le64		csum_none;
	__le64		csum_partial;
	__le64		csum_partial_inner;
	__le64		csum_hw;
	__le64		csum_hw_inner;
	__le64		vlan_inserted;

	__le64		dma_map_error;

    // stats for perfmon interface
    __le32      nbl_count;
    __le32      nb_count;

    __le32      pending_nbl_count;
    __le32      pending_nb_count;

    __le32      descriptor_count;
    __le32      descriptor_sample;
    __le32      descriptor_max;

};

#define IONIC_LIF_FLAG_ALLOCATED		0x00000001
#define IONIC_LIF_FLAG_VF_CONNECTED		0x00000002

#define IONIC_LIF_TYPE_DEFAULT			0x00000000
#define IONIC_LIF_TYPE_VMQ				0x00000001
#define IONIC_LIF_TYPE_VPORT			0x00000002

struct dev_lif_stats {

	__le32		flags;

	__le32		lif_id;
	__le32		lif_type;

	char		lif_name[LIF_NAME_MAX_SZ];

	__le32		rx_count;
	__le32		tx_count;

	__le32		rx_pool_alloc_cnt;

	__le32		rx_pool_free_cnt;

	__le64		rx_pool_alloc_time;
	
	__le64		rx_pool_free_time;

	struct dev_tx_ring_stats tx_ring[ MAX_QUEUE_PER_LIF_COUNT];

	struct dev_rx_ring_stats rx_ring[ MAX_QUEUE_PER_LIF_COUNT];
};

#define IONIC_PORT_FLAG_RSS					0x00000001
#define IONIC_PORT_FLAG_VMQ					0x00000002
#define IONIC_PORT_FLAG_SRIOV				0x00000004

struct dev_port_stats {

	__le32		flags;

	__le16		vendor_id;
	__le16		device_id;

	__le64		link_up;
	__le64		link_dn;

	__le32		lif_count;

	struct dev_lif_stats		lif_stats[ MAX_LIF_COUNT];
};

/**
 * struct port_stats
 */
struct port_stats {
	__le64 frames_rx_ok;
	__le64 frames_rx_all;
	__le64 frames_rx_bad_fcs;
	__le64 frames_rx_bad_all;
	__le64 octets_rx_ok;
	__le64 octets_rx_all;
	__le64 frames_rx_unicast;
	__le64 frames_rx_multicast;
	__le64 frames_rx_broadcast;
	__le64 frames_rx_pause;
	__le64 frames_rx_bad_length;
	__le64 frames_rx_undersized;
	__le64 frames_rx_oversized;
	__le64 frames_rx_fragments;
	__le64 frames_rx_jabber;
	__le64 frames_rx_pripause;
	__le64 frames_rx_stomped_crc;
	__le64 frames_rx_too_long;
	__le64 frames_rx_vlan_good;
	__le64 frames_rx_dropped;
	__le64 frames_rx_less_than_64b;
	__le64 frames_rx_64b;
	__le64 frames_rx_65b_127b;
	__le64 frames_rx_128b_255b;
	__le64 frames_rx_256b_511b;
	__le64 frames_rx_512b_1023b;
	__le64 frames_rx_1024b_1518b;
	__le64 frames_rx_1519b_2047b;
	__le64 frames_rx_2048b_4095b;
	__le64 frames_rx_4096b_8191b;
	__le64 frames_rx_8192b_9215b;
	__le64 frames_rx_other;
	__le64 frames_tx_ok;
	__le64 frames_tx_all;
	__le64 frames_tx_bad;
	__le64 octets_tx_ok;
	__le64 octets_tx_total;
	__le64 frames_tx_unicast;
	__le64 frames_tx_multicast;
	__le64 frames_tx_broadcast;
	__le64 frames_tx_pause;
	__le64 frames_tx_pripause;
	__le64 frames_tx_vlan;
	__le64 frames_tx_less_than_64b;
	__le64 frames_tx_64b;
	__le64 frames_tx_65b_127b;
	__le64 frames_tx_128b_255b;
	__le64 frames_tx_256b_511b;
	__le64 frames_tx_512b_1023b;
	__le64 frames_tx_1024b_1518b;
	__le64 frames_tx_1519b_2047b;
	__le64 frames_tx_2048b_4095b;
	__le64 frames_tx_4096b_8191b;
	__le64 frames_tx_8192b_9215b;
	__le64 frames_tx_other;
	__le64 frames_tx_pri_0;
	__le64 frames_tx_pri_1;
	__le64 frames_tx_pri_2;
	__le64 frames_tx_pri_3;
	__le64 frames_tx_pri_4;
	__le64 frames_tx_pri_5;
	__le64 frames_tx_pri_6;
	__le64 frames_tx_pri_7;
	__le64 frames_rx_pri_0;
	__le64 frames_rx_pri_1;
	__le64 frames_rx_pri_2;
	__le64 frames_rx_pri_3;
	__le64 frames_rx_pri_4;
	__le64 frames_rx_pri_5;
	__le64 frames_rx_pri_6;
	__le64 frames_rx_pri_7;
	__le64 tx_pripause_0_1us_count;
	__le64 tx_pripause_1_1us_count;
	__le64 tx_pripause_2_1us_count;
	__le64 tx_pripause_3_1us_count;
	__le64 tx_pripause_4_1us_count;
	__le64 tx_pripause_5_1us_count;
	__le64 tx_pripause_6_1us_count;
	__le64 tx_pripause_7_1us_count;
	__le64 rx_pripause_0_1us_count;
	__le64 rx_pripause_1_1us_count;
	__le64 rx_pripause_2_1us_count;
	__le64 rx_pripause_3_1us_count;
	__le64 rx_pripause_4_1us_count;
	__le64 rx_pripause_5_1us_count;
	__le64 rx_pripause_6_1us_count;
	__le64 rx_pripause_7_1us_count;
	__le64 rx_pause_1us_count;
	__le64 frames_tx_truncated;
};

struct mgmt_port_stats {
	__le64 frames_rx_ok;
	__le64 frames_rx_all;
	__le64 frames_rx_bad_fcs;
	__le64 frames_rx_bad_all;
	__le64 octets_rx_ok;
	__le64 octets_rx_all;
	__le64 frames_rx_unicast;
	__le64 frames_rx_multicast;
	__le64 frames_rx_broadcast;
	__le64 frames_rx_pause;
	__le64 frames_rx_bad_length0;
	__le64 frames_rx_undersized1;
	__le64 frames_rx_oversized2;
	__le64 frames_rx_fragments3;
	__le64 frames_rx_jabber4;
	__le64 frames_rx_64b5;
	__le64 frames_rx_65b_127b6;
	__le64 frames_rx_128b_255b7;
	__le64 frames_rx_256b_511b8;
	__le64 frames_rx_512b_1023b9;
	__le64 frames_rx_1024b_1518b0;
	__le64 frames_rx_gt_1518b1;
	__le64 frames_rx_fifo_full2;
	__le64 frames_tx_ok3;
	__le64 frames_tx_all4;
	__le64 frames_tx_bad5;
	__le64 octets_tx_ok6;
	__le64 octets_tx_total7;
	__le64 frames_tx_unicast8;
	__le64 frames_tx_multicast9;
	__le64 frames_tx_broadcast0;
	__le64 frames_tx_pause1;
};

/**
 * struct lif_stats
 */
struct lif_stats {
	/* RX */
	__le64 rx_ucast_bytes;
	__le64 rx_ucast_packets;
	__le64 rx_mcast_bytes;
	__le64 rx_mcast_packets;
	__le64 rx_bcast_bytes;
	__le64 rx_bcast_packets;
	__le64 rsvd0;
	__le64 rsvd1;
	/* RX drops */
	__le64 rx_ucast_drop_bytes;
	__le64 rx_ucast_drop_packets;
	__le64 rx_mcast_drop_bytes;
	__le64 rx_mcast_drop_packets;
	__le64 rx_bcast_drop_bytes;
	__le64 rx_bcast_drop_packets;
	__le64 rx_dma_error;
	__le64 rsvd2;
	/* TX */
	__le64 tx_ucast_bytes;
	__le64 tx_ucast_packets;
	__le64 tx_mcast_bytes;
	__le64 tx_mcast_packets;
	__le64 tx_bcast_bytes;
	__le64 tx_bcast_packets;
	__le64 rsvd3;
	__le64 rsvd4;
	/* TX drops */
	__le64 tx_ucast_drop_bytes;
	__le64 tx_ucast_drop_packets;
	__le64 tx_mcast_drop_bytes;
	__le64 tx_mcast_drop_packets;
	__le64 tx_bcast_drop_bytes;
	__le64 tx_bcast_drop_packets;
	__le64 tx_dma_error;
	__le64 rsvd5;
	/* Rx Queue/Ring drops */
	__le64 rx_queue_disabled;
	__le64 rx_queue_empty;
	__le64 rx_queue_error;
	__le64 rx_desc_fetch_error;
	__le64 rx_desc_data_error;
	__le64 rsvd6;
	__le64 rsvd7;
	__le64 rsvd8;
	/* Tx Queue/Ring drops */
	__le64 tx_queue_disabled;
	__le64 tx_queue_error;
	__le64 tx_desc_fetch_error;
	__le64 tx_desc_data_error;
	__le64 tx_queue_empty;
	__le64 rsvd10;
	__le64 rsvd11;
	__le64 rsvd12;

	/* RDMA/ROCE TX */
	__le64 tx_rdma_ucast_bytes;
	__le64 tx_rdma_ucast_packets;
	__le64 tx_rdma_mcast_bytes;
	__le64 tx_rdma_mcast_packets;
	__le64 tx_rdma_cnp_packets;
	__le64 rsvd13;
	__le64 rsvd14;
	__le64 rsvd15;

	/* RDMA/ROCE RX */
	__le64 rx_rdma_ucast_bytes;
	__le64 rx_rdma_ucast_packets;
	__le64 rx_rdma_mcast_bytes;
	__le64 rx_rdma_mcast_packets;
	__le64 rx_rdma_cnp_packets;
	__le64 rx_rdma_ecn_packets;
	__le64 rsvd16;
	__le64 rsvd17;

	__le64 rsvd18;
	__le64 rsvd19;
	__le64 rsvd20;
	__le64 rsvd21;
	__le64 rsvd22;
	__le64 rsvd23;
	__le64 rsvd24;
	__le64 rsvd25;

	__le64 rsvd26;
	__le64 rsvd27;
	__le64 rsvd28;
	__le64 rsvd29;
	__le64 rsvd30;
	__le64 rsvd31;
	__le64 rsvd32;
	__le64 rsvd33;

	__le64 rsvd34;
	__le64 rsvd35;
	__le64 rsvd36;
	__le64 rsvd37;
	__le64 rsvd38;
	__le64 rsvd39;
	__le64 rsvd40;
	__le64 rsvd41;

	__le64 rsvd42;
	__le64 rsvd43;
	__le64 rsvd44;
	__le64 rsvd45;
	__le64 rsvd46;
	__le64 rsvd47;
	__le64 rsvd48;
	__le64 rsvd49;

	/* RDMA/ROCE REQ Error/Debugs (768 - 895) */
	__le64 rdma_req_rx_pkt_seq_err;
	__le64 rdma_req_rx_rnr_retry_err;
	__le64 rdma_req_rx_remote_access_err;
	__le64 rdma_req_rx_remote_inv_req_err;
	__le64 rdma_req_rx_remote_oper_err;
	__le64 rdma_req_rx_implied_nak_seq_err;
	__le64 rdma_req_rx_cqe_err;
	__le64 rdma_req_rx_cqe_flush_err;

	__le64 rdma_req_rx_dup_responses;
	__le64 rdma_req_rx_invalid_packets;
	__le64 rdma_req_tx_local_access_err;
	__le64 rdma_req_tx_local_oper_err;
	__le64 rdma_req_tx_memory_mgmt_err;
	__le64 rsvd52;
	__le64 rsvd53;
	__le64 rsvd54;

	/* RDMA/ROCE RESP Error/Debugs (896 - 1023) */
	__le64 rdma_resp_rx_dup_requests;
	__le64 rdma_resp_rx_out_of_buffer;
	__le64 rdma_resp_rx_out_of_seq_pkts;
	__le64 rdma_resp_rx_cqe_err;
	__le64 rdma_resp_rx_cqe_flush_err;
	__le64 rdma_resp_rx_local_len_err;
	__le64 rdma_resp_rx_inv_request_err;
	__le64 rdma_resp_rx_local_qp_oper_err;

	__le64 rdma_resp_rx_out_of_atomic_resource;
	__le64 rdma_resp_tx_pkt_seq_err;
	__le64 rdma_resp_tx_remote_inv_req_err;
	__le64 rdma_resp_tx_remote_access_err;
	__le64 rdma_resp_tx_remote_oper_err;
	__le64 rdma_resp_tx_rnr_retry_err;
	__le64 rsvd57;
	__le64 rsvd58;
};

#endif
