
/*****************************************************************************/
/* Inter pipe : ingress pipeline                                             */
/*****************************************************************************/
control ingress_inter_pipe(inout cap_phv_intr_global_h capri_intrinsic,
			    inout cap_phv_intr_p4_h capri_p4_intrinsic,
			    inout headers hdr,
			    inout metadata_t metadata) {
  

  
  
   @name(".p4i_to_p4e") action p4i_to_p4e_a() {
     if(hdr.ingress_recirc_header.flow_done != TRUE ||
	hdr.ingress_recirc_header.dnat_done != TRUE) {
         /* Recirc back to P4I */
       hdr.ingress_recirc_header.setValid();
       hdr.ingress_recirc_header.direction = metadata.cntrl.direction;       
       capri_intrinsic.tm_oport = TM_PORT_INGRESS;
     } else {
        /* To P4E */
       hdr.ingress_recirc_header.setInvalid();
       hdr.p4i_to_p4e_header.setValid();
       hdr.p4i_to_p4e_header.flow_miss = metadata.cntrl.flow_miss;
       capri_intrinsic.tm_oport = TM_PORT_EGRESS;
     }
   }


    @name(".p4i_to_p4e") table p4i_to_p4e {
        actions  = {
            p4i_to_p4e_a;
        }
        default_action = p4i_to_p4e_a;
	stage = 5;
    }  

    apply{
      if(capri_intrinsic.drop == 0) {
	p4i_to_p4e.apply();
      }
    }
    
}

/*****************************************************************************/
/* Inter pipe : egress pipeline                                              */
/*****************************************************************************/

