control EgressStatsFixup(inout cap_phv_intr_global_h intr_global,
             inout cap_phv_intr_p4_h intr_p4,
             inout headers hdr,
             inout metadata_t metadata) {

    @name(".nop") action nop() {
    }

    @name(".stats_ptr_fixup_action") action stats_ptr_fixup_action() {
      metadata.cntrl.counterset1 = hdr.p4i_to_p4e_header.counterset1[9:0];
      metadata.cntrl.counterset2 = hdr.p4i_to_p4e_header.counterset2[9:0];
      metadata.cntrl.rx_histogram_0_index = hdr.p4i_to_p4e_header.histogram[12:0];
      metadata.cntrl.rx_histogram_1_index = hdr.p4i_to_p4e_header.histogram[12:0];
      metadata.cntrl.tx_histogram_0_index = hdr.p4i_to_p4e_header.histogram[12:0];
      metadata.cntrl.tx_histogram_1_index = hdr.p4i_to_p4e_header.histogram[12:0];
    }
    
    @name(".histogram_bkt_a") action histogram_bkt_a() {

      //DO I NEED TO ADJUST 4 BYTES FOR NO CRC?
      //	  metadata.cntrl.rx_pkt_len = hdr.i2e.rx_pkt_len;
	  if(metadata.cntrl.rx_pkt_len >= 256) {
	    if(metadata.cntrl.rx_pkt_len >= 1024) {
	      if(metadata.cntrl.rx_pkt_len >= 1518) {
		metadata.cntrl.rx_pkt_hist_bucket = 7;
	      } else {
		metadata.cntrl.rx_pkt_hist_bucket = 6;
	      }
	    } else {
	      if(metadata.cntrl.rx_pkt_len >= 512) {
		metadata.cntrl.rx_pkt_hist_bucket = 5;
	      } else {
		metadata.cntrl.rx_pkt_hist_bucket = 4;
	      }
	    }
	  } else {
	    if(metadata.cntrl.rx_pkt_len >= 128) {
	      if(metadata.cntrl.rx_pkt_len >= 256) {
		metadata.cntrl.rx_pkt_hist_bucket = 3;
	      } else {
		metadata.cntrl.rx_pkt_hist_bucket = 2;
	      }
	    } else {
	      if(metadata.cntrl.rx_pkt_len > 64) {
		metadata.cntrl.rx_pkt_hist_bucket = 1;
	      } else {
		metadata.cntrl.rx_pkt_hist_bucket = 0;
	      }
	    }	      
	  }

	  //TX
	  if(intr_p4.packet_len >= 256) {
	    if(intr_p4.packet_len >= 1024) {
	      if(intr_p4.packet_len >= 1518) {
		metadata.cntrl.tx_pkt_hist_bucket = 7;
	      } else {
		metadata.cntrl.tx_pkt_hist_bucket = 6;
	      }
	    } else {
	      if(intr_p4.packet_len >= 512) {
		metadata.cntrl.tx_pkt_hist_bucket = 5;
	      } else {
		metadata.cntrl.tx_pkt_hist_bucket = 4;
	      }
	    }
	  } else {
	    if(intr_p4.packet_len >= 128) {
	      if(intr_p4.packet_len >= 256) {
		metadata.cntrl.tx_pkt_hist_bucket = 3;
	      } else {
		metadata.cntrl.tx_pkt_hist_bucket = 2;
	      }
	    } else {
	      if(intr_p4.packet_len > 64) {
		metadata.cntrl.tx_pkt_hist_bucket = 1;
	      } else {
		metadata.cntrl.tx_pkt_hist_bucket = 0;
	      }
	    }	      
	  }

    }

 
    @name(".histogram_bkt") table histogram_bkt {
        key = {
        }
        actions = {
           histogram_bkt_a;
        }
        stage = 3;
    }

    @name(".stats_ptr_fixup") table stats_ptr_fixup {
        key = {
        }
        actions = {
            stats_ptr_fixup_action;
        }
        stage = 0;
    }



    apply {
      
      histogram_bkt.apply();
      stats_ptr_fixup.apply();
    }

}
