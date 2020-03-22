control Stats(inout cap_phv_intr_global_h capri_intrinsic,
	      inout cap_phv_intr_p4_h intr_p4,
	      inout headers hdr,
	      inout metadata_t metadata) {


    @name(".nop") action nop() {
    }

    @name(".update_rx_stats_0") action update_rx_stats_0(@__ref bit<64>  pkt_cnt,
							@__ref bit<64>  byte_cnt) {

        pkt_cnt = pkt_cnt + 1;
	byte_cnt = byte_cnt + (bit<64>)metadata.cntrl.rx_pkt_len;

    }

    @name(".update_rx_stats_1") action update_rx_stats_1(@__ref bit<64>  pkt_cnt,
							@__ref bit<64>  byte_cnt) {

        pkt_cnt = pkt_cnt + 1;
	byte_cnt = byte_cnt + (bit<64>)metadata.cntrl.rx_pkt_len;

    }

    @name(".update_tx_stats_0") action update_tx_stats_0(@__ref bit<64>  pkt_cnt,
							@__ref bit<64>  byte_cnt) {

        pkt_cnt = pkt_cnt + 1;
	byte_cnt = byte_cnt + (bit<64>)intr_p4.packet_len;

    }

    @name(".update_tx_stats_1") action update_tx_stats_1(@__ref bit<64>  pkt_cnt,
							@__ref bit<64>  byte_cnt) {

        pkt_cnt = pkt_cnt + 1;
	byte_cnt = byte_cnt + (bit<64>)intr_p4.packet_len;

    }

    @name(".rx_stats_0") table rx_stats_0 {
        key = {
	  metadata.cntrl.counterset1 : exact;
        }
        actions = {
            update_rx_stats_0;
            nop;
        }
        size = STATS_TABLE_SIZE;
        placement = HBM;
        default_action = update_rx_stats_0;
        stage = 3;
    }

    @name(".rx_stats_1") table rx_stats_1 {
        key = {
            metadata.cntrl.counterset2 : exact;
        }
        actions = {
            update_rx_stats_1;
            nop;
        }
        size = STATS_TABLE_SIZE;
        placement = HBM;
        default_action = update_rx_stats_1;
        stage = 3;
    }

    @name(".tx_stats_0") table tx_stats_0 {
        key = {
            metadata.cntrl.counterset1 : exact;
        }
        actions = {
            update_tx_stats_0;
            nop;
        }
        size = STATS_TABLE_SIZE;
        placement = HBM;
        default_action = update_tx_stats_0;
        stage = 2;
    }

    @name(".tx_stats_1") table tx_stats_1 {
        key = {
            metadata.cntrl.counterset2 : exact;
        }
        actions = {
            update_tx_stats_1;
            nop;
        }
        size = STATS_TABLE_SIZE;
        placement = HBM;
        default_action = update_tx_stats_1;
        stage = 2;
    }
    
    apply {
        rx_stats_0.apply();
        rx_stats_1.apply();
        tx_stats_0.apply();
        tx_stats_1.apply();
    }
}

