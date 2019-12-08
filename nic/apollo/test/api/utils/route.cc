//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#include "nic/apollo/api/include/pds_tep.hpp"
#include "nic/apollo/test/api/utils/route.hpp"

namespace test {
namespace api {

pds_nh_type_t nhtype_supported[] = {
    PDS_NH_TYPE_OVERLAY,
    PDS_NH_TYPE_PEER_VPC,
#ifdef ARTEMIS
    PDS_NH_TYPE_IP,
#endif
#ifdef APULU
    PDS_NH_TYPE_OVERLAY_ECMP,
    PDS_NH_TYPE_VNIC,
#endif
};

pds_nh_type_t g_rt_def_nh_type = (apulu() || apollo()) ? PDS_NH_TYPE_OVERLAY
                                                         : PDS_NH_TYPE_IP;

uint8_t get_num_supported_types (void)
{
    return sizeof(nhtype_supported)/sizeof(nhtype_supported[0]);
}

//----------------------------------------------------------------------------
// Route table feeder class routines
//----------------------------------------------------------------------------

void
route_table_feeder::init(std::string base_route_pfx_str,
                         uint8_t af, uint32_t num_routes,
                         uint32_t num_route_tables,
                         uint32_t id) {
    spec.key.id = id;
    spec.af = af;
    spec.num_routes = num_routes;
    this->base_route_pfx_str = base_route_pfx_str;
    this->num_obj = num_route_tables;
}

void
route_table_feeder::iter_next(int width) {
    spec.key.id += width;
    cur_iter_pos++;
}

void
route_table_feeder::key_build(pds_route_table_key_t *key) const {
    memset(key, 0, sizeof(pds_route_table_key_t));
    key->id = this->spec.key.id;
}

void
route_table_feeder::spec_build(pds_route_table_spec_t *spec) const {
    ip_prefix_t route_pfx;
    ip_addr_t route_addr;
    uint32_t num_routes_per_type, route_index;
    uint32_t num_types;

    test::extract_ip_pfx(base_route_pfx_str.c_str(), &route_pfx);

    this->key_build(&spec->key);
    spec->af = this->spec.af;
    spec->num_routes = this->spec.num_routes;
    if (spec->num_routes) {
        spec->routes = (pds_route_t *)SDK_CALLOC(PDS_MEM_ALLOC_ID_ROUTE_TABLE,
                                      (spec->num_routes * sizeof(pds_route_t)));
    }
    num_types = get_num_supported_types();
    num_routes_per_type = spec->num_routes/num_types;

    route_index = 0;
    for (uint32_t i = 0; i < num_types; i++) {
        for (uint32_t j = 0; j < num_routes_per_type; j++) {
            spec->routes[route_index].prefix = route_pfx;
            fill_spec(nhtype_supported[i], spec, route_index);
            ip_prefix_ip_next(&route_pfx, &route_addr);
            route_pfx.addr = route_addr;
            route_index++;
        }
    }
    while (route_index < spec->num_routes) {
        spec->routes[route_index].prefix = route_pfx;
        fill_spec(g_rt_def_nh_type, spec, route_index);
        ip_prefix_ip_next(&route_pfx, &route_addr);
        route_pfx.addr = route_addr;
        route_index++;
    }
}

void
route_table_feeder::fill_spec(pds_nh_type_t type,
                              pds_route_table_spec_t *spec,
                              uint32_t index) const {
    uint32_t num = 0;
    uint32_t base_id = 1;
    uint32_t base_tep_id = 2;

    spec->routes[index].nh_type = type;
    switch (type) {
    case PDS_NH_TYPE_OVERLAY:
        num = (index % PDS_MAX_TEP);
        spec->routes[index].tep.id = base_tep_id + num;
        break;
    case PDS_NH_TYPE_OVERLAY_ECMP:
        num = (index % PDS_MAX_NEXTHOP_GROUP);
        spec->routes[index].nh_group.id = base_id + num;
        break;
    case PDS_NH_TYPE_PEER_VPC:
        num = (index % PDS_MAX_VPC);
        spec->routes[index].vpc.id = base_id + num;
        break;
    case PDS_NH_TYPE_VNIC:
        num = (index % PDS_MAX_VNIC);
        spec->routes[index].vnic.id = base_id + num;
        break;
    case PDS_NH_TYPE_IP:
        num = (index % PDS_MAX_NEXTHOP);
        spec->routes[index].nh.id = base_id + num;
        break;
    case PDS_NH_TYPE_BLACKHOLE:
    default:
        break;
    }
}

bool
route_table_feeder::key_compare(const pds_route_table_key_t *key) const {
    if (key->id != spec.key.id)
        return false;
    return true;
}

bool
route_table_feeder::spec_compare(const pds_route_table_spec_t *spec) const {
    if (spec->af != this->spec.af)
        return false;
    if (spec->num_routes != 0)
        return false;
    return true;
}

//----------------------------------------------------------------------------
// Misc routines
//----------------------------------------------------------------------------

// do not modify these sample values as rest of system is sync with these
static route_table_feeder k_route_table_feeder;

void sample_route_table_setup(
    pds_batch_ctxt_t bctxt, ip_prefix_t base_route_pfx, uint8_t af,
    uint32_t num_routes, uint32_t num_route_tables, uint32_t id) {
    // setup and teardown parameters should be in sync
    k_route_table_feeder.init(ippfx2str(&base_route_pfx),
                              af, num_routes,
                              num_route_tables, id);
    many_create(bctxt, k_route_table_feeder);
}

void sample_route_table_teardown(pds_batch_ctxt_t bctxt, uint32_t id,
                                 uint32_t num_route_tables) {
    k_route_table_feeder.init("0.0.0.0/0", IP_AF_IPV4,
                              PDS_MAX_ROUTE_PER_TABLE, num_route_tables, id);
    many_delete(bctxt, k_route_table_feeder);
}

}    // namespace api
}    // namespace test
