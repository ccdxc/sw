control ValidatePacket(inout cap_phv_intr_global_h capri_intrinsic,
                       inout cap_phv_intr_p4_h intr_p4,
                       inout headers hdr,
                       inout metadata_t metadata) {

    @name(".check_parser_errors") action check_parser_errors(inout bool parser_err_drop) {
        if (intr_p4.parser_err == 1) {
            if (metadata.cntrl.uplink == TRUE) {
                if (intr_p4.len_err != 0) {
                    DROP_PACKET_INGRESS(DROP_PARSER_LEN_ERR);
                    parser_err_drop = true;
                } else {
                    if (hdr.ip_1.ipv4.isValid()) {
                        if (ipv4HdrCsum_1.get_validate_status() == 0) {
                            DROP_PACKET_INGRESS(DROP_PARSER_CSUM_ERR);
                            parser_err_drop = true;
                        }			
			if (hdr.l4_u.icmp.isValid()) {
			  if (icmpCsum_1.get_validate_status() == 0) {
			    DROP_PACKET_INGRESS(DROP_PARSER_CSUM_ERR);
			    parser_err_drop = true;
			  }
			} else if (hdr.l4_u.tcp.isValid()) {
			  if (tcpCsum_1.get_validate_status() == 0) {
                            DROP_PACKET_INGRESS(DROP_PARSER_CSUM_ERR);
                            parser_err_drop = true;
			  }			  
			} else if (hdr.udp.isValid() && (metadata.tunnel.tunnel_type_1 == INGRESS_TUNNEL_TYPE_NONE)) {
			  if (udpCsum_1.get_validate_status() == 0) {
			    DROP_PACKET_INGRESS(DROP_PARSER_CSUM_ERR);
			    parser_err_drop = true;
			  }
			}
		    }

		    if (hdr.ip_2.ipv4.isValid()) {
                        if (ipv4HdrCsum_2.get_validate_status() == 0) {
                            DROP_PACKET_INGRESS(DROP_PARSER_CSUM_ERR);
                            parser_err_drop = true;
                        }
                    }
		    if (hdr.ip_2.ipv4.isValid() || hdr.ip_2.ipv6.isValid()) {
			if (hdr.l4_u.icmp.isValid()) {
			  if (icmpCsum_2.get_validate_status() == 0) {
			    DROP_PACKET_INGRESS(DROP_PARSER_CSUM_ERR);
			    parser_err_drop = true;
			  }
			} else if (hdr.l4_u.tcp.isValid()) {
			  if (tcpCsum_2.get_validate_status() == 0) {
                            DROP_PACKET_INGRESS(DROP_PARSER_CSUM_ERR);
                            parser_err_drop = true;
			  }			  
			} else if (hdr.l4_u.udp.isValid()) {
			  if (udpCsum_2.get_validate_status() == 0) {
			    DROP_PACKET_INGRESS(DROP_PARSER_CSUM_ERR);
			    parser_err_drop = true;
			  }
			}
			
		    }
                }
            } else {
	      if ((intr_p4.len_err != 0) &&
		  ((hdr.p4plus_to_p4.update_ip_len == 0) &&
		   (hdr.p4plus_to_p4.update_udp_len == 0))) {
		DROP_PACKET_INGRESS(DROP_PARSER_LEN_ERR);
		parser_err_drop = true;
                }
            }
        }
    }
    

    @name(".validate_headers") action validate_headers() {
        bool parser_err_drop = false;
        check_parser_errors(parser_err_drop);
        if (parser_err_drop) {
            return;
        }

    }

    @name(".validate_packet") table validate_packet {
        actions = {
            validate_headers;
        }
        default_action = validate_headers;
        stage = 1;
    }

    apply {
        validate_packet.apply();
    }
}
