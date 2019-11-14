//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#ifndef __TEST_UTILS_IF_HPP__
#define __TEST_UTILS_IF_HPP__

#include "nic/apollo/api/include/pds_if.hpp"
#include "nic/apollo/test/utils/api_base.hpp"
#include "nic/apollo/test/utils/feeder.hpp"
#include "nic/apollo/api/if_utils.hpp"

namespace api_test {

// Export variables
extern std::string k_if_ippfx;
extern uint32_t k_l3_if_id;

// Interface test feeder class
class if_feeder : public feeder {
public:
    pds_if_spec_t spec_feeder;
    // Constructor
    if_feeder() { };
    if_feeder(const if_feeder& feeder) {
        init(feeder.spec_feeder.key.id,
             ippfx2str(&feeder.spec_feeder.l3_if_info.ip_prefix),
             feeder.spec_feeder.type, feeder.num_obj);
    }

    // Initialize feeder with the base set of values
    void init(pds_if_id_t id, std::string ip_pfx_str = k_if_ippfx,
              pds_if_type_t type = PDS_IF_TYPE_L3, int num_ifs = 2);

    // Iterate helper routines
    void iter_next(int width = 1);

    // Build routines
    void spec_build(pds_if_spec_t *spec) const;
    void key_build(pds_if_key_t *key) const;

    // Compare routines
    bool spec_compare(const pds_if_spec_t *spec) const;
    bool key_compare(const pds_if_key_t *key) const;
};

// Dump prototypes
inline std::ostream&
operator<<(std::ostream& os, const if_feeder& obj) {
    os << "Interface feeder =>" << &obj.spec_feeder;
    return os;
}

// CRUD prototypes
API_CREATE(if);
API_READ(if);
API_UPDATE(if);
API_DELETE(if);

// Misc function prototypes
void sample_if_setup(pds_batch_ctxt_t);
void sample_if_teardown(pds_batch_ctxt_t);

}    // namespace api_test

#endif    // __TEST_UTILS_IF_HPP__
