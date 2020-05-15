/******************************************************************************/
/* Rx pipeline                                                                */
/******************************************************************************/
control mac_flow_lookup(inout cap_phv_intr_global_h intr_global,
             inout cap_phv_intr_p4_h intr_p4,
             inout headers hdr,
             inout metadata_t metadata) {

    @name(".nop") action nop() {
    }
    
    @appdatafields ("idx", "idx_type")
    @hwfields_access_api  
    @name(".l2_flow_hash")
    action l2_flow_hash(	           
		            bit<12>    hash1,
		            bit<18>    hint1,
		            bit<12>    hash2,
		            bit<18>    hint2,
		            bit<12>    hash3,
		            bit<18>    hint3,
		            bit<12>    hash4,
		            bit<18>    hint4,
		            bit<12>    hash5,
		            bit<18>    hint5,
                            bit<1>      more_hashes,
			   bit<18>     more_hints,
			    bit<22>     idx,
			   bit<1>      entry_valid ) {
       bit<32>  hardware_hash = __hash_value();

        if (entry_valid == FALSE) {
	    hdr.ingress_recirc_header.l2_flow_done = TRUE;
	    hdr.p4i_to_p4e_header.l2_index = 0;
	    hdr.p4i_to_p4e_header.flow_miss = TRUE;
          return;
        }

        if (__table_hit()) {
	    hdr.ingress_recirc_header.l2_flow_done = TRUE;
	    hdr.p4i_to_p4e_header.l2_index = (bit<24>)idx;
	    metadata.cntrl.l2_index = idx;
	    hdr.ingress_recirc_header.l2_flow_ohash_lkp = metadata.cntrl.l2_flow_ohash_lkp;


	} else {
	  bit<1> hint_valid = FALSE;
	  if ((hint1 != 0 )  && (hash1 == hardware_hash[31:20])) {
	    hint_valid = TRUE;
	    metadata.scratch.l2_flow_hint = hint1;
	  } else if ((hint2 != 0 )  && (hash2 == hardware_hash[31:20])) {
	    hint_valid = TRUE;
	    metadata.scratch.l2_flow_hint = hint2;
	  } else if ((hint3 != 0 )  && (hash3 == hardware_hash[31:20])) {
	    hint_valid = TRUE;
	    metadata.scratch.l2_flow_hint = hint3;
	  } else if ((hint4 != 0 ) &&  (hash4 == hardware_hash[31:20])) {
	    hint_valid = TRUE;
	    metadata.scratch.l2_flow_hint = hint4;
	  } else if ((hint5 != 0 ) && (hash5 == hardware_hash[31:20])) {
	    hint_valid = TRUE;
	    metadata.scratch.l2_flow_hint = hint5;
	  } else if ((more_hashes == 1)) {
	    hint_valid = TRUE;
	    metadata.scratch.l2_flow_hint = more_hints;
	  }
	  
	  if( hint_valid == TRUE) {
	    metadata.cntrl.flow_ohash_lkp = TRUE;
	    hdr.ingress_recirc_header.l2_flow_ohash_lkp = TRUE;
	    hdr.ingress_recirc_header.l2_flow_ohash = 
	      POS_OVERFLOW_HASH_BIT | (bit<32>)metadata.scratch.l2_flow_hint;
	  } else {
	    hdr.ingress_recirc_header.l2_flow_done = TRUE;
	    hdr.p4i_to_p4e_header.l2_index = 0;
	    hdr.p4i_to_p4e_header.flow_miss = TRUE;
	  }
	}
    }
    
    @hbm_table    
    @name(".l2_flow_ohash") 
    table l2_flow_ohash {
        key = {
            hdr.ingress_recirc_header.l2_flow_ohash : exact;
        }
        actions = {
            l2_flow_hash;
        }
	  size = L2_FLOW_OHASH_TABLE_SIZE;
        default_action = l2_flow_hash;
        stage = 4;
        placement = HBM;
    }

    @capi_bitfields_struct
    @name(".l2_flow") table l2_flow {
        key = {
	    metadata.key.vnic_id       : exact;
            metadata.l2_key.dmac       : exact;
        }
        actions = {
            l2_flow_hash;
        }
        size = L2_FLOW_TABLE_SIZE;
        placement = HBM;
        default_action = l2_flow_hash;
        stage = 3;
        hash_collision_table = l2_flow_ohash;
    }



    apply {
      //   if (!hdr.ingress_recirc_header.isValid()) {
	if(metadata.cntrl.skip_l2_flow_lkp == FALSE) {
	  l2_flow.apply();	  
	}
	//  }
      
      if (metadata.cntrl.l2_flow_ohash_lkp == TRUE) {
	 l2_flow_ohash.apply();
       } 
    }

}

