//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#ifndef __TEST_API_UTILS_NH_GROUP_HPP__
#define __TEST_API_UTILS_NH_GROUP_HPP__

#include "nic/apollo/api/include/pds_nexthop.hpp"
#include "nic/apollo/test/api/utils/api_base.hpp"
#include "nic/apollo/test/api/utils/feeder.hpp"

namespace test {
namespace api {

// globals
static constexpr uint16_t k_max_groups = PDS_MAX_NEXTHOP_GROUP;

// NH group test feeder class
class nexthop_group_feeder : public feeder {
public:
    pds_nexthop_group_spec_t spec;

    // Constructor
    nexthop_group_feeder() { };
    nexthop_group_feeder(const nexthop_group_feeder& feeder) {
        init(feeder.spec.type, feeder.spec.key.id, feeder.num_obj,
             feeder.spec.num_nexthops);
    }

    // Initialize feeder with the base set of values
    void init(pds_nexthop_group_type_t type = PDS_NHGROUP_TYPE_UNDERLAY_ECMP,
              pds_nexthop_group_id_t id = 1,
              uint32_t num_obj = k_max_groups,
              uint8_t num_nexthops = PDS_MAX_ECMP_NEXTHOP);

    // Iterate helper routines
    void iter_next(int width = 1);

    // Build routines
    void key_build(pds_nexthop_group_key_t *key) const;
    void spec_build(pds_nexthop_group_spec_t *spec) const;

    // Compare routines
    bool key_compare(const pds_nexthop_group_key_t *key) const;
    bool spec_compare(const pds_nexthop_group_spec_t *spec) const;
};

// Dump prototypes
inline std::ostream&
operator<<(std::ostream& os, const pds_nexthop_group_key_t *key) {
    os << " id " << key->id;
    return os;
}

inline std::ostream&
operator<<(std::ostream& os, const pds_nexthop_group_spec_t *spec) {
    os << &spec->key
       << " type " << spec->type
       << " num nexthops " << unsigned(spec->num_nexthops);
    for (uint8_t i = 0; i < spec->num_nexthops; i++) {
        os << "nexthop " << unsigned(i + 1)
           << &spec->nexthops[i] << std::endl;
    }
    return os;
}

inline std::ostream&
operator<<(std::ostream& os, const pds_nexthop_group_status_t *status) {
    os << " HW id " << status->hw_id
       << "nh base idx " << status->nh_base_idx;
    return os;
}

inline std::ostream&
operator<<(std::ostream& os, const pds_nexthop_group_info_t *obj) {
    os << " NH group info =>"
       << &obj->spec
       << &obj->status
       << &obj->stats
       << std::endl;
    return os;
}

inline std::ostream&
operator<<(std::ostream& os, const nexthop_group_feeder& obj) {
    os << "NH group feeder =>"
       << &obj.spec;
    return os;
}

// CRUD prototypes
API_CREATE(nexthop_group);
API_READ(nexthop_group);
API_UPDATE(nexthop_group);
API_DELETE(nexthop_group);

// Misc function prototypes
void sample_nexthop_group_setup(pds_batch_ctxt_t bctxt);
void sample_nexthop_group_teardown(pds_batch_ctxt_t bctxt);

}    // namespace api
}    // namespace test

#endif    // __TEST_API_UTILS_NH_GROUP_HPP__
