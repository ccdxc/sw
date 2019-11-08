//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
#ifndef __TEST_UTILS_NH_GROUP_HPP__
#define __TEST_UTILS_NH_GROUP_HPP__

#include "nic/apollo/api/include/pds_nexthop.hpp"
#include "nic/apollo/api/nexthop_utils.hpp"
#include "nic/apollo/test/utils/api_base.hpp"
#include "nic/apollo/test/utils/feeder.hpp"

namespace api_test {

uint16_t k_max_groups = PDS_MAX_NEXTHOP_GROUP;

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
operator<<(std::ostream& os, const nexthop_group_feeder& obj) {
    os << "NH group feeder =>"
       << " id: " << obj.spec.key.id
       << " type: " << obj.spec.type
       << " num nexthops: " << unsigned(obj.spec.num_nexthops) << endl;
    for (uint8_t i = 0; i < obj.spec.num_nexthops; i++) {
        os << " nexthop" << unsigned(i) << ": ";
        if (obj.spec.nexthops[i].type == PDS_NH_TYPE_UNDERLAY) {
            os << " l3 if: " << obj.spec.nexthops[i].l3_if.id
               << " underlay mac: "
               << macaddr2str(obj.spec.nexthops[i].underlay_mac)
               << endl;
        } else if (obj.spec.nexthops[i].type == PDS_NH_TYPE_OVERLAY) {
            os << " tep id: " << obj.spec.nexthops[i].tep.id << endl;
        }
    }
    return os;
}

// CRUD prototypes
API_CREATE(nexthop_group);
API_READ(nexthop_group);
API_UPDATE(nexthop_group);
API_DELETE(nexthop_group);

// Misc function prototypes
void sample_nexthop_setup(pds_batch_ctxt_t bctxt);
void sample_nexthop_group_setup(pds_batch_ctxt_t bctxt);
void sample_nexthop_teardown(pds_batch_ctxt_t bctxt);
void sample_nexthop_group_teardown(pds_batch_ctxt_t bctxt);

}    // namespace api_test

#endif    // __TEST_UTILS_NH_GROUP_HPP__