control egress_inter_pipe(inout cap_phv_intr_global_h capri_intrinsic,
			    inout cap_phv_intr_p4_h capri_p4_intrinsic,
			    inout headers hdr,
			    inout metadata_t metadata) {

  @name(".p4i_to_p4e_state") action p4i_to_p4e_state_a() {
    metadata.cntrl.direction = hdr.p4i_to_p4e_header.direction;
    //    metadata.cntrl.flow_miss = hdr.p4i_to_p4e_header.flow_miss;
     if(hdr.p4i_to_p4e_header.isValid()) {
       metadata.cntrl.update_checksum = hdr.p4i_to_p4e_header.update_checksum;
       if(hdr.p4i_to_p4e_header.flow_miss == FALSE) {
	 if(hdr.p4i_to_p4e_header.index_type == FLOW_CACHE_INDEX_TYPE_SESSION_INFO) {
	 
	   metadata.cntrl.session_index = hdr.p4i_to_p4e_header.index;
	   metadata.cntrl.session_index_valid = TRUE;
	 }
	 if(hdr.p4i_to_p4e_header.index_type == FLOW_CACHE_INDEX_TYPE_CONNTRACK_INFO) {
	   metadata.cntrl.conntrack_index = hdr.p4i_to_p4e_header.index[21:0];
	   metadata.cntrl.conntrack_index_valid = TRUE;
	 }
	 
	 metadata.cntrl.l2_session_index = hdr.p4i_to_p4e_header.l2_index;
	 metadata.cntrl.l2_session_index_valid = TRUE;
	
       }
     }
     
  }


  @name(".p4i_to_p4e_state") table p4i_to_p4e_state {
        actions  = {
            p4i_to_p4e_state_a;
        }
	  default_action = p4i_to_p4e_state_a;
	stage = 0;
    }  

  
  @name(".p4e_to_uplink") action p4e_to_uplink_a() {
       capri_intrinsic.tm_oport = metadata.cntrl.redir_oport;
       //      capri_intrinsic.lif = p4i_to_p4e_header.nacl_redir_lif;
      
   }


   @name(".p4e_to_rxdma") action p4e_to_rxdma_a() {
        capri_intrinsic.tm_oport = TM_PORT_DMA;
        capri_intrinsic.lif = metadata.cntrl.redir_lif;
	hdr.capri_rxdma_intrinsic.setValid();
	hdr.capri_rxdma_intrinsic.qid = metadata.cntrl.redir_qid;
	hdr.capri_rxdma_intrinsic.qtype = metadata.cntrl.redir_qtype;
	hdr.capri_rxdma_intrinsic.rx_splitter_offset = CAPRI_GLOBAL_INTRINSIC_HDR_SZ +
	  CAPRI_RXDMA_INTRINSIC_HDR_SZ + P4PLUS_CLASSIC_NIC_HDR_SZ;
	
	hdr.p4_to_p4plus_classic_nic.setValid();
	hdr.p4_to_p4plus_classic_nic.p4plus_app_id = metadata.cntrl.redir_app_id;
	hdr.p4_to_p4plus_classic_nic_ip.setValid();
	hdr.p4_to_p4plus_classic_nic.packet_len = (bit<16>)capri_p4_intrinsic.packet_len;
	if(hdr.ip_1.ipv4.isValid()) {
	  hdr.p4_to_p4plus_classic_nic_ip.ip_sa = (bit<128>)hdr.ip_1.ipv4.srcAddr;
	  hdr.p4_to_p4plus_classic_nic_ip.ip_da = (bit<128>)hdr.ip_1.ipv4.dstAddr;
	  if(hdr.l4_u.tcp.isValid()) {
	    hdr.p4_to_p4plus_classic_nic.pkt_type = CLASSIC_NIC_PKT_TYPE_IPV4_TCP;
	    hdr.p4_to_p4plus_classic_nic.l4_sport = hdr.l4_u.tcp.srcPort;
	    hdr.p4_to_p4plus_classic_nic.l4_dport = hdr.l4_u.tcp.dstPort;
	  } else {
	    if(hdr.udp.isValid()) {
	      hdr.p4_to_p4plus_classic_nic.pkt_type = CLASSIC_NIC_PKT_TYPE_IPV4_UDP;		    hdr.p4_to_p4plus_classic_nic.l4_sport = hdr.udp.srcPort;
	      hdr.p4_to_p4plus_classic_nic.l4_dport = hdr.udp.dstPort;
     
	    } else {
	      hdr.p4_to_p4plus_classic_nic.pkt_type = CLASSIC_NIC_PKT_TYPE_IPV4;	    
	    }
	  }
	}
	if(hdr.ip_1.ipv6.isValid()) {
	  hdr.p4_to_p4plus_classic_nic_ip.ip_sa = hdr.ip_1.ipv6.srcAddr;
	  hdr.p4_to_p4plus_classic_nic_ip.ip_da = hdr.ip_1.ipv6.dstAddr;
	  if(hdr.l4_u.tcp.isValid()) {
	    hdr.p4_to_p4plus_classic_nic.pkt_type = CLASSIC_NIC_PKT_TYPE_IPV6_TCP;
	    hdr.p4_to_p4plus_classic_nic.l4_sport = hdr.l4_u.tcp.srcPort;
	    hdr.p4_to_p4plus_classic_nic.l4_dport = hdr.l4_u.tcp.dstPort;
	    
	  } else {
	    if(hdr.udp.isValid()) {
	      hdr.p4_to_p4plus_classic_nic.pkt_type = CLASSIC_NIC_PKT_TYPE_IPV6_UDP;		    hdr.p4_to_p4plus_classic_nic.l4_sport = hdr.udp.srcPort;
	      hdr.p4_to_p4plus_classic_nic.l4_dport = hdr.udp.dstPort;
     
	    } else {
	      hdr.p4_to_p4plus_classic_nic.pkt_type = CLASSIC_NIC_PKT_TYPE_IPV6;	    
	    }
	  }
	}
   }
	     


    @name(".p4e_redir") table p4e_redir {
      key = {
	metadata.cntrl.redir_type : exact;
      }
        actions  = {
            p4e_to_rxdma_a;
	    p4e_to_uplink_a;
        }
        default_action = p4e_to_rxdma_a;
        placement = SRAM;
        size = P4E_REDIR_TABLE_SIZE; 
	stage = 5;
    }  


    apply{
      p4i_to_p4e_state.apply();
      p4e_redir.apply();

 
    }
}
