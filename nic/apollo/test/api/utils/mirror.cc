//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#include "nic/apollo/test/api/utils/mirror.hpp"

namespace test {
namespace api {

//----------------------------------------------------------------------------
// Mirror feeder class routines
//----------------------------------------------------------------------------

void
mirror_session_feeder::init(pds_obj_key_t key, uint8_t max_ms,
                            pds_obj_key_t interface, uint16_t vlan_tag,
                            std::string src_ip, pds_obj_key_t tep,
                            uint32_t span_id, uint32_t dscp) {

    this->key =  key;
    // init with rspan type, then alternate b/w rspan and erspan in iter_next
    this->type = PDS_MIRROR_SESSION_TYPE_RSPAN;
    this->interface = interface;
    this->encap.type = PDS_ENCAP_TYPE_DOT1Q;
    this->encap.val.vlan_tag = vlan_tag;
    this->num_obj  = max_ms;
    this->vpc = int2pdsobjkey(1);
    this->tep = tep;
    memset(&this->src_ip, 0x0, sizeof(ip_addr_t));
    test::extract_ip_addr((char *)src_ip.c_str(), &this->src_ip);
    this->span_id = span_id;
    this->dscp =  dscp;
    this->snap_len = 100;
}

void
mirror_session_feeder::iter_next(int width) {
    key = int2pdsobjkey(pdsobjkey2int(key) + width);
    if (type == PDS_MIRROR_SESSION_TYPE_RSPAN) {
        type = PDS_MIRROR_SESSION_TYPE_ERSPAN;
        span_id++;
        dscp++;
        src_ip.addr.v4_addr += 1;
    } else if (type == PDS_MIRROR_SESSION_TYPE_ERSPAN) {
        type = PDS_MIRROR_SESSION_TYPE_RSPAN;
        if (encap.type  == PDS_ENCAP_TYPE_DOT1Q)
            encap.val.vlan_tag++;
        else if (encap.type == PDS_ENCAP_TYPE_QINQ) {
            encap.val.qinq_tag.c_tag++;
            encap.val.qinq_tag.s_tag++;
        }
    }
    cur_iter_pos++;
}

void
mirror_session_feeder::key_build(pds_obj_key_t *key) const {
    *key = this->key;
}

void
mirror_session_feeder::spec_build(pds_mirror_session_spec_t *spec) const {
    memset(spec, 0, sizeof(pds_mirror_session_spec_t));
    this->key_build(&spec->key);

    spec->type = type;
    spec->snap_len = snap_len;
    if (type  == PDS_MIRROR_SESSION_TYPE_RSPAN) {
        spec->rspan_spec.interface = interface;
        memcpy(&spec->rspan_spec.encap, &encap, sizeof(pds_encap_t));
    } else if (type == PDS_MIRROR_SESSION_TYPE_ERSPAN) {
        spec->erspan_spec.vpc = vpc;
        spec->erspan_spec.dscp = dscp;
        spec->erspan_spec.span_id = span_id;
        spec->erspan_spec.tep = tep;
        // todo: need to handle mapping key
    }
}
bool
mirror_session_feeder::key_compare(const pds_obj_key_t *key) const {
    if (this->key != *key)
        return false;
    return true;
}

bool
mirror_session_feeder::spec_compare(
                                const pds_mirror_session_spec_t *spec) const {
    if (spec->type != type)
        return false;

    if (spec->snap_len != snap_len)
        return false;

    if (spec->type == PDS_MIRROR_SESSION_TYPE_RSPAN) {
        // validate rspan spec
        if ((spec->rspan_spec.interface != interface) ||
            (spec->rspan_spec.encap.type != encap.type) ||
            (spec->rspan_spec.encap.val.vlan_tag != encap.val.vlan_tag))
            return false;

    } else if (type == PDS_MIRROR_SESSION_TYPE_ERSPAN) {
        // validate erspan spec
        if ((spec->erspan_spec.vpc != vpc) ||
            (spec->erspan_spec.dscp != dscp) ||
            (spec->erspan_spec.span_id != span_id) ||
             (spec->erspan_spec.tep != tep))
            return false;
    }
    return true;
}

bool
mirror_session_feeder::status_compare(
    const pds_mirror_session_status_t *status1,
    const pds_mirror_session_status_t *status2) const {
    return true;
}

}    // namespace api
}    // namespace test
