

#include "pds_flow_session_rewrite.h"


#define REWRITE_FILL_DATA(data, idx) \
    { \
        data.strip_encap_header = 0;\
        data.strip_l2_header = 1;\
        data.strip_vlan_tag = 1;\
        data.nat_info.nat_type = REWRITE_NAT_TYPE_IPV4_SNAT;\
        data.nat_info.u.ipv4_addr = 0x7000 + idx;\
        data.encap_type = ENCAP_TYPE_MPLSOUDP;\
        data.u.mplsoudp_encap.l2_encap.insert_vlan_tag = idx%2;\
        data.u.mplsoudp_encap.l2_encap.vlan_id = idx & 0xFFF;\
        data.u.mplsoudp_encap.ip_encap.ip_saddr = idx + 0x3000;\
        data.u.mplsoudp_encap.ip_encap.ip_daddr = idx + 0x4000;\
        data.u.mplsoudp_encap.udp_encap.udp_sport = idx & 0xFFFF;\
        data.u.mplsoudp_encap.udp_encap.udp_dport = (idx + 1) & 0xFFFF;\
        data.u.mplsoudp_encap.mpls1_label = idx & 0xfffff;\
        data.u.mplsoudp_encap.mpls2_label = (idx & 0xfffff) + 1;\
        data.u.mplsoudp_encap.mpls2_label = (idx & 0xfffff) + 3;\
    }

static inline void
test_pds_flow_session_rewrite_create (uint32_t idx) {

    pds_flow_session_rewrite_spec_t spec = {0};
    pds_ret_t                       ret = PDS_RET_OK;

    spec.key.session_rewrite_id = idx;
    REWRITE_FILL_DATA(spec.data, idx);

    ret = pds_flow_session_rewrite_create(&spec);
    assert(ret==PDS_RET_OK);
}

static inline void
test_pds_flow_session_rewrite_update (uint32_t idx) {

    pds_flow_session_rewrite_spec_t spec = {0};
    pds_ret_t                       ret = PDS_RET_OK;

    spec.key.session_rewrite_id = idx;
    REWRITE_FILL_DATA(spec.data, (idx+1));

    ret = pds_flow_session_rewrite_update(&spec);
    assert(ret==PDS_RET_OK);
}

static inline void
test_pds_flow_session_rewrite_read (uint32_t idx) {

    pds_flow_session_rewrite_key_t  key = {0};
    pds_flow_session_rewrite_info_t info = {0};
    pds_ret_t                       ret = PDS_RET_OK;

    key.session_rewrite_id = idx;
    ret = pds_flow_session_rewrite_read(&key, &info);
    assert(ret==PDS_RET_OK);
}

static inline void
test_pds_flow_session_rewrite_delete (uint32_t idx) {

    pds_flow_session_rewrite_key_t key = {0};
    pds_ret_t                       ret = PDS_RET_OK;

    key.session_rewrite_id = idx;
    ret = pds_flow_session_rewrite_delete(&key); 
    assert(ret==PDS_RET_OK);
}
