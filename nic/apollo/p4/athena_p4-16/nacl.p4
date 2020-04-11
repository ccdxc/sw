/******************************************************************************/
/* Rx pipeline                                                                */
/******************************************************************************/
control nacl_lookup(inout cap_phv_intr_global_h capri_intrinsic,
             inout cap_phv_intr_p4_h intr_p4,
             inout headers hdr,
             inout metadata_t metadata) {

  /*
  
   @name(".nacl_key_action_0") action nacl_key_action_0() {
       metadata.nacl_lkp.direction        = metadata.cntrl.direction;
       metadata.nacl_lkp.uplink           = metadata.cntrl.uplink;

       if(metadata.tunnel.tunnel_type_1 == INGRESS_TUNNEL_TYPE_NONE) {
       
	    if (hdr.ip_1.ipv4.isValid()) {
	        metadata.nacl_lkp.ip_src           = (bit<128>)hdr.ip_1.ipv4.srcAddr ;
                metadata.nacl_lkp.ip_dst           = (bit<128>)hdr.ip_1.ipv4.dstAddr ;
                metadata.nacl_lkp.ip_dscp          = hdr.ip_1.ipv4.diffserv ;
                metadata.nacl_lkp.ip_proto         = hdr.ip_1.ipv4.protocol ;
                metadata.nacl_lkp.ip_ttl           = hdr.ip_1.ipv4.ttl ;
	    }
	    
	    if (hdr.ip_1.ipv6.isValid()) {
                metadata.nacl_lkp.ip_src           = hdr.ip_1.ipv6.srcAddr ;
                metadata.nacl_lkp.ip_dst           = hdr.ip_1.ipv6.dstAddr ;
                metadata.nacl_lkp.ip_dscp          = hdr.ip_1.ipv6.trafficClass;
                metadata.nacl_lkp.ip_proto         = hdr.ip_1.ipv6.nextHdr;
                metadata.nacl_lkp.ip_ttl           = hdr.ip_1.ipv6.hopLimit;
	    }

       } else {
	    if (hdr.ip_2.ipv4.isValid()) {
                metadata.nacl_lkp.ip_src           = (bit<128>)hdr.ip_2.ipv4.srcAddr ;
                metadata.nacl_lkp.ip_dst           = (bit<128>)hdr.ip_2.ipv4.dstAddr ;
                metadata.nacl_lkp.ip_dscp          = hdr.ip_2.ipv4.diffserv ;
                metadata.nacl_lkp.ip_proto         = hdr.ip_2.ipv4.protocol ;
                metadata.nacl_lkp.ip_ttl           = hdr.ip_2.ipv4.ttl ;
	    }
	    
	    if (hdr.ip_2.ipv6.isValid()) {
                metadata.nacl_lkp.ip_src           = hdr.ip_2.ipv6.srcAddr ;
                metadata.nacl_lkp.ip_dst           = hdr.ip_2.ipv6.dstAddr ;
                metadata.nacl_lkp.ip_dscp          = hdr.ip_2.ipv6.trafficClass;
                metadata.nacl_lkp.ip_proto         = hdr.ip_2.ipv6.nextHdr;
                metadata.nacl_lkp.ip_ttl           = hdr.ip_2.ipv6.hopLimit;
	    }

	 
       }
    }

   @name(".nacl_key_action_1") action nacl_key_action_1() {
       if(metadata.tunnel.tunnel_type_1 == INGRESS_TUNNEL_TYPE_NONE) {
       
            metadata.nacl_lkp.ethernet_dst     = hdr.ethernet_1.dstAddr;
            metadata.nacl_lkp.ethernet_src     = hdr.ethernet_1.srcAddr;
            metadata.nacl_lkp.ethernet_type    = hdr.ethernet_1.etherType;
            metadata.nacl_lkp.ctag             = hdr.ctag_1.vid;
            metadata.nacl_lkp.ctag_pcp         = hdr.ctag_1.pcp;
            metadata.nacl_lkp.ctag_dei         = hdr.ctag_1.dei;

            metadata.nacl_lkp.l4_sport         = metadata.l4.l4_sport_1;
            metadata.nacl_lkp.l4_dport         = metadata.l4.l4_dport_1;
            metadata.nacl_lkp.tcp_flags        = hdr.l4_u.tcp.flags ;
            metadata.nacl_lkp.tenant_id        = metadata.tunnel.tunnel_vni_1;
            metadata.nacl_lkp.gre_proto        = hdr.gre_1.proto;
	    metadata.nacl_lkp.tunnel_type      = metadata.tunnel.tunnel_type_1;
       } else {

            metadata.nacl_lkp.ethernet_dst     = hdr.ethernet_2.dstAddr;
            metadata.nacl_lkp.ethernet_src     = hdr.ethernet_2.srcAddr;
            metadata.nacl_lkp.ethernet_type    = hdr.ethernet_2.etherType;
            metadata.nacl_lkp.ctag             = hdr.ctag_2.vid;
            metadata.nacl_lkp.ctag_pcp              = hdr.ctag_2.pcp;
            metadata.nacl_lkp.ctag_dei              = hdr.ctag_2.dei;

            metadata.nacl_lkp.mpls_label_0     = hdr.mpls_0_1.label;
            metadata.nacl_lkp.l4_sport         = metadata.l4.l4_sport_2;
            metadata.nacl_lkp.l4_dport         = metadata.l4.l4_dport_2;
            metadata.nacl_lkp.tcp_flags        = hdr.l4_u.tcp.flags ;
            metadata.nacl_lkp.tenant_id        = metadata.tunnel.tunnel_vni_2;
            metadata.nacl_lkp.gre_proto        = hdr.gre_1.proto;
	    metadata.nacl_lkp.tunnel_type      = metadata.tunnel.tunnel_type_1;

	    
       }
    }

  */
   
   @name(".nop") action nop() {
    }
    
 
    @name(".nacl_drop")
      action nacl_drop() {
      	  DROP_PACKET_INGRESS(P4I_DROP_NACL);
      }

    @name(".nacl_permit")
      action nacl_permit() {
      //      if(!__table_hit()) {
      //	DROP_PACKET_INGRESS(P4I_DROP_NACL);
      // }
    }

    
    @name(".nacl_redirect")
      action nacl_redirect( bit <1> redir_type,
			    bit <4> app_id,
			    bit <4> oport,
			    bit <11> lif,
			    bit <3>  qtype,
 			    bit <24> qid) {
      metadata.cntrl.redir_type = redir_type;
      metadata.cntrl.redir_oport = oport;
      metadata.cntrl.redir_lif = lif;
      metadata.cntrl.redir_qtype = qtype;
      metadata.cntrl.redir_qid = qid;
      metadata.cntrl.redir_app_id = app_id;
      capri_intrinsic.drop = 0;
      metadata.cntrl.p4i_drop_reason = 0;

    } 

    
    @name(".nacl") table nacl {
        key = {
            metadata.cntrl.direction        : ternary;
            metadata.cntrl.flow_miss        : ternary;
 	    capri_intrinsic.lif : ternary;
        }
        actions = {
	  nacl_permit;
          nacl_redirect;
	  nacl_drop;
        }
        size = NACL_TABLE_SIZE;
        default_action = nacl_drop;
        stage = 3;
    }


    apply {
        nacl.apply();
    }

}

