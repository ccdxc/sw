/******************************************************************************/
/* TX pipeline                                                                */
/******************************************************************************/
control flow_log_lookup(inout cap_phv_intr_global_h intr_global,
             inout cap_phv_intr_p4_h intr_p4,
             inout headers hdr,
             inout metadata_t metadata) {

    @name(".nop") action nop() {
    }

       
    @name(".flow_log_hash")
      action flow_log_hash_a() {
      metadata.cntrl.flow_log_hash =  __hash_value()[19:0];
    }
   
   
   @name(".flow_log_key")
     action flow_log_key_a ( @__ref bit<1> vld,
			   @__ref bit<2> ktype,
			   @__ref bit<10> vnic_id,
			   @__ref bit<128> src_ip,
			   @__ref bit<128> dst_ip,
			   @__ref bit<8> proto,
			   @__ref bit<16> sport,
			   @__ref bit<16> dport) {
     
       if(vld == FALSE) {
	 metadata.cntrl.flow_log_hash_insert = TRUE;
	 vld = 1;
	 ktype = metadata.key.ktype;
	 vnic_id = metadata.key.vnic_id;
	 src_ip = metadata.key.src;
	 dst_ip = metadata.key.dst;
	 proto= metadata.key.proto;
	 sport = metadata.key.sport;
	 dport = metadata.key.dport;
       } else {
	 if((ktype == metadata.key.ktype) &&
	    (vnic_id == metadata.key.vnic_id) &&
	    (src_ip == metadata.key.src) &&
	    (dst_ip == metadata.key.dst) &&
	    (proto == metadata.key.proto) &&
	    (sport == metadata.key.sport) &&
	    (dport == metadata.key.dport)) {

	   metadata.cntrl.flow_log_hash_hit = TRUE;
	   
	 } else {
	   metadata.cntrl.flow_log_hash_hit = FALSE;
	 }
	    
	 vld = 1;
	 ktype = ktype;
	 vnic_id = vnic_id;
	 src_ip = src_ip;
	 dst_ip = dst_ip;
	 proto= proto;
	 sport = sport;
	 dport = dport;
       }
    }
   
   @name(".flow_log_data")
     action flow_log_data_a ( @__ref bit<1> vld,
			   @__ref bit<2> security_state,
			   @__ref bit<40> pkt_from_host,
			   @__ref bit<40> pkt_from_switch,
			   @__ref bit<64> bytes_from_host,
			   @__ref bit<64> bytes_from_switch,
			   @__ref bit<48> start_timestamp,			    
			   @__ref bit<48> last_timestamp) {
     
     if(metadata.cntrl.flow_log_hash_insert == TRUE) {
	 vld = 1;
	 security_state = 0;//TO BE DONE
	 pkt_from_host = (metadata.cntrl.direction == TX_FROM_HOST) ? (bit<40>)1 : 0;
	 pkt_from_switch = (metadata.cntrl.direction == RX_FROM_SWITCH) ? (bit<40>)1 : 0;
	 bytes_from_host = (metadata.cntrl.direction == TX_FROM_HOST) ? (bit<64>)intr_p4.packet_len : 0;
	 bytes_from_switch = (metadata.cntrl.direction == RX_FROM_SWITCH) ? (bit<64>)intr_p4.packet_len : 0;
	 start_timestamp = intr_global.timestamp;	 
	 last_timestamp = intr_global.timestamp;	 
     } else if(metadata.cntrl.flow_log_hash_hit == TRUE) {
	 vld = 1;
	 security_state = 0;//TO BE DONE
	 pkt_from_host = pkt_from_host + ((metadata.cntrl.direction == TX_FROM_HOST) ? (bit<40>)1 : 0);
	 pkt_from_switch = pkt_from_switch + ((metadata.cntrl.direction == RX_FROM_SWITCH) ? (bit<40>)1 : 0);
	 bytes_from_host = bytes_from_host + ((metadata.cntrl.direction == TX_FROM_HOST) ? (bit<64>)intr_p4.packet_len : 0);
	 bytes_from_switch = bytes_from_switch + ((metadata.cntrl.direction == RX_FROM_SWITCH) ? (bit<64>)intr_p4.packet_len : 0);
	 start_timestamp = start_timestamp;	 
	 last_timestamp = intr_global.timestamp;	        
     } else {
	 vld = vld;
	 security_state = security_state;
	 pkt_from_host = pkt_from_host;
	 pkt_from_switch = pkt_from_switch;
	 bytes_from_host = bytes_from_host;
	 bytes_from_switch = bytes_from_switch;
	 start_timestamp = start_timestamp;	 
	 last_timestamp = intr_global.timestamp;	        
     }
    }

    

    @name(".flow_log") table flow_log {
        key = {
	  //            metadata.key.src_mac       : exact;
          //  metadata.key.dst_mac       : exact;
            metadata.key.src           : exact;
            metadata.key.dst           : exact;
            metadata.key.proto         : exact;
            metadata.key.sport         : exact;
            metadata.key.dport         : exact;
            metadata.key.vnic_id       : exact;
            metadata.key.ktype         : exact;

        }
        actions = {
	  flow_log_hash_a;
        }
        size = 0;
        placement = HBM;
	default_action = flow_log_hash_a;
        stage = 0;
    }

    @name(".flow_log_key") table flow_log_key {
        key = {
           metadata.cntrl.flow_log_hash : exact;
        }
        actions = {
	  //nop;
	  flow_log_key_a;
        }
	  
	size = FLOW_LOG_TABLE_SIZE;
	default_action = flow_log_key_a;
        stage = 3;
        placement = HBM;
    }

    @name(".flow_log_data") table flow_log_data {
        key = {
           metadata.cntrl.flow_log_hash : exact;
        }
        actions = {
	  //nop();
	  flow_log_data_a;
        }
	  
	size = FLOW_LOG_TABLE_SIZE;
	default_action = flow_log_data_a;
        stage = 4;
        placement = HBM;
    }
 
    apply {
      flow_log.apply();
      flow_log_key.apply();
      flow_log_data.apply();
    }

}

