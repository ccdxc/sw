//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
#include "nic/apollo/api/include/pds_nexthop.hpp"
#include "nic/apollo/api/include/pds_tep.hpp"
#include "nic/apollo/test/utils/route.hpp"
#include "nic/apollo/test/utils/utils.hpp"

namespace api_test {

pds_nh_type_t g_rt_def_nh_type = apollo() ? PDS_NH_TYPE_TEP : PDS_NH_TYPE_IP;

//----------------------------------------------------------------------------
// Route table feeder class routines
//----------------------------------------------------------------------------

void
route_table_feeder::init(std::string base_route_pfx_str,
                         std::string base_nh_ip_str, uint8_t af,
                         uint32_t num_routes, uint32_t num_route_tables,
                         uint32_t id, pds_nh_type_t nh_type,
                         pds_vpc_id_t peer_vpc_id,
                         pds_nexthop_id_t base_nh_id) {
    uint32_t nh_offset = 0, nh_offset_max = PDS_MAX_TEP - 1;
    uint32_t nh_id_offset = 0, nh_id_offset_max = PDS_MAX_NEXTHOP - 1;
    ip_prefix_t base_route_pfx;
    ip_addr_t route_addr, base_nh_ip;

    extract_ip_pfx(base_route_pfx_str.c_str(), &base_route_pfx);
    extract_ip_addr(base_nh_ip_str.c_str(), &base_nh_ip);
    memset(routes, 0, sizeof(route_util) * (PDS_MAX_ROUTE_PER_TABLE + 1));
    this->id = id;
    this->af = af;
    this->base_route_pfx = base_route_pfx;
    this->base_nh_ip = base_nh_ip;
    this->base_nh_id = base_nh_id;
    this->peer_vpc_id = peer_vpc_id;
    this->nh_type = nh_type;
    this->num_obj = num_route_tables;
    this->num_routes = num_routes;
    for (uint32_t i = 0; i < this->num_routes; ++i) {
        this->routes[i].ip_pfx = base_route_pfx;
        this->routes[i].nh_type = nh_type;
        switch (nh_type) {
        case PDS_NH_TYPE_IP:
            this->routes[i].nh = base_nh_id + nh_id_offset;
            nh_id_offset += 1;
            if (nh_id_offset > nh_id_offset_max) {
                nh_id_offset %= nh_id_offset_max;
            }
            break;
        case PDS_NH_TYPE_TEP:
            this->routes[i].nh_ip.addr.v4_addr =
                base_nh_ip.addr.v4_addr + nh_offset;
            nh_offset += 1;
            if (nh_offset > nh_offset_max) {
                nh_offset %= nh_offset_max;
            }
            break;
        case PDS_NH_TYPE_PEER_VPC:
            this->routes[i].peer_vpc_id = peer_vpc_id;
            break;
        default:
            break;
        }
        ip_prefix_ip_next(&base_route_pfx, &route_addr);
        base_route_pfx.addr = route_addr;
    }
}

void
route_table_feeder::iter_next(int width) {
    id += width;
    cur_iter_pos++;
}

void
route_table_feeder::key_build(pds_route_table_key_t *key) const {
    memset(key, 0, sizeof(pds_route_table_key_t));
    key->id = this->id;
}

void
route_table_feeder::spec_build(pds_route_table_spec_t *spec) const {
    memset(spec, 0, sizeof(pds_route_table_spec_t));
    this->key_build(&spec->key);

    spec->af = this->af;
    spec->num_routes = this->num_routes;
    spec->routes = (pds_route_t *)calloc(spec->num_routes, sizeof(pds_route_t));
    for (uint32_t i = 0; i < this->num_routes; ++i) {
        spec->routes[i].prefix = this->routes[i].ip_pfx;
        spec->routes[i].nh_type = this->routes[i].nh_type;
        switch (this->routes[i].nh_type) {
        case PDS_NH_TYPE_IP:
            spec->routes[i].nh = this->routes[i].nh;
            break;
        case PDS_NH_TYPE_TEP:
            spec->routes[i].nh_ip = this->routes[i].nh_ip;
            break;
        case PDS_NH_TYPE_PEER_VPC:
            spec->routes[i].vpc.id = this->routes[i].peer_vpc_id;
            break;
        case PDS_NH_TYPE_BLACKHOLE:
        default:
            break;
        }
    }
}

//----------------------------------------------------------------------------
// Misc routines
//----------------------------------------------------------------------------

// do not modify these sample values as rest of system is sync with these
static route_table_feeder k_route_table_feeder;

void sample_route_table_setup(ip_prefix_t base_route_pfx, ip_addr_t base_nh_ip,
                              uint8_t af, uint32_t num_routes,
                              uint32_t num_route_tables, uint32_t id) {
    // setup and teardown parameters should be in sync
    k_route_table_feeder.init(ippfx2str(&base_route_pfx),
                              ipaddr2str(&base_nh_ip), af, num_routes,
                              num_route_tables, id);
    many_create(k_route_table_feeder);
}

void sample_route_table_teardown(uint32_t id, uint32_t num_route_tables) {
    k_route_table_feeder.init("0.0.0.0/0", "0.0.0.0", IP_AF_IPV4,
                              PDS_MAX_ROUTE_PER_TABLE, num_route_tables, id);
    many_delete(k_route_table_feeder);
}

}    // namespace api_test
