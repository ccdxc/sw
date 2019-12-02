//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
#ifndef __TEST_UTILS_TEP_HPP__
#define __TEST_UTILS_TEP_HPP__

#include "nic/apollo/api/include/pds_tep.hpp"
#include "nic/apollo/test/utils/api_base.hpp"
#include "nic/apollo/test/utils/feeder.hpp"

namespace api_test {

extern const pds_encap_t k_default_tep_encap;
extern const pds_encap_t k_zero_encap;
extern const pds_nexthop_key_t k_base_nh_key;
extern const pds_nexthop_group_key_t k_base_nh_group_key;
extern const uint64_t k_tep_mac;
extern const uint16_t k_max_tep;

// TEP test feeder class
class tep_feeder : public feeder {
public:
    pds_tep_spec_t spec;

    // Constructor
    tep_feeder() { };
    tep_feeder(const tep_feeder& feeder) {
        memcpy(&this->spec, &feeder.spec, sizeof(pds_tep_spec_t));
        this->num_obj = feeder.num_obj;
    }

    // Initialize feeder with the base set of values for apollo & artemis
    void init(uint32_t id, std::string ip_str, uint32_t num_tep=k_max_tep,
              pds_encap_t encap=k_default_tep_encap, bool nat=FALSE,
              pds_tep_type_t type=PDS_TEP_TYPE_WORKLOAD,
              std::string dipi_str="0.0.0.0", uint64_t dmac=k_tep_mac);

    // Initialize feeder with the base set of values for apulu
    void init(uint32_t id, uint64_t dmac, std::string ip_str,
              uint32_t num_tep=k_max_tep,
              pds_nh_type_t nh_type=PDS_NH_TYPE_UNDERLAY,
              pds_nexthop_key_t nh=k_base_nh_key,
              pds_nexthop_group_key_t base_nh_group=k_base_nh_group_key);

    // Iterate helper routines
    void iter_next(int width = 1);

    // Build routines
    void key_build(pds_tep_key_t *key) const;
    void spec_build(pds_tep_spec_t *spec) const;

    // Compare routines
    bool key_compare(const pds_tep_key_t *key) const;
    bool spec_compare(const pds_tep_spec_t *spec) const;
};

// Dump prototypes
inline std::ostream&
operator<<(std::ostream& os, const pds_tep_key_t *key) {
    os << " ID: " << key->id;
    return os;
}

inline std::ostream&
operator<<(std::ostream& os, const pds_tep_spec_t *spec) {
    os << &spec->key
       << " vpc: " << spec->vpc.id
       << " type: " << spec->type
       << " remote ip: " << spec->remote_ip
       << " DIPi: " << spec->ip_addr
       << " dmac: " << macaddr2str(spec->mac)
       << " nat: " << spec->nat
       << " encap: " << pds_encap2str(&spec->encap)
       << " remote svc: " << spec->remote_svc
       << " remote svc encap: " << pds_encap2str(&spec->remote_svc_encap)
       << " remote svc public ip: " << spec->remote_svc_public_ip
       << " nh type: " << spec->nh_type;
    switch (spec->nh_type) {
    case PDS_NH_TYPE_UNDERLAY_ECMP:
        os << " nh group id: " << spec->nh_group.id;
        break;
    case PDS_NH_TYPE_UNDERLAY:
        os << " nh id: " << spec->nh.id;
        break;
    default:
        break;
    }
    return os;
}

inline std::ostream&
operator<<(std::ostream& os, const pds_tep_status_t *status) {
    os << " HW id: " << status->hw_id
       << " NH id: " << status->nh_id
       << " DMAC: " << macaddr2str(status->dmac);
    return os;
}

inline std::ostream&
operator<<(std::ostream& os, const pds_tep_info_t *obj) {
    os << "TEP info =>"
       << &obj->spec
       << &obj->status
       << std::endl;
    return os;
}

inline std::ostream&
operator<<(std::ostream& os, const tep_feeder& obj) {
    os << "Tep feeder =>" << &obj.spec << " ";
    return os;
}

// CRUD prototypes
API_CREATE(tep);
API_READ(tep);
API_UPDATE(tep);
API_DELETE(tep);

// Misc function prototypes
void sample_tep_setup(pds_batch_ctxt_t bctxt, uint32_t tep_id=2,
                      std::string ip_str="30.30.30.1",
                      uint32_t num_tep=PDS_MAX_TEP);
void sample_tep_validate(uint32_t tep_id=2, std::string ip_str="30.30.30.1",
                         uint32_t num_tep=PDS_MAX_TEP);
void sample_tep_teardown(pds_batch_ctxt_t bctx, uint32_t tep_id=2,
                         std::string ip_str="30.30.30.1",
                         uint32_t num_tep=PDS_MAX_TEP);

}    // namespace api_test

#endif    // __TEST_UTILS_TEP_HPP__
