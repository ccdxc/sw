#define IPFIX_VERSION                   10
#define IPFIX_IPv4_RECORD_ID            257
#define IPFIX_IPv6_RECORD_ID            258
#define IPFIX_NON_IP_RECORD_ID          259

#define IPFIX_EXPORT_ID_MAX             16

#define IPFIX_FLOW_HASH_TABLE           0
#define IPFIX_FLOW_HASH_OVERFLOW_TABLE  1

#define IPFIX_SCAN_COMPLETE             0xFFFE
#define IPFIX_SCAN_INCOMPLETE           0xFFFF

#define IPFIX_HEADER_SIZE \
    ((CAPRI_PHV_END_OFFSET(ipfix_record_header_domain_id) - \
      CAPRI_PHV_START_OFFSET(ipfix_record_header_version)) + 1)

#define IPFIX_IPv4_RECORD_SIZE \
    (((CAPRI_PHV_END_OFFSET(ipfix_record_common_drop_vector) - \
       CAPRI_PHV_START_OFFSET(ipfix_record_common_flow_id)) + 1) + \
     ((CAPRI_PHV_END_OFFSET(ipfix_record_ipv4_dport) - \
       CAPRI_PHV_START_OFFSET(ipfix_record_ipv4_set_id)) + 1) + \
     ((CAPRI_PHV_END_OFFSET(ipfix_record_ip_tcp_rtt) - \
      CAPRI_PHV_START_OFFSET(ipfix_record_ip_flow_state_index)) + 1))

#define IPFIX_IPv6_RECORD_SIZE \
    (((CAPRI_PHV_END_OFFSET(ipfix_record_common_drop_vector) - \
       CAPRI_PHV_START_OFFSET(ipfix_record_common_flow_id)) + 1) + \
     ((CAPRI_PHV_END_OFFSET(ipfix_record_ipv6_dport) - \
       CAPRI_PHV_START_OFFSET(ipfix_record_ipv6_set_id)) + 1) + \
     ((CAPRI_PHV_END_OFFSET(ipfix_record_ip_tcp_rtt) - \
      CAPRI_PHV_START_OFFSET(ipfix_record_ip_flow_state_index)) + 1))

#define IPFIX_NON_IP_RECORD_SIZE \
    (((CAPRI_PHV_END_OFFSET(ipfix_record_common_drop_vector) - \
       CAPRI_PHV_START_OFFSET(ipfix_record_common_flow_id)) + 1) + \
     ((CAPRI_PHV_END_OFFSET(ipfix_record_nonip_ether_type) - \
       CAPRI_PHV_START_OFFSET(ipfix_record_nonip_set_id)) + 1))

#define IPFIX_MAX_RECORD_SIZE IPFIX_IPv6_RECORD_SIZE
