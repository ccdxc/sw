/******************************************************************************/
/* Rx pipeline                                                                */
/******************************************************************************/

enum bit<8> conn_track_state_e {
 CONN_TRACK_STATE_UNESTABLISHED = 0,
 CONN_TRACK_STATE_REMOVED = 1,
 CONN_TRACK_STATE_SYN_SENT= 2,
 CONN_TRACK_STATE_SYN_RECV = 3,
 CONN_TRACK_STATE_SYNACK_SENT = 4,
 CONN_TRACK_STATE_SYNACK_RECV = 5,
 CONN_TRACK_STATE_ESTABLISHED = 6,
 CONN_TRACK_STATE_FIN_SENT = 7,
 CONN_TRACK_STATE_FIN_RECV = 8,
 CONN_TRACK_STATE_TIME_WAIT = 9,
 CONN_TRACK_STATE_RST_CLOSE = 10   }
  


control conn_track_lookup(inout cap_phv_intr_global_h intr_global,
             inout cap_phv_intr_p4_h intr_p4,
             inout headers hdr,
             inout metadata_t metadata) {

    @name(".nop") action nop() {
    }

          
   
   @name("conn_track_a")
     action conn_track_a ( @__ref bit<1> vld,
			   @__ref conn_track_state_e state,
			   @__ref bit<48> timestamp) {
     
       if(vld == TRUE) {
	 metadata.cntrl.conn_track_prev_state =  (bit<8>) state;
	 timestamp = intr_global.timestamp;
	 if(metadata.cntrl.conn_track_tcp == TRUE) {
	 }
	 if(metadata.cntrl.conn_track_tcp == FALSE) {
	   if(state == conn_track_state_e.CONN_TRACK_STATE_UNESTABLISHED)
	     state = conn_track_state_e.CONN_TRACK_STATE_ESTABLISHED;
	 }
	 metadata.cntrl.conn_track_curr_state = (bit<8>)state;
	 
       } else {
	 //TBD
       }
    }
   

    

    @name(".conn_track") table conn_track {
        key = {
            metadata.cntrl.conn_track_id  : exact;

        }
        actions = {
	  conn_track_a;
        }
        size = CONN_TRACK_TABLE_SIZE;
        placement = HBM;
	default_action = conn_track_a;
        stage = 5;
    }


    apply {
      if(metadata.cntrl.conn_track_vld == TRUE) {
	conn_track.apply();
      }
    }

}

