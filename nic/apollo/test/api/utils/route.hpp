//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#ifndef __TEST_API_UTILS_ROUTE_TABLE_HPP__
#define __TEST_API_UTILS_ROUTE_TABLE_HPP__

#include "nic/apollo/api/include/pds_route.hpp"
#include "nic/apollo/api/include/pds_nexthop.hpp"
#include "nic/apollo/test/api/utils/api_base.hpp"
#include "nic/apollo/test/api/utils/feeder.hpp"

namespace test {
namespace api {

// Route table test feeder class
class route_table_feeder : public feeder {
public:
    uint8_t af;
    bool enable_pbr;
    uint32_t num_routes;
    pds_route_table_spec_t spec;
    std::string base_route_pfx_str;

    route_table_feeder() {};
    route_table_feeder(const route_table_feeder& feeder) {
        pds_obj_key_t key = feeder.spec.key;
        init(feeder.base_route_pfx_str, feeder.af,
             feeder.num_routes, feeder.num_obj,
             pdsobjkey2int(key));
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
    void key_build(pds_obj_key_t *key) const;
    void spec_build(pds_route_table_spec_t *spec) const;
    void spec_fill(pds_nh_type_t type,
                   pds_route_table_spec_t *spec, uint32_t index) const;
    // Compare routines
    bool key_compare(const pds_obj_key_t *key) const;
    bool spec_compare(const pds_route_table_spec_t *spec) const;
    bool status_compare(const pds_route_table_status_t *status1,
                        const pds_route_table_status_t *status2) const;
};

// Dump prototypes
inline std::ostream&
operator<<(std::ostream& os, const pds_route_t *route) {
    os << " pfx: " << ippfx2str(&route->prefix)
       << " nh type: " << route->nh_type;
    switch (route->nh_type) {
    case PDS_NH_TYPE_OVERLAY:
        os << " TEP: " << route->tep.str();
        break;
    case PDS_NH_TYPE_IP:
        os << " NH: " << route->nh.str();
        break;
    case PDS_NH_TYPE_PEER_VPC:
        os << " vpc: " << route->vpc.str();
        break;
    default:
        break;
    }
    os << std::endl;
    return os;
}

inline std::ostream&
operator<<(std::ostream& os, const pds_route_table_spec_t *spec) {
    os << &spec->key
       << " af: " << +(spec->route_info ? spec->route_info->af : 0)
       << " num routes: " << (spec->route_info ?
                                  spec->route_info->num_routes : 0);
    if (spec->route_info) {
        for (uint32_t i = 0; i < spec->route_info->num_routes; i++) {
            os << &spec->route_info->routes[i];
        }
    }

    return os;
}

inline std::ostream&
operator<<(std::ostream& os, const pds_route_table_status_t *status) {
    os << " Base address: " << status->route_table_base_addr;
    return os;
}

inline std::ostream&
operator<<(std::ostream& os, const pds_route_table_stats_t *stats) {
    os << "  ";
    return os;
}

inline std::ostream&
operator<<(std::ostream& os, const pds_route_table_info_t *obj) {
    os << " Route Table info =>"
       << &obj->spec
       << &obj->status
       << &obj->stats
       << std::endl;
    return os;
}

inline std::ostream&
operator<<(std::ostream& os, const route_table_feeder& obj) {
    os << "Route table feeder => "
       << " key: " << obj.spec.key.str()
       << " af: " << +(obj.spec.route_info ? obj.spec.route_info->af : 0)
       << " num routes: " << (obj.spec.route_info ?
                                  obj.spec.route_info->num_routes : 0);
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

}    // namespace api
}    // namespace test

#endif    // __TEST_API_UTILS_ROUTE_TABLE_HPP__
