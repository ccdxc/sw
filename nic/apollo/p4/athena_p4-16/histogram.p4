control Histogram(inout cap_phv_intr_global_h capri_intrinsic,
	      inout cap_phv_intr_p4_h intr_p4,
	      inout headers hdr,
	      inout metadata_t metadata) {


    @name(".nop") action nop() {
    }
    
    @name(".update_rx_histogram_0") action update_rx_histogram_0(@__ref bit<64>  pkt_cnt) {
						
        pkt_cnt = pkt_cnt + 1;

    }

    @name(".update_rx_histogram_1") action update_rx_histogram_1(@__ref bit<64>  pkt_cnt){
							

        pkt_cnt = pkt_cnt + 1;

    }

    @name(".update_tx_histogram_0") action update_tx_histogram_0(@__ref bit<64>  pkt_cnt) {
						
        pkt_cnt = pkt_cnt + 1;

    }

    @name(".update_tx_histogram_1") action update_tx_histogram_1(@__ref bit<64>  pkt_cnt) {

        pkt_cnt = pkt_cnt + 1;

    }

    @name(".rx_histogram_0") table rx_histogram_0 {
        key = {
            metadata.cntrl.rx_histogram_0_index : exact;
            metadata.cntrl.rx_pkt_hist_bucket : exact;
	    
        }
        actions = {
            update_rx_histogram_0;
            nop;
        }
        size = HISTOGRAM_TABLE_SIZE;
        placement = HBM;
        default_action = update_rx_histogram_0;
        stage = 5;
    }

    @name(".rx_histogram_1") table rx_histogram_1 {
        key = {
            metadata.cntrl.rx_histogram_1_index : exact;
            metadata.cntrl.rx_pkt_hist_bucket : exact;
        }
        actions = {
            update_rx_histogram_1;
            nop;
        }
        size = HISTOGRAM_TABLE_SIZE;
        placement = HBM;
        default_action = update_rx_histogram_1;
        stage = 5;
    }

    @name(".tx_histogram_0") table tx_histogram_0 {
        key = {
            metadata.cntrl.tx_histogram_0_index : exact;
            metadata.cntrl.tx_pkt_hist_bucket : exact;
        }
        actions = {
            update_tx_histogram_0;
            nop;
        }
        size = HISTOGRAM_TABLE_SIZE;
        placement = HBM;
        default_action = update_tx_histogram_0;
        stage = 4;
    }

    @name(".tx_histogram_1") table tx_histogram_1 {
        key = {
            metadata.cntrl.tx_histogram_1_index : exact;
            metadata.cntrl.tx_pkt_hist_bucket : exact;
        }
        actions = {
            update_tx_histogram_1;
            nop;
        }
        size = HISTOGRAM_TABLE_SIZE;
        placement = HBM;
        default_action = update_tx_histogram_1;
        stage = 4;
    }
    
    apply {
        rx_histogram_0.apply();
        rx_histogram_1.apply();
        tx_histogram_0.apply();
        tx_histogram_1.apply();
    }
}

