//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#ifndef __TEST_UTILS_ROUTE_TABLE_HPP__
#define __TEST_UTILS_ROUTE_TABLE_HPP__

#include "nic/apollo/api/include/pds_route.hpp"
#include "nic/apollo/api/include/pds_nexthop.hpp"
#include "nic/apollo/api/route_utils.hpp"
#include "nic/apollo/test/utils/api_base.hpp"
#include "nic/apollo/test/utils/feeder.hpp"

namespace api_test {

extern pds_nh_type_t g_rt_def_nh_type;

// Route table test feeder class
class route_table_feeder : public feeder {
public:
    pds_route_table_spec_t spec;
    std::string base_route_pfx_str;

    route_table_feeder() {};
    route_table_feeder(const route_table_feeder& feeder) {
        init(feeder.base_route_pfx_str, feeder.spec.af,
             feeder.spec.num_routes, feeder.num_obj,
             feeder.spec.key.id);
    }
    ~route_table_feeder() {};

    // Initialize feeder with the base set of values
    void init(std::string base_route_pfx_str,
              uint8_t af=IP_AF_IPV4,
              uint32_t num_routes=PDS_MAX_ROUTE_PER_TABLE,
              uint32_t num_route_tables=PDS_MAX_ROUTE_TABLE,
              uint32_t id = 1);

    // Iterate helper routines
    void iter_next(int width = 1);

    // Build routines
    void key_build(pds_route_table_key_t *key) const;
    void spec_build(pds_route_table_spec_t *spec) const;
    void fill_spec(pds_nh_type_t type,
                   pds_route_table_spec_t *spec, uint32_t index) const;
    // Compare routines
    bool key_compare(const pds_route_table_key_t *key) const;
    bool spec_compare(const pds_route_table_spec_t *spec) const;
};

// Dump prototypes
inline std::ostream&
operator<<(std::ostream& os, const route_table_feeder& obj) {
    os << "Route table feeder => "
       << " id: " << obj.spec.key.id
       << " af: " << +obj.spec.af
       << " num routes: " << obj.spec.num_routes;  
    return os;
}

// CRUD prototypes
API_CREATE(route_table);
API_READ(route_table);
API_UPDATE(route_table);
API_DELETE(route_table);

// Misc function prototypes
void sample_route_table_setup(
    pds_batch_ctxt_t bctxt, ip_prefix_t base_route_pfx,
    uint8_t af=IP_AF_IPV4, uint32_t num_routes=PDS_MAX_ROUTE_PER_TABLE,
    uint32_t num_route_tables=PDS_MAX_ROUTE_TABLE, uint32_t id=1);
void sample_route_table_teardown(
    pds_batch_ctxt_t bctxt, uint32_t id=1,
    uint32_t num_route_tables=PDS_MAX_ROUTE_TABLE);

}    // namespace api_test

#endif    // __TEST_UTILS_ROUTE_TABLE_HPP__
