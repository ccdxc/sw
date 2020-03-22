/******************************************************************************/
/* Rx pipeline                                                                */
/******************************************************************************/
control session_info_lookup(inout cap_phv_intr_global_h intr_global,
             inout cap_phv_intr_p4_h capri_p4_intrinsic,
             inout headers hdr,
             inout metadata_t metadata) {

    @name(".session_info")
    action session_info_a(@__ref bit<1>  valid_flag,
				        bit<1>  skip_flow_log,
				        bit<22> conntrack_id,
				 @__ref bit<18> timestamp,
				        bit<48> smac,
				 bit<16> h2s_epoch_vnic_value,
				 bit<20> h2s_epoch_vnic_id,
				 bit<16> h2s_epoch_mapping_value,
				 bit<20> h2s_epoch_mapping_id,
				 bit<13>     h2s_throttle_bw1_id,
				 bit<13>     h2s_throttle_bw2_id,
				        bit<9>     h2s_vnic_statistics_id,
				        bit<32>     h2s_vnic_statistics_mask,
				        bit<9>     h2s_vnic_histogram_packet_len_id,
				        bit<9>     h2s_vnic_histogram_latency_id,
				        bit<8>     h2s_slow_path_tcp_flags_match,
				 bit<22> h2s_session_rewrite_id,
				 bit<3> h2s_egress_action,
				 bit<10> h2s_allowed_flow_state_bitmap,


				 bit<16> s2h_epoch_vnic_value,
				 bit<20> s2h_epoch_vnic_id,
				 bit<16> s2h_epoch_mapping_value,
				 bit<20> s2h_epoch_mapping_id,
				 bit<13>     s2h_throttle_bw1_id,
				 bit<13>     s2h_throttle_bw2_id,
				        bit<9>     s2h_vnic_statistics_id,
				        bit<32>     s2h_vnic_statistics_mask,
				        bit<9>     s2h_vnic_histogram_packet_len_id,
				        bit<9>     s2h_vnic_histogram_latency_id,
				        bit<8>     s2h_slow_path_tcp_flags_match,
				 bit<22> s2h_session_rewrite_id,
				 bit<3> s2h_egress_action,
				 bit<10> s2h_allowed_flow_state_bitmap

						       ) {
      if(valid_flag == TRUE) {
	metadata.cntrl.skip_flow_log = skip_flow_log;
	if(conntrack_id != 0) {
	    metadata.cntrl.conntrack_index = conntrack_id;
	    metadata.cntrl.conntrack_index_valid = TRUE;
	}
	
	if(metadata.cntrl.direction == TX_FROM_HOST) {
	  if(metadata.cntrl.l2_vnic == FALSE) {
	    if(h2s_session_rewrite_id != 0) {
	      metadata.cntrl.session_rewrite_id = h2s_session_rewrite_id;
	    } else {
	      metadata.cntrl.flow_miss = TRUE;
	    }
	  }
	  
	  if((hdr.l4_u.tcp.flags & h2s_slow_path_tcp_flags_match) != 0) {
	    metadata.cntrl.flow_miss = TRUE;
	  }
	  metadata.cntrl.smac = smac;
	    
	  if(h2s_epoch_vnic_id != 0) {
	    metadata.cntrl.epoch1_id = h2s_epoch_vnic_id;
	    metadata.cntrl.epoch1_value = h2s_epoch_vnic_value;
	    metadata.cntrl.epoch1_id_valid = TRUE;
	  }

	  if(h2s_epoch_mapping_id != 0) {
	    metadata.cntrl.epoch2_id = h2s_epoch_mapping_id;
	    metadata.cntrl.epoch2_value = h2s_epoch_mapping_value;
	    metadata.cntrl.epoch2_id_valid = TRUE;
	  }

	  if (h2s_throttle_bw1_id != 0) {
	    metadata.cntrl.throttle_bw1_id = h2s_throttle_bw1_id;
	    metadata.cntrl.throttle_bw1_id_valid = TRUE;
	  }
	  if (h2s_throttle_bw2_id != 0) {
	    metadata.cntrl.throttle_bw2_id = h2s_throttle_bw2_id;
	    metadata.cntrl.throttle_bw2_id_valid = TRUE;
	  }

            /* TODO: Stats ID and histogram IDs are likely shared across both directions, 
                Confirm if this is true and if stats mask needs to be per-direction
            */
	  
	  if (h2s_vnic_statistics_id != 0) {
	    metadata.cntrl.statistics_id_valid = TRUE;
	    metadata.cntrl.vnic_statistics_id = h2s_vnic_statistics_id;
	    metadata.cntrl.vnic_statistics_mask = h2s_vnic_statistics_mask;
	  }

	  if (h2s_vnic_histogram_packet_len_id != 0) {
	    metadata.cntrl.histogram_packet_len_id_valid = TRUE;
	    metadata.cntrl.histogram_packet_len_id = h2s_vnic_histogram_packet_len_id;
	  }
	  if (h2s_vnic_histogram_latency_id != 0) {
	    metadata.cntrl.histogram_latency_id_valid = TRUE;
	    metadata.cntrl.histogram_latency_id = h2s_vnic_histogram_latency_id;
	  }
	  metadata.cntrl.egress_action = h2s_egress_action;
	  metadata.cntrl.allowed_flow_state_bitmap = h2s_allowed_flow_state_bitmap;

	}

	if(metadata.cntrl.direction == RX_FROM_SWITCH) {
	  if(metadata.cntrl.l2_vnic == FALSE) {
	    if(s2h_session_rewrite_id != 0) {
	      metadata.cntrl.session_rewrite_id = s2h_session_rewrite_id;
	    } else {
	      metadata.cntrl.flow_miss = TRUE;
	    }
	  }
	  
	  if((hdr.l4_u.tcp.flags & s2h_slow_path_tcp_flags_match) != 0) {
	    metadata.cntrl.flow_miss = TRUE;
	  }
	  metadata.cntrl.smac = smac;
	    
	  if(s2h_epoch_vnic_id != 0) {
	    metadata.cntrl.epoch1_id = s2h_epoch_vnic_id;
	    metadata.cntrl.epoch1_value = s2h_epoch_vnic_value;
	    metadata.cntrl.epoch1_id_valid = TRUE;
	  }

	  if(s2h_epoch_mapping_id != 0) {
	    metadata.cntrl.epoch2_id = s2h_epoch_mapping_id;
	    metadata.cntrl.epoch2_value = s2h_epoch_mapping_value;
	    metadata.cntrl.epoch2_id_valid = TRUE;
	  }

	  if (s2h_throttle_bw1_id != 0) {
	    metadata.cntrl.throttle_bw1_id = s2h_throttle_bw1_id;
	    metadata.cntrl.throttle_bw1_id_valid = TRUE;
	  }
	  if (s2h_throttle_bw2_id != 0) {
	    metadata.cntrl.throttle_bw2_id = s2h_throttle_bw2_id;
	    metadata.cntrl.throttle_bw2_id_valid = TRUE;
	  }

            /* TODO: Stats ID and histogram IDs are likely shared across both directions, 
                Confirm if this is true and if stats mask needs to be per-direction
            */
	  
	  if (s2h_vnic_statistics_id != 0) {
	    metadata.cntrl.statistics_id_valid = TRUE;
	    metadata.cntrl.vnic_statistics_id = s2h_vnic_statistics_id;
	    metadata.cntrl.vnic_statistics_mask = s2h_vnic_statistics_mask;
	  }

	  if (s2h_vnic_histogram_packet_len_id != 0) {
	    metadata.cntrl.histogram_packet_len_id_valid = TRUE;
	    metadata.cntrl.histogram_packet_len_id = s2h_vnic_histogram_packet_len_id;
	  }
	  if (s2h_vnic_histogram_latency_id != 0) {
	    metadata.cntrl.histogram_latency_id_valid = TRUE;
	    metadata.cntrl.histogram_latency_id = s2h_vnic_histogram_latency_id;
	  }
	  metadata.cntrl.egress_action = s2h_egress_action;
	  metadata.cntrl.allowed_flow_state_bitmap = s2h_allowed_flow_state_bitmap;


	}

	timestamp = timestamp;
	valid_flag = valid_flag;
      } else {
	metadata.cntrl.flow_miss = TRUE;
      }	
    }

      @name(".session_info")
	table session_info {
        key = {
	metadata.cntrl.session_index  : exact;
        } 
        actions  = {
	  session_info_a;
        }
        size  = SESSION_TABLE_SIZE;
	default_action = session_info_a; 
        stage = 1;
        placement = HBM;
    }


#define SESSION_REWRITE_COMMON_FIELDS       bit<1> valid_flag,		\
                                            bit<1> strip_outer_encap_flag, \
                                            bit<1> strip_l2_header_flag, \
                                            bit<1> strip_vlan_tag_flag

#define SESSION_REWRITE_COMMON_FIELDS_ARGS   valid_flag,		\
                                              strip_outer_encap_flag, \
                                              strip_l2_header_flag, \
                                              strip_vlan_tag_flag



   @name(".session_rewrite_common") 
     action session_rewrite_common(SESSION_REWRITE_COMMON_FIELDS) {
     if (valid_flag == TRUE) {
         /* TODO: Confirm the required semantics for these flags */
       
       if (strip_l2_header_flag == TRUE) {
	 hdr.ethernet_1.setInvalid();
       }
       if (strip_vlan_tag_flag == TRUE) {
	 hdr.ctag_1.setInvalid();
       }
       
       if (metadata.cntrl.direction == RX_FROM_SWITCH) {
	 if (strip_outer_encap_flag == TRUE) {
	   hdr.ip_1.ipv4.setInvalid();
	   hdr.udp.setInvalid();
	   hdr.mpls_src.setInvalid();
	   hdr.mpls_dst.setInvalid();
	      hdr.mpls_label3_1.setInvalid();
	      
	 }
       }
       
       metadata.scratch.packet_len = hdr.p4i_to_p4e_header.packet_len;
#if 0
       hdr.ip_2.ipv4.srcAddr = metadata.scratch.addr;
       hdr.ip_2.ipv4.dstAddr = metadata.scratch.addr;
       hdr.ipv6_2.srcAddr = nat_address;
       hdr.ipv6_2.dstAddr = nat_addresss;
       metadata.cntrl.egress_action = egress_action;
#endif
     } else {
       metadata.cntrl.flow_miss = TRUE;
     }
   }


   @name(".session_rewrite") 
   action session_rewrite(SESSION_REWRITE_COMMON_FIELDS) {
     session_rewrite_common(SESSION_REWRITE_COMMON_FIELDS_ARGS);
   }

   @name(".session_rewrite_ipv4_snat") 
   action session_rewrite_ipv4_snat(SESSION_REWRITE_COMMON_FIELDS,
				    bit<32> ipv4_addr_snat) {
     session_rewrite_common(SESSION_REWRITE_COMMON_FIELDS_ARGS);

     if(metadata.cntrl.direction == TX_FROM_HOST) {
       hdr.ip_1.ipv4.srcAddr = ipv4_addr_snat;
     } 

     if(metadata.cntrl.direction == RX_FROM_SWITCH) {
       hdr.ip_2.ipv4.srcAddr = ipv4_addr_snat;
     } 
    
   }

   @name(".session_rewrite_ipv4_dnat") 
   action session_rewrite_ipv4_dnat(SESSION_REWRITE_COMMON_FIELDS,
				    bit<32> ipv4_addr_dnat) {
     session_rewrite_common(SESSION_REWRITE_COMMON_FIELDS_ARGS);

     if(metadata.cntrl.direction == TX_FROM_HOST) {
       hdr.ip_1.ipv4.dstAddr = ipv4_addr_dnat;
     } 

     if(metadata.cntrl.direction == RX_FROM_SWITCH) {
       hdr.ip_2.ipv4.dstAddr = ipv4_addr_dnat;
     } 
    
   }

   @name(".session_rewrite_ipv4_pat") 
   action session_rewrite_ipv4_pat(SESSION_REWRITE_COMMON_FIELDS,
				   bit<32> ipv4_addr_spat,
				   bit<32> ipv4_addr_dpat,
				   bit<16> l4_port_spat,
				   bit<16> l4_port_dpat

) {
     session_rewrite_common(SESSION_REWRITE_COMMON_FIELDS_ARGS);

     if(metadata.cntrl.direction == TX_FROM_HOST) {
       if(ipv4_addr_spat != 0) {
	 hdr.ip_1.ipv4.srcAddr = ipv4_addr_spat;
       }
       if(ipv4_addr_dpat != 0) {
	 hdr.ip_1.ipv4.dstAddr = ipv4_addr_dpat;
       }

     } 
     if(metadata.cntrl.direction == RX_FROM_SWITCH) {
       if(ipv4_addr_spat != 0) {
	 hdr.ip_2.ipv4.srcAddr = ipv4_addr_spat;
       }
       if(ipv4_addr_dpat != 0) {
	 hdr.ip_2.ipv4.dstAddr = ipv4_addr_dpat;
       }

     } 
     
    if (l4_port_spat != 0) {
      hdr.l4_u.tcp.srcPort = l4_port_spat;
    }    
    if (l4_port_dpat != 0) {
      hdr.l4_u.tcp.dstPort = l4_port_dpat;
    }    

   }

   @name(".session_rewrite_ipv6_snat") 
   action session_rewrite_ipv6_snat(SESSION_REWRITE_COMMON_FIELDS,
				    bit<128> ipv6_addr_snat) {
     session_rewrite_common(SESSION_REWRITE_COMMON_FIELDS_ARGS);

     if(metadata.cntrl.direction == TX_FROM_HOST) {
       hdr.ip_1.ipv6.srcAddr = ipv6_addr_snat;
     } 

     if(metadata.cntrl.direction == RX_FROM_SWITCH) {
       hdr.ip_2.ipv6.srcAddr = ipv6_addr_snat;
     } 
    
   }

   @name(".session_rewrite_ipv6_dnat") 
   action session_rewrite_ipv6_dnat(SESSION_REWRITE_COMMON_FIELDS,
				    bit<128> ipv6_addr_dnat) {
     session_rewrite_common(SESSION_REWRITE_COMMON_FIELDS_ARGS);

     if(metadata.cntrl.direction == TX_FROM_HOST) {
       hdr.ip_1.ipv6.dstAddr = ipv6_addr_dnat;
     } 

     if(metadata.cntrl.direction == RX_FROM_SWITCH) {
       hdr.ip_2.ipv6.dstAddr = ipv6_addr_dnat;
     } 
    
   }



    @name(".session_rewrite")
      table table_session_rewrite {
        key = {
           metadata.cntrl.session_rewrite_id  : exact;
        } 
        actions  = {
	  session_rewrite;
	  session_rewrite_ipv4_snat;
	  session_rewrite_ipv4_dnat;
	  session_rewrite_ipv4_pat;
	  session_rewrite_ipv6_snat;
	  session_rewrite_ipv6_dnat;
	  
        }
        size  = SESSION_TABLE_SIZE;
        stage = 5;
        placement = HBM;
    }

#define SESSION_REWRITE_ENCAP_COMMON_FIELDS       bit<1> valid_flag,		\
                                            bit<1> add_vlan_tag_flag, \
                                            bit<48> dmac, \
                                            bit<48> smac, \
                                            bit<12> vlan


#define SESSION_REWRITE_ENCAP_COMMON_FIELDS_ARGS     valid_flag,		\
                                                     add_vlan_tag_flag, \
                                                     dmac, \
                                                     smac, \
                                                     vlan



   @name(".session_rewrite_encap_common") 
     action session_rewrite_encap_common(SESSION_REWRITE_ENCAP_COMMON_FIELDS) {
     if (valid_flag == TRUE) {
       hdr.ethernet_0.setValid();
       hdr.ethernet_0.dstAddr     = dmac;
       hdr.ethernet_0.srcAddr     = smac;

       if (add_vlan_tag_flag == TRUE) {
	 hdr.ctag_0.setValid();
	 hdr.ctag_0.vid   = vlan;
       }
       metadata.scratch.packet_len = hdr.p4i_to_p4e_header.packet_len;
     } else {
       metadata.cntrl.flow_miss = TRUE;
     }
   }

   @name(".session_rewrite_encap_l2") 
     action session_rewrite_encap_l2(SESSION_REWRITE_ENCAP_COMMON_FIELDS) {
      session_rewrite_encap_common(SESSION_REWRITE_ENCAP_COMMON_FIELDS_ARGS);
   }

   @name(".session_rewrite_encap_mplsoudp") 
     action session_rewrite_encap_mplsoudp(SESSION_REWRITE_ENCAP_COMMON_FIELDS,
					   bit <32> ipv4_sa,
					   bit <32> ipv4_da,
					   bit <16> udp_sport,
					   bit <16> udp_dport,
					   bit<20>    mpls_label1,
					   bit<20>    mpls_label2,
					   bit<20>    mpls_label3
					  
) {
      session_rewrite_encap_common(SESSION_REWRITE_ENCAP_COMMON_FIELDS_ARGS);
	 hdr.ip_0.ipv4.setValid();
	 //	 hdr.ip_0.ipv4.ttl = ip_ttl;
	 hdr.ip_0.ipv4.srcAddr = ipv4_sa;
	 hdr.ip_0.ipv4.dstAddr = ipv4_da;

	 hdr.l4_0.udp.setValid();
	 hdr.l4_0.udp.srcPort = udp_sport;
	 hdr.l4_0.udp.dstPort = udp_dport;
	 hdr.l4_0.udp.srcPort = hdr.p4i_to_p4e_header.hash[15:0];
	 hdr.mpls_label1_0.setValid();
	 hdr.mpls_label2_0.setValid();
	 hdr.mpls_label3_0.setValid();
	 hdr.mpls_label1_0.label = mpls_label1;
	 hdr.mpls_label2_0.label = mpls_label2;
	 hdr.mpls_label3_0.label = mpls_label3;
	 //	 hdr.ip_0.ipv4.totalLen = hdr.p4i_to_p4e_header.packet_len + 20 + 8 + 8;
	 
   }

   @name(".session_rewrite_encap_geneve") 
     action session_rewrite_encap_geneve(SESSION_REWRITE_ENCAP_COMMON_FIELDS,
					   bit <32> ipv4_sa,
					   bit <32> ipv4_da,
					   bit <16> udp_sport,
					   bit <16> udp_dport,
					 bit <24> vni, 
					 bit<32> source_slot_id,
					 bit<32> destination_slot_id,
					 bit<16> sg_id1,
					 bit<16> sg_id2,
					 bit<16> sg_id3,
					 bit<16> sg_id4,
					 bit<16> sg_id5,
					 bit<16> sg_id6,
					 bit<32> originator_physical_ip
					  
) {
      session_rewrite_encap_common(SESSION_REWRITE_ENCAP_COMMON_FIELDS_ARGS);
	 hdr.ip_0.ipv4.setValid();
	 //	 hdr.ip_0.ipv4.ttl = ip_ttl;
	 hdr.ip_0.ipv4.srcAddr = ipv4_sa;
	 hdr.ip_0.ipv4.dstAddr = ipv4_da;

	 hdr.l4_0.udp.setValid();
	 hdr.l4_0.udp.srcPort = udp_sport;
	 hdr.l4_0.udp.dstPort = udp_dport;
	 hdr.l4_0.udp.srcPort = hdr.p4i_to_p4e_header.hash[15:0];

	 metadata.scratch.vni = vni;
	 metadata.scratch.source_slot_id = source_slot_id;
	 metadata.scratch.destination_slot_id = destination_slot_id;
	 metadata.scratch.sg_id = sg_id1;
	 metadata.scratch.sg_id = sg_id2;
	 metadata.scratch.sg_id = sg_id3;
	 metadata.scratch.sg_id = sg_id4;
	 metadata.scratch.sg_id = sg_id5;
	 metadata.scratch.sg_id = sg_id6;
	 metadata.scratch.originator_physical_ip = originator_physical_ip;
	 
	 
   }

    @capi_bitfields_struct
    @name(".session_rewrite_encap")
      table table_session_encap {
        key = {
           metadata.cntrl.session_rewrite_id  : exact;
        } 
        actions  = {
	  session_rewrite_encap_l2;
	  session_rewrite_encap_mplsoudp;
	  session_rewrite_encap_geneve;	  
        }
	default_action = session_rewrite_encap_l2;
        size  = SESSION_TABLE_SIZE;
        stage = 5;
        placement = HBM;
    }

    apply {
      if(metadata.cntrl.flow_miss == FALSE) {
        session_info.apply();
      }
      if(metadata.cntrl.session_rewrite_id_valid == TRUE) {
        table_session_rewrite.apply();
	table_session_encap.apply();
      }
      
    }
}

