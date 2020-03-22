/******************************************************************************/
/* TX pipeline                                                                */
/******************************************************************************/
control conntrack_state_update(inout cap_phv_intr_global_h intr_global,
             inout cap_phv_intr_p4_h intr_p4,
             inout headers hdr,
             inout metadata_t metadata) {

    @name(".nop") action nop() {
    }

          
   
   @name(".conntrack")
     action conntrack_a (@__ref bit<1> valid_flag,
			   @__ref bit<2> flow_type,
			   @__ref bit<8> flow_state,
			   @__ref bit<18> timestamp) {
     
	 metadata.scratch.flag =  valid_flag;
	 metadata.scratch.flow_type =  flow_type;
	 metadata.cntrl.conn_track_prev_state =  flow_state;
	 timestamp = intr_global.timestamp[47:30];
	 if(metadata.cntrl.conn_track_tcp == TRUE) {
	 }
	 if(metadata.cntrl.conn_track_tcp == FALSE) {
	   if(flow_state == CONNTRACK_FLOW_STATE_UNESTABLISHED)
	     flow_state = CONNTRACK_FLOW_STATE_ESTABLISHED;
	 }
	 metadata.cntrl.conn_track_curr_state = flow_state;
	 
    }
   

    

    @name(".conntrack") table conntrack {
        key = {
            metadata.cntrl.conntrack_index  : exact;

        }
        actions = {
	  conntrack_a;
        }
        size = CONNTRACK_TABLE_SIZE;
        placement = HBM;
	default_action = conntrack_a;
        stage = 2;
    }


    apply {
      if(metadata.cntrl.conntrack_index_valid == TRUE) {
	conntrack.apply();
      }
    }

}

