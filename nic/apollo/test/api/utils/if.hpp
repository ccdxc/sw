//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#ifndef __TEST_API_UTILS_IF_HPP__
#define __TEST_API_UTILS_IF_HPP__

#include "nic/apollo/api/include/pds_if.hpp"
#include "nic/apollo/test/api/utils/api_base.hpp"
#include "nic/apollo/test/api/utils/feeder.hpp"

namespace test {
namespace api {

// Export variables
extern std::string k_if_ippfx;
extern pds_obj_key_t k_l3_if_key;

// Interface test feeder class
class if_feeder : public feeder {
public:
    pds_if_spec_t spec_feeder;

    // Constructor
    if_feeder() { };
    if_feeder(const if_feeder& feeder) {
        init(feeder.spec_feeder.key,
             ippfx2str(&feeder.spec_feeder.l3_if_info.ip_prefix),
             feeder.spec_feeder.type, feeder.num_obj);
    }

    // Initialize feeder with the base set of values
    void init(pds_obj_key_t key, std::string ip_pfx_str = k_if_ippfx,
              pds_if_type_t type = PDS_IF_TYPE_L3, int num_ifs = 2);

    // Iterate helper routines
    void iter_next(int width = 1);

    // Build routines
    void spec_build(pds_if_spec_t *spec) const;
    void key_build(pds_obj_key_t *key) const;

    // Compare routines
    bool spec_compare(const pds_if_spec_t *spec) const;
    bool key_compare(const pds_obj_key_t *key) const;
    bool status_compare(const pds_if_status_t *status1,
                        const pds_if_status_t *status2) const;
};

// Dump prototypes
inline std::ostream&
operator<<(std::ostream& os, const pds_if_spec_t *spec) {
    os << &spec->key
       << " type: " << spec->type;

    if (spec->type == PDS_IF_TYPE_L3) {
        os << " port: " << spec->l3_if_info.port.str()
           << " ip pfx: " << ippfx2str(&spec->l3_if_info.ip_prefix)
           << " mac addr: " << macaddr2str(spec->l3_if_info.mac_addr);
    }

    return os;
}

inline std::ostream&
operator<<(std::ostream& os, const pds_if_info_t *obj) {
    os << "Interface info =>" << &obj->spec << std::endl;
    return os;
}
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

}    // namespace api
}    // namespace test

#endif    // __TEST_API_UTILS_IF_HPP__
