/******************************************************************************/
/* Tx pipeline                                                                */
/******************************************************************************/
control flow_log_key_init(inout cap_phv_intr_global_h intr_global,
             inout cap_phv_intr_p4_h intr_p4,
             inout headers hdr,
             inout metadata_t metadata) {

    @name(".nop") action nop() {
    }

    @name(".native_ipv4_packet") action native_ipv4_packet() {
        metadata.key.ktype =  KEY_TYPE_IPV4;
	metadata.key.vnic_id = hdr.p4i_to_p4e_header.vnic_id;
	metadata.key.src_mac = hdr.ethernet_1.ethernet.srcAddr;
	metadata.key.dst_mac = hdr.ethernet_1.ethernet.dstAddr;
	metadata.key.src = (bit<128>)hdr.ip_1.ipv4.srcAddr;
	metadata.key.dst = (bit<128>)hdr.ip_1.ipv4.dstAddr;
	metadata.key.proto = hdr.ip_1.ipv4.protocol;
	metadata.key.sport = metadata.l4.l4_sport_1;
	metadata.key.dport = metadata.l4.l4_dport_1;
    }

   @name(".native_ipv6_packet") action native_ipv6_packet() {
        metadata.key.ktype =  KEY_TYPE_IPV6;
	metadata.key.vnic_id = hdr.p4i_to_p4e_header.vnic_id;
	metadata.key.src_mac = hdr.ethernet_1.ethernet.srcAddr;
	metadata.key.dst_mac = hdr.ethernet_1.ethernet.dstAddr;
	metadata.key.src = hdr.ip_1.ipv6.srcAddr;
	metadata.key.dst = hdr.ip_1.ipv6.dstAddr;
	metadata.key.proto = hdr.ip_1.ipv6.nextHdr;
	metadata.key.sport = metadata.l4.l4_sport_1;
	metadata.key.dport = metadata.l4.l4_dport_1;
    }

   @name(".native_nonip_packet") action native_nonip_packet() {
        metadata.cntrl.skip_flow_lkp = TRUE;
    }

    @name(".tunneled_ipv4_packet") action tunneled_ipv4_packet() {
        metadata.key.ktype =  KEY_TYPE_IPV4;
	metadata.key.vnic_id = hdr.p4i_to_p4e_header.vnic_id;
	metadata.key.src_mac = hdr.ethernet_2.ethernet.dstAddr;
	metadata.key.dst_mac = hdr.ethernet_2.ethernet.srcAddr;
	metadata.key.src = (bit<128>)hdr.ip_2.ipv4.dstAddr;
	metadata.key.dst = (bit<128>)hdr.ip_2.ipv4.srcAddr;
	metadata.key.proto = hdr.ip_2.ipv4.protocol;
	metadata.key.sport = metadata.l4.l4_dport_2;
	metadata.key.dport = metadata.l4.l4_sport_2;
	
    }

    @name(".tunneled_ipv6_packet") action tunneled_ipv6_packet() {
        metadata.key.ktype =  KEY_TYPE_IPV6;
	metadata.key.vnic_id = hdr.p4i_to_p4e_header.vnic_id;
	metadata.key.src_mac = hdr.ethernet_2.ethernet.dstAddr;
	metadata.key.dst_mac = hdr.ethernet_2.ethernet.srcAddr;
	metadata.key.src = hdr.ip_2.ipv6.dstAddr;
	metadata.key.dst = hdr.ip_2.ipv6.srcAddr;
	metadata.key.proto = hdr.ip_2.ipv6.nextHdr;
	metadata.key.sport = metadata.l4.l4_dport_2;
	metadata.key.dport = metadata.l4.l4_sport_2;	
    }

   @name(".tunneled_nonip_packet") action tunneled_nonip_packet() {
        metadata.cntrl.skip_flow_lkp = TRUE;
    }

    
    @name(".key_native") table key_native {
        key = {
	metadata.cntrl.direction                      : ternary;
            hdr.ip_1.ipv4.isValid()                   : ternary;
            hdr.ip_1.ipv6.isValid()                   : ternary;
            hdr.ethernet_2.ethernet.isValid()         : ternary;
            hdr.ip_2.ipv4.isValid()                   : ternary;
            hdr.ip_2.ipv6.isValid()                   : ternary;
        }
        actions  = {
            native_ipv4_packet;
            native_ipv6_packet;
            native_nonip_packet;
            nop;
        }
        size  = KEY_MAPPING_TABLE_SIZE;
        stage = 0;
        default_action = nop;
    }

    @name(".key_tunneled") table key_tunneled {
        key = {
            metadata.cntrl.direction                  : ternary;
            hdr.ip_1.ipv4.isValid()                   : ternary;
            hdr.ip_1.ipv6.isValid()                   : ternary;
            hdr.ethernet_2.ethernet.isValid()         : ternary;
            hdr.ip_2.ipv4.isValid()                   : ternary;
            hdr.ip_2.ipv6.isValid()                   : ternary;
        }
        actions  = {
            nop;
            tunneled_ipv4_packet;
            tunneled_ipv6_packet;
            tunneled_nonip_packet;
        }
        size  = KEY_MAPPING_TABLE_SIZE;
        stage = 1;
        default_action = nop;
    }


    
  
    apply {
      key_tunneled.apply();
      //      init_config.apply();
      //      if(metadata.cntrl.direction == TX_FROM_HOST) {
        key_native.apply();
	//      }
    }
}



