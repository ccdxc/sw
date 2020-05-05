//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#ifndef __TEST_API_UTILS_ROUTE_TABLE_HPP__
#define __TEST_API_UTILS_ROUTE_TABLE_HPP__

#include "nic/apollo/api/include/pds_route.hpp"
#include "nic/apollo/api/include/pds_nexthop.hpp"
#include "nic/apollo/test/api/utils/api_base.hpp"
#include "nic/apollo/test/api/utils/batch.hpp"
#include "nic/apollo/test/api/utils/feeder.hpp"

namespace test {
namespace api {

//TODO: setting to 5 for now, MAX fails
#define NUM_ROUTE_TABLES 5

// Enumerate route_table attributes
enum route_table_attrs {
    ROUTE_TABLE_ATTR_AF                   =  bit(0),
    ROUTE_TABLE_ATTR_ROUTES               =  bit(1),
    ROUTE_TABLE_ATTR_PRIORITY_EN          =  bit(2),
};

// Route table test feeder class
class route_table_feeder : public feeder {
public:
    pds_route_table_spec_t spec;
    std::string base_route_pfx_str;
    route_table_feeder() {};
    route_table_feeder(const route_table_feeder& feeder) {
        pds_obj_key_t key = feeder.spec.key;
        init(feeder.base_route_pfx_str, feeder.spec.route_info->af,
             feeder.spec.route_info->num_routes, feeder.num_obj,
             pdsobjkey2int(key), feeder.spec.route_info->priority_en);
    }
    ~route_table_feeder() {};

    // Initialize feeder with the base set of values
    void init(std::string base_route_pfx_str,
              uint8_t af=IP_AF_IPV4,
              uint32_t num_routes=16,
              uint32_t num_route_tables=1,
              uint32_t id = 1,
              bool priority_en = false);

    // Iterate helper routines
    void iter_next(int width = 1);

    // Init routine
    void spec_alloc(pds_route_table_spec_t *spec);
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
    os << " pfx: " << ippfx2str(&route->attrs.prefix)
       << " nh type: " << route->attrs.nh_type;
    switch (route->attrs.nh_type) {
    case PDS_NH_TYPE_OVERLAY:
        os << " TEP: " << route->attrs.tep.str();
        break;
    case PDS_NH_TYPE_IP:
        os << " NH: " << route->attrs.nh.str();
        break;
    case PDS_NH_TYPE_PEER_VPC:
        os << " vpc: " << route->attrs.vpc.str();
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
                                  spec->route_info->num_routes : 0)
        << " priority enable: " << (spec->route_info ?
                                    spec->route_info->priority_en : false);
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
                                  obj.spec.route_info->num_routes : 0)
       << " priority enable: " << (obj.spec.route_info ?
                                  obj.spec.route_info->priority_en : false);
    return os;
}

// CRUD prototypes
API_CREATE(route_table);
API_READ(route_table);
API_UPDATE(route_table);
API_DELETE(route_table);

// Route_table CRUD helper functions
void create_route_table_spec(std::string base_route_pfx_str,uint8_t af,
                             uint32_t num_routes,uint32_t num_route_tables,
                             pds_route_table_spec_t *spec,
                             bool priority_en = false);
void route_table_create(route_table_feeder& feeder);
void route_table_read(route_table_feeder& feeder,
                      sdk_ret_t exp_result = SDK_RET_OK);
void route_table_update(route_table_feeder& feeder,
                        pds_route_table_spec_t *spec,
                        uint64_t chg_bmap, sdk_ret_t exp_result = SDK_RET_OK);
void route_table_delete(route_table_feeder& feeder);

// Misc function prototypes
void sample_route_table_setup(
    pds_batch_ctxt_t bctxt, const string base_route_pfx="10.0.0.1/16",
    uint8_t af=IP_AF_IPV4, uint32_t num_routes=PDS_MAX_ROUTE_PER_TABLE,
    uint32_t num_route_tables=NUM_ROUTE_TABLES, uint32_t id=1);
void sample_route_table_teardown(
    pds_batch_ctxt_t bctxt, uint32_t id=1,
    uint32_t num_route_tables=NUM_ROUTE_TABLES);

}    // namespace api
}    // namespace test

#endif    // __TEST_API_UTILS_ROUTE_TABLE_HPP__
