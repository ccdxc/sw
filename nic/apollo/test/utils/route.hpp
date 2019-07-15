//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#ifndef __TEST_UTILS_ROUTE_TABLE_HPP__
#define __TEST_UTILS_ROUTE_TABLE_HPP__

#include "nic/apollo/api/include/pds_route.hpp"
#include "nic/apollo/test/utils/api_base.hpp"
#include "nic/apollo/test/utils/feeder.hpp"

namespace api_test {

extern pds_nh_type_t g_rt_def_nh_type;

/// Route test utility class
class route_util {
public:
    ip_prefix_t ip_pfx;
    ip_addr_t nh_ip;
    pds_nexthop_key_t nh;
    pds_nh_type_t nh_type;
    pds_vpc_id_t peer_vpc_id;

    route_util() {};
    ~route_util() {};
};

// Route table test feeder class
class route_table_feeder : public feeder {
public:
    pds_route_table_id_t id;
    uint8_t af;
    uint32_t num_routes;
    ip_prefix_t base_route_pfx;
    ip_addr_t base_nh_ip;
    pds_nh_type_t nh_type;
    pds_vpc_id_t peer_vpc_id;
    pds_nexthop_id_t base_nh_id;
    route_util routes[PDS_MAX_ROUTE_PER_TABLE + 1];

    route_table_feeder() {};
    route_table_feeder(const route_table_feeder& feeder) {
        init(ippfx2str(&feeder.base_route_pfx), ipaddr2str(&feeder.base_nh_ip),
             feeder.af, feeder.num_routes, feeder.num_obj, feeder.id,
             feeder.nh_type, feeder.peer_vpc_id, feeder.base_nh_id);
    }
    ~route_table_feeder() {};

    // Initialize feeder with the base set of values
    void init(std::string base_route_pfx_str,
              std::string base_nh_ip_str, uint8_t af=IP_AF_IPV4,
              uint32_t num_routes=PDS_MAX_ROUTE_PER_TABLE,
              uint32_t num_route_tables=PDS_MAX_ROUTE_TABLE,
              uint32_t id=1, pds_nh_type_t nh_type=g_rt_def_nh_type,
              pds_vpc_id_t peer_vpc_id=PDS_VPC_ID_INVALID,
              pds_nexthop_id_t base_nh_id=1);

    // Iterate helper routines
    void iter_next(int width = 1);

    bool read_unsupported(void) const { return true; }

    // Build routines
    void key_build(pds_route_table_key_t *key) const;
    void spec_build(pds_route_table_spec_t *spec) const;
};

// Dump prototypes
inline std::ostream&
operator<<(std::ostream& os, const route_table_feeder& obj) {
    os << "Route table feeder => "
       << "id: " << obj.id;
    return os;
}

// CRUD prototypes
API_CREATE(route_table);
API_NO_READ(route_table);
API_UPDATE(route_table);
API_DELETE(route_table);

// Misc function prototypes
void sample_route_table_setup(ip_prefix_t base_route_pfx, ip_addr_t base_nh_ip,
                              uint8_t af=IP_AF_IPV4,
                              uint32_t num_routes=PDS_MAX_ROUTE_PER_TABLE,
                              uint32_t num_route_tables=PDS_MAX_ROUTE_TABLE,
                              uint32_t id=1);
void sample_route_table_teardown(uint32_t id=1,
                                 uint32_t num_route_tables=PDS_MAX_ROUTE_TABLE);

}    // namespace api_test

#endif    // __TEST_UTILS_ROUTE_TABLE_HPP__
