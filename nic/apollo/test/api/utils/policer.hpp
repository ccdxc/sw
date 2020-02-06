//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#ifndef __TEST_API_UTILS_POLICER_HPP__
#define __TEST_API_UTILS_POLICER_HPP__

#include "nic/sdk/include/sdk/qos.hpp"
#include "nic/apollo/api/include/pds_policer.hpp"
#include "nic/apollo/test/api/utils/api_base.hpp"
#include "nic/apollo/test/api/utils/feeder.hpp"

namespace test {
namespace api {

// Policer test feeder class
class policer_feeder : public feeder {
public:
    // Test params
    pds_policer_spec_t spec;

    // constructor
    policer_feeder() { };
    policer_feeder(policer_feeder& feeder) {
        init(feeder.spec.key, feeder.spec.type, feeder.spec.dir,
             feeder.spec.pps, feeder.spec.pps_burst, feeder.num_obj);
    }

    // Initialize feeder with the base set of values
    void init(pds_obj_key_t key, sdk::policer_type_t type, pds_policer_dir_t dir,
              uint64_t rate, uint64_t burst, uint32_t num_policer = 1);

    // Iterate helper routines
    void iter_next(int width = 1);

    // Build routines
    void key_build(pds_obj_key_t *key) const;
    void spec_build(pds_policer_spec_t *spec) const;

    // Compare routines
    bool key_compare(const pds_obj_key_t *key) const;
    bool spec_compare(const pds_policer_spec_t *spec) const;

};

// Dump prototypes
inline std::ostream&
operator<<(std::ostream& os, const pds_policer_spec_t *spec) {
    os << &spec->key
       << " type: " << (uint32_t)spec->type
       << " dir: " << (uint32_t)spec->dir
       << " rate: " << spec->pps
       << " burst: " << spec->pps_burst;
    return os;
}

inline std::ostream&
operator<<(std::ostream& os, const pds_policer_info_t *obj) {
    os << " Policer info =>" << &obj->spec << std::endl;
    return os;
}

inline std::ostream&
operator<<(std::ostream& os, const policer_feeder& obj) {
    os << "Policer feeder =>" << &obj.spec;
    return os;
}

// CRUD prototypes
API_CREATE(policer);
API_READ(policer);
API_UPDATE(policer);
API_DELETE(policer);

// Function prototypes
void sample_policer_setup(pds_batch_ctxt_t bctxt);
void sample_policer_teardown(pds_batch_ctxt_t bctxt);

}    // namespace api
}    // namespace test

#endif    // __TEST_API_UTILS_POLICER_HPP__
