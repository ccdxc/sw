//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the route table test utility routines implementation
///
//----------------------------------------------------------------------------

#include "nic/apollo/test/utils/route.hpp"
#include "nic/apollo/test/utils/utils.hpp"

namespace api_test {

route_util::route_util() {
    this->nh_type = PDS_NH_TYPE_BLACKHOLE;
    this->vcn_id = PDS_VCN_ID_INVALID;
}

route_util::~route_util() {}

route_table_util::route_table_util() {
    this->id = 0;
    this->af = 0;
    this->num_routes = 0;
    this->routes = new route_util[0];
}

route_table_util::route_table_util(pds_route_table_id_t id,
                                   std::string first_route_pfx_str,
                                   std::string first_nh_ip_str, uint8_t af,
                                   uint32_t num_routes) {
    uint32_t nh_offset = 0;
    ip_prefix_t route_pfx;
    ip_addr_t route_addr, first_nh_ip;

    str2ipv4pfx((char *)first_route_pfx_str.c_str(), &route_pfx);
    extract_ip_addr((char *)first_nh_ip_str.c_str(), &first_nh_ip);
    this->id = id;
    this->af = af;
    this->first_route_pfx = route_pfx;
    this->first_nh_ip = first_nh_ip;
    this->num_routes = num_routes;
    this->routes = new route_util[num_routes];

    for (uint32_t i = 0; i < this->num_routes; ++i) {
        this->routes[i].ip_pfx = route_pfx;
        this->routes[i].nh_type = PDS_NH_TYPE_REMOTE_TEP;
        this->routes[i].vcn_id = PDS_VCN_ID_INVALID;

        // TODO: revisit for IPv6
        ip_prefix_ip_next(&route_pfx, &route_addr);
        route_pfx.addr = route_addr;

        // TODO: Increment first_nh_ip within the range (make it proper API)
        this->routes[i].nh_ip.addr.v4_addr = first_nh_ip.addr.v4_addr + nh_offset;
        nh_offset += 1;
        if (nh_offset >= 1024) {
            nh_offset %= 1024;
        }
    }
}

route_table_util::~route_table_util() {
    delete[] this->routes;
}

sdk::sdk_ret_t
route_table_util::create() {
    pds_route_table_spec_t spec;
    uint32_t i;

    memset(&spec, 0, sizeof(pds_route_table_spec_t));
    spec.key.id = this->id;
    spec.af = this->af;
    spec.num_routes = this->num_routes;
    spec.routes = (pds_route_t *)malloc((this->num_routes * sizeof(pds_route_t)));
    for (i = 0; i < this->num_routes; ++i) {
        spec.routes[i].prefix = this->routes[i].ip_pfx;
        spec.routes[i].nh_ip = this->routes[i].nh_ip;
        spec.routes[i].nh_type = this->routes[i].nh_type;
        spec.routes[i].vcn_id = this->routes[i].vcn_id;
    }
    return (pds_route_table_create(&spec));
}

#if 0
sdk::sdk_ret_t
route_table_util::read(pds_route_table_info_t *info, bool compare_spec) {
    sdk_ret_t rv;
    pds_route_table_key_t key;

    memset(&key, 0, sizeof(pds_route_table_key_t));
    key.id = this->id;
    rv = pds_route_table_read(&key, info);
    if (rv != sdk::SDK_RET_OK) {
        return rv;
    }
    if (compare_spec) {
    }
    return sdk::SDK_RET_OK;
}
#endif

sdk::sdk_ret_t
route_table_util::del() {
    pds_route_table_key_t key;

    memset(&key, 0, sizeof(pds_route_table_key_t));
    key.id = this->id;
    return (pds_route_table_delete(&key));
}

}    // namespace api_test
