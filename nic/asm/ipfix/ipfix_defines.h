#define IPFIX_VERSION           10
#define IPFIX_IPv4_RECORD_ID    257
#define IPFIX_IPv6_RECORD_ID    258
#define IPFIX_NON_IP_RECORD_ID  259

#define IPFIX_HEADER_SIZE \
    (CAPRI_PHV_START_OFFSET(ipfix_record_header_domain_id) - \
     CAPRI_PHV_END_OFFSET(ipfix_record_header_version))

#define IPFIX_IPv4_RECORD_SIZE \
    ((CAPRI_PHV_START_OFFSET(ipfix_record_common_drop_vector) - \
      CAPRI_PHV_END_OFFSET(ipfix_record_common_set_id)) + \
     (CAPRI_PHV_START_OFFSET(ipfix_record_ipv4_ip_da) - \
      CAPRI_PHV_END_OFFSET(ipfix_record_ipv4_ip_sa)) + \
     (CAPRI_PHV_START_OFFSET(ipfix_record_ip_ttl) - \
      CAPRI_PHV_END_OFFSET(ipfix_record_ip_flow_state_index)))

#define IPFIX_IPv6_RECORD_SIZE \
    ((CAPRI_PHV_START_OFFSET(ipfix_record_common_drop_vector) - \
      CAPRI_PHV_END_OFFSET(ipfix_record_common_set_id)) + \
     (CAPRI_PHV_START_OFFSET(ipfix_record_ipv6_ip_da) - \
      CAPRI_PHV_END_OFFSET(ipfix_record_ipv6_ip_sa)) + \
     (CAPRI_PHV_START_OFFSET(ipfix_record_ip_ttl) - \
      CAPRI_PHV_END_OFFSET(ipfix_record_ip_flow_state_index)))
