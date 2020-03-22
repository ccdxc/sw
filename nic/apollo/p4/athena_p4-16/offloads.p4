control offloads(inout cap_phv_intr_global_h capri_intrinsic,
                 inout cap_phv_intr_p4_h intr_p4,
                 inout headers hdr, 
                 inout metadata_t metadata) {

    
    @name(".offloads") action offloads_a() {
      //update ip_len
      if(hdr.p4plus_to_p4.update_ip_len == TRUE) {
	if(hdr.ctag_1.isValid()) {
	  metadata.scratch.packet_len = (bit<16>)(intr_p4.packet_len - 18);
	} else {
	  metadata.scratch.packet_len = (bit<16>)(intr_p4.packet_len - 14);
	}
	
	if(hdr.ip_1.ipv4.isValid()) {
	  hdr.ip_1.ipv4.totalLen = metadata.scratch.packet_len;
          metadata.scratch.packet_len = metadata.scratch.packet_len - (bit<16>)(hdr.ip_1.ipv4.ihl << 2);
	  metadata.scratch.update_ip_chksum = TRUE;
	} else {
	  if (hdr.ip_1.ipv6.isValid()) {
	    metadata.scratch.packet_len = metadata.scratch.packet_len - 40;
	    hdr.ip_1.ipv6.payloadLen = metadata.scratch.packet_len;
	  }
	}
	
      } else {
	if (hdr.ip_1.ipv4.isValid()) {
          metadata.scratch.packet_len = metadata.scratch.packet_len - (bit<16>)(hdr.ip_1.ipv4.ihl << 2);
	  
	} else {
	  if (hdr.ip_1.ipv6.isValid()) {
	    metadata.scratch.packet_len =  hdr.ip_1.ipv6.payloadLen;
	    
	  }
	}
      }

      //update L4 Length
      if(hdr.udp.isValid()) {
	metadata.scratch.packet_len =  (bit<16>)hdr.p4plus_to_p4.udp_opt_bytes;	
      }
      
      // update IP id
      if (hdr.p4plus_to_p4.update_ip_id == TRUE) {
	hdr.ip_1.ipv4.identification =  hdr.ip_1.ipv4.identification + hdr.p4plus_to_p4.ip_id_delta;
	metadata.scratch.update_ip_chksum = TRUE;
      }
      
      // update UDP length
      if (hdr.p4plus_to_p4.update_udp_len == 1) {
	hdr.udp.len = metadata.scratch.packet_len;
	metadata.scratch.update_l4_chksum = TRUE;
      }
      
      // update TCP sequence number
      if (hdr.p4plus_to_p4.update_tcp_seq_no == TRUE) {
	hdr.l4_u.tcp.seqNo = hdr.l4_u.tcp.seqNo + hdr.p4plus_to_p4.tcp_seq_delta;
	metadata.scratch.update_l4_chksum = TRUE;	
      }
      
      // tso
      if (hdr.p4plus_to_p4.tso_valid == TRUE) {
        if (hdr.p4plus_to_p4.tso_first_segment != TRUE) {
	  // reset CWR bit
	  hdr.l4_u.tcp.flags = hdr.l4_u.tcp.flags & ~(bit<8>)(TCP_FLAG_CWR);
	  metadata.scratch.update_l4_chksum = TRUE;	
        }
        if (hdr.p4plus_to_p4.tso_last_segment != TRUE) {
	  // reset FIN and PSH bits
	  hdr.l4_u.tcp.flags = hdr.l4_u.tcp.flags & ~(bit<8>)(TCP_FLAG_FIN|TCP_FLAG_PSH);
	  metadata.scratch.update_l4_chksum = TRUE;	
        }
      }
      
      // insert vlan tag
      if (hdr.p4plus_to_p4.insert_vlan_tag == TRUE) {
        hdr.ctag_1.setValid();
        hdr.ctag_1.pcp =  hdr.p4plus_to_p4_vlan.pcp;
        hdr.ctag_1.dei = hdr.p4plus_to_p4_vlan.dei;
        hdr.ctag_1.vid = hdr.p4plus_to_p4_vlan.vid;
        hdr.ctag_1.etherType = hdr.ethernet_1.etherType;
        hdr.ethernet_1.etherType = ETHERTYPE_VLAN;
        intr_p4.packet_len = intr_p4.packet_len + 4;
      }
      
      // copy gso valid to capri_gso_csum.gso
      metadata.scratch.flag = hdr.p4plus_to_p4.gso_valid;
      
      // update checksum compute flags
      if (hdr.p4plus_to_p4.p4plus_app_id == P4PLUS_APPTYPE_CLASSIC_NIC) {
	if (hdr.p4plus_to_p4.compute_ip_csum == 1) {
	  if (hdr.ip_1.ipv4.isValid()) {
	    metadata.scratch.update_ip_chksum = TRUE;
	  }
	}
	if (hdr.p4plus_to_p4.compute_l4_csum == 1) {
	  metadata.scratch.update_l4_chksum = TRUE;
	}
      } else {
	metadata.scratch.update_ip_chksum = TRUE;
	metadata.scratch.update_l4_chksum = TRUE;
	
      }
      
      if ((metadata.scratch.update_ip_chksum == TRUE) ||
	  (metadata.scratch.update_l4_chksum == TRUE)) {
	hdr.p4i_to_p4e_header.update_checksum=  TRUE;
      }
      
      
    }

    @name(".offloads") table offloads_t {

       actions = {
        offloads_a;
        }

	default_action = offloads_a;
        stage = 1;
    }

    apply {
      if(hdr.p4plus_to_p4.isValid()) {
	offloads_t.apply();
      }
    }
}

