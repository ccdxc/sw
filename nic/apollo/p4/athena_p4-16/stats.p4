/*
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

    @hbm_table
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

    @hbm_table
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

    @hbm_table
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

    @hbm_table
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

*/

control p4i_statistics(inout cap_phv_intr_global_h capri_intrinsic,
	      inout cap_phv_intr_p4_h intr_p4,
	      inout headers hdr,
	      inout metadata_t metadata) {


    @name(".p4i_stats") action p4i_stats_a(@__ref bit<64>  rx_from_host,
					 @__ref bit<64>  rx_from_switch,
					 @__ref bit<64>  rx_from_arm
					 ) {
      if(metadata.cntrl.from_arm == TRUE) {
	rx_from_arm = rx_from_arm + 1;
      } else {
	if(metadata.cntrl.direction == TX_FROM_HOST) {
	  rx_from_host = rx_from_host + 1;	  
	} else {
	  rx_from_switch = rx_from_switch + 1;

	}

      }
    }


    @name(".p4i_stats") table p4i_stats {
        key = {
	  metadata.cntrl.stats_id : exact;
        }
        actions = {
            p4i_stats_a;
        }
        size = 2;
        placement = HBM;
        default_action = p4i_stats_a;
        stage = 5;
    }

    apply {
      p4i_stats.apply();
    }
}

control p4e_statistics(inout cap_phv_intr_global_h capri_intrinsic,
	      inout cap_phv_intr_p4_h intr_p4,
	      inout headers hdr,
	      inout metadata_t metadata) {


    @name(".p4e_stats") action p4e_stats_a(@__ref bit<64>  tx_to_host,
					 @__ref bit<64>  tx_to_switch,
					 @__ref bit<64>  tx_to_arm
					 ) {
      if((metadata.cntrl.p4e_stats_flag & P4E_STATS_FLAG_TX_TO_HOST) == P4E_STATS_FLAG_TX_TO_HOST) {
	tx_to_host = tx_to_host + 1;
      } 
      if((metadata.cntrl.p4e_stats_flag & P4E_STATS_FLAG_TX_TO_SWITCH) == P4E_STATS_FLAG_TX_TO_SWITCH) {
	tx_to_switch = tx_to_switch + 1;
      } 
      if((metadata.cntrl.p4e_stats_flag & P4E_STATS_FLAG_TX_TO_ARM) == P4E_STATS_FLAG_TX_TO_ARM) {
	tx_to_arm = tx_to_arm + 1;
      } 
    }


    @name(".p4e_stats") table p4e_stats {
        key = {
	  metadata.cntrl.stats_id : exact;
        }
        actions = {
            p4e_stats_a;
        }
        size = 2;
        placement = HBM;
        default_action = p4e_stats_a;
        stage = 5;
    }

    apply {
      p4e_stats.apply();
    }
}
