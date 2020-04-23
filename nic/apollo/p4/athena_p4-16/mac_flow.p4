/******************************************************************************/
/* Rx pipeline                                                                */
/******************************************************************************/
control mac_flow_lookup(inout cap_phv_intr_global_h intr_global,
             inout cap_phv_intr_p4_h intr_p4,
             inout headers hdr,
             inout metadata_t metadata) {

    @name(".nop") action nop() {
    }
    
    // @appdatafields ("idx", "idx_type")
    //@hwfields_access_api  
    @name(".mac_lkp_a")
    action mac_lkp_a(
		           bit<22>     idx,
                            bit<1>     idx_type,
			   //	   bit<7>     pad,
		            bit<11>    hash1,
		            bit<17>    hint1,
		            bit<11>    hash2,
		            bit<17>    hint2,
		            bit<11>    hash3,
		            bit<17>    hint3,
		            bit<11>    hash4,
		            bit<17>    hint4,
		            bit<11>    hash5,
		            bit<17>    hint5,
                            bit<1>      more_hashes,
			   bit<17>     more_hints,
			   bit<1>      entry_valid ) {
       bit<32>  hardware_hash = __hash_value();
       hdr.p4i_to_p4e_header.flow_hash = hardware_hash;

        if (entry_valid == FALSE) {
	    hdr.ingress_recirc_header.flow_done = TRUE;
	    hdr.p4i_to_p4e_header.l2_index = 0;
//	    hdr.p4i_to_p4e_header.l2_index_type = 0;
	    hdr.p4i_to_p4e_header.flow_miss = TRUE;
            metadata.cntrl.flow_miss = TRUE;
 	    metadata.cntrl.index = 0;
          return;
        }

        if (__table_hit()) {
	    hdr.ingress_recirc_header.l2_flow_done = TRUE;
	    hdr.p4i_to_p4e_header.l2_index = (bit<24>)idx;
	  //  hdr.p4i_to_p4e_header.l2_index_type = idx_type;
	    hdr.p4i_to_p4e_header.direction = metadata.cntrl.direction;
	    metadata.cntrl.l2_index = idx;
            metadata.scratch.hint_valid = TRUE;

	} else {
	  metadata.scratch.hint_valid = FALSE;
            if ((hint1 != 0 ) && (metadata.scratch.hint_valid == FALSE) && (hash1 == hardware_hash[31:21])) {
	      metadata.scratch.hint_valid = TRUE;
	      metadata.scratch.l2_flow_hint = hint1;
	    }
            if ((hint2 != 0 ) && (metadata.scratch.hint_valid == FALSE) && (hash2 == hardware_hash[31:21])) {
	      metadata.scratch.hint_valid = TRUE;
	      metadata.scratch.l2_flow_hint = hint2;
	    }
            if ((hint3 != 0 ) && (metadata.scratch.hint_valid == FALSE) && (hash3 == hardware_hash[31:21])) {
	      metadata.scratch.hint_valid = TRUE;
	      metadata.scratch.l2_flow_hint = hint3;
	    }
            if ((hint4 != 0 ) && (metadata.scratch.hint_valid == FALSE) && (hash4 == hardware_hash[31:21])) {
	      metadata.scratch.hint_valid = TRUE;
	      metadata.scratch.l2_flow_hint = hint4;
	    }
            if ((hint5 != 0 ) && (metadata.scratch.hint_valid == FALSE) && (hash5 == hardware_hash[31:21])) {
	      metadata.scratch.hint_valid = TRUE;
	      metadata.scratch.l2_flow_hint = hint5;
	    }
	    metadata.scratch.flag = more_hashes;
            if ((more_hashes == 1) && (metadata.scratch.hint_valid == FALSE)) {
	      metadata.scratch.hint_valid = TRUE;
	      metadata.scratch.l2_flow_hint = more_hints;
	    }
	    
	    if( metadata.scratch.hint_valid == TRUE) {
	      metadata.cntrl.flow_ohash_lkp = TRUE;
	      hdr.ingress_recirc_header.l2_flow_ohash = 
		POS_OVERFLOW_HASH_BIT | (bit<32>)metadata.scratch.l2_flow_hint;
	    } else {
	      hdr.ingress_recirc_header.l2_flow_done = TRUE;
	    }

	}
        
    }

    //    @hbm_table    
    @name(".mac_lkp_ohash") 
    table mac_lkp_ohash {
        key = {
            hdr.ingress_recirc_header.l2_flow_ohash : exact;
        }
        actions = {
            mac_lkp_a;
        }
	  size = MAC_OHASH_TABLE_SIZE;
        default_action = mac_lkp_a;
        stage = 4;
        placement = HBM;
    }

    //    @capi_bitfields_struct
    @name(".mac_lkp_tbl") table mac_lkp_tbl {
        key = {
	  //            metadata.l2_key.vnic_id16       : exact;
	    metadata.key.vnic_id       : exact;
            metadata.l2_key.dmac       : exact;
        }
        actions = {
            mac_lkp_a;
        }
        size = MAC_TABLE_SIZE;
        placement = HBM;
        default_action = mac_lkp_a;
        stage = 3;
        hash_collision_table = mac_lkp_ohash;
    }



    apply {
      if (!hdr.ingress_recirc_header.isValid()) {
	if(metadata.cntrl.skip_l2_flow_lkp == FALSE) {
	  mac_lkp_tbl.apply();	  
	}
      }
      
      if (metadata.cntrl.l2_flow_ohash_lkp == TRUE) {
	 mac_lkp_ohash.apply();
       } 
    }

}

