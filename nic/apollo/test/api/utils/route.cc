//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#include "nic/apollo/api/include/pds_tep.hpp"
#include "nic/apollo/test/api/utils/route.hpp"

namespace test {
namespace api {

static const pds_nh_type_t k_rt_def_nh_type = (apulu() || apollo()) ?
                                               PDS_NH_TYPE_OVERLAY
                                               : PDS_NH_TYPE_IP;
static void
set_nhtype_bit (uint16_t &bmap, uint8_t pos)
{
    bmap |= ((uint16_t)1 << pos);
}

uint8_t
learn_nhtypes_supported (uint16_t &nhtypes_bmap)
{
    uint8_t num_types = 0;

    set_nhtype_bit(nhtypes_bmap, PDS_NH_TYPE_OVERLAY);
    set_nhtype_bit(nhtypes_bmap, PDS_NH_TYPE_PEER_VPC);
    num_types += 2;

    if (artemis()) {
        set_nhtype_bit(nhtypes_bmap, PDS_NH_TYPE_IP);
        num_types++;
    } else if (apulu()) {
        set_nhtype_bit(nhtypes_bmap, PDS_NH_TYPE_OVERLAY_ECMP);
        set_nhtype_bit(nhtypes_bmap, PDS_NH_TYPE_VNIC);
        num_types += 2;
    }

    return num_types;
}

static uint16_t g_nhtypes_supported_bmap;
static const uint8_t k_num_nhtypes_supported = learn_nhtypes_supported(g_nhtypes_supported_bmap);

//----------------------------------------------------------------------------
// Route table feeder class routines
//----------------------------------------------------------------------------

void
route_table_feeder::init(std::string base_route_pfx_str,
                         uint8_t af, uint32_t num_routes,
                         uint32_t num_route_tables,
                         uint32_t id, bool priority_en) {
    memset(&spec, 0, sizeof(pds_route_table_spec_t));
    spec.key = int2pdsobjkey(id);
    this->base_route_pfx_str = base_route_pfx_str;
    num_obj = num_route_tables;
    create_route_table_spec(base_route_pfx_str, af, num_routes,
                            num_obj, &spec, priority_en);
}

void
route_table_feeder::iter_next(int width) {
    spec.key = int2pdsobjkey(pdsobjkey2int(spec.key) + width);
    cur_iter_pos++;
}

void
route_table_feeder::key_build(pds_obj_key_t *key) const {
    memcpy(key, &this->spec.key, sizeof(pds_obj_key_t));
}

void
route_table_feeder::spec_alloc(pds_route_table_spec_t *spec) {
    spec->route_info =
        (route_info_t *)SDK_CALLOC(PDS_MEM_ALLOC_ID_ROUTE_TABLE,
                                   ROUTE_INFO_SIZE(0));
    // To-do we need to free this allocated memory during delete
}

void
route_table_feeder::spec_build(pds_route_table_spec_t *spec) const {
    memcpy(spec, &this->spec, sizeof(pds_route_table_spec_t));
    create_route_table_spec(this->base_route_pfx_str, this->spec.route_info->af,
                            this->spec.route_info->num_routes, this->num_obj,
                            (pds_route_table_spec_t *)spec,
                            this->spec.route_info->priority_en);
}

void
route_table_feeder::spec_fill(pds_nh_type_t type,
                              pds_route_table_spec_t *spec,
                              uint32_t index) const {
    uint32_t num = 0;
    uint32_t base_id = 1;
    uint32_t base_tep_id = 2;

    spec->route_info->routes[index].key = int2pdsobjkey(index + 1);
    spec->route_info->routes[index].attrs.nh_type = type;
    switch (type) {
    case PDS_NH_TYPE_OVERLAY:
        num = (index % PDS_MAX_TEP);
        spec->route_info->routes[index].attrs.tep =
            int2pdsobjkey(base_tep_id + num);
        break;
    case PDS_NH_TYPE_OVERLAY_ECMP:
        num = (index % (PDS_MAX_NEXTHOP_GROUP-1));
        spec->route_info->routes[index].attrs.nh_group =
            int2pdsobjkey(base_id + num);
        break;
    case PDS_NH_TYPE_PEER_VPC:
        num = (index % PDS_MAX_VPC);
        spec->route_info->routes[index].attrs.vpc =
            int2pdsobjkey(base_id + num);
        break;
    case PDS_NH_TYPE_VNIC:
        num = (index % PDS_MAX_VNIC);
        spec->route_info->routes[index].attrs.vnic =
            int2pdsobjkey(base_id + num);
        break;
    case PDS_NH_TYPE_IP:
        num = (index % PDS_MAX_NEXTHOP);
        spec->route_info->routes[index].attrs.nh = int2pdsobjkey(base_id + num);
        break;
    case PDS_NH_TYPE_BLACKHOLE:
    default:
        break;
    }
}

bool
route_table_feeder::key_compare(const pds_obj_key_t *key) const {
    return *key == this->spec.key;
}

bool
route_table_feeder::spec_compare(const pds_route_table_spec_t *spec) const {
    if (spec->route_info == NULL)
        return false;
    if (spec->route_info->af != this->spec.route_info->af)
        return false;
    if (spec->route_info->num_routes != this->spec.route_info->num_routes)
        return false;
    return true;
}

bool
route_table_feeder::status_compare(
    const pds_route_table_status_t *status1,
    const pds_route_table_status_t *status2) const {
    return true;
}

// route-table CRUD helper functions
void
spec_fill (pds_nh_type_t type,pds_route_table_spec_t *spec,
          uint32_t index)
{
    uint32_t num = 0;
    uint32_t base_id = 1;
    uint32_t base_tep_id = 2;

    spec->route_info->routes[index].key = int2pdsobjkey(index + 1);
    spec->route_info->routes[index].attrs.nh_type = type;
    switch (type) {
    case PDS_NH_TYPE_OVERLAY:
        num = (index % PDS_MAX_TEP);
        spec->route_info->routes[index].attrs.tep =
            int2pdsobjkey(base_tep_id + num);
        break;
    case PDS_NH_TYPE_OVERLAY_ECMP:
        num = (index % (PDS_MAX_NEXTHOP_GROUP-1));
        spec->route_info->routes[index].attrs.nh_group =
            int2pdsobjkey(base_id + num);
        break;
    case PDS_NH_TYPE_PEER_VPC:
        num = (index % PDS_MAX_VPC);
        spec->route_info->routes[index].attrs.vpc =
            int2pdsobjkey(base_id + num);
        break;
    case PDS_NH_TYPE_VNIC:
        num = (index % PDS_MAX_VNIC);
        spec->route_info->routes[index].attrs.vnic =
            int2pdsobjkey(base_id + num);
        break;
    case PDS_NH_TYPE_IP:
        num = (index % PDS_MAX_NEXTHOP);
        spec->route_info->routes[index].attrs.nh =
            int2pdsobjkey(base_id + num);
        break;
    case PDS_NH_TYPE_BLACKHOLE:
    default:
        break;
    }
}

void
create_route_table_spec (std::string base_route_pfx_str,
                        uint8_t af, uint32_t num_routes,
                        uint32_t num_route_tables,
                        pds_route_table_spec_t *spec,
                        bool priority_en)
{
    ip_prefix_t route_pfx;
    ip_addr_t route_addr;
    uint32_t num_routes_per_type, route_index = 0;
    uint16_t tmp_bmap;
    uint32_t index = 0;

    test::extract_ip_pfx(base_route_pfx_str.c_str(), &route_pfx);
    spec->route_info =
        (route_info_t *)SDK_CALLOC(PDS_MEM_ALLOC_ID_ROUTE_TABLE,
                                   ROUTE_INFO_SIZE(num_routes));
    spec->route_info->af = af;
    spec->route_info->priority_en = priority_en;
    spec->route_info->num_routes = num_routes;
    num_routes_per_type = num_routes/k_num_nhtypes_supported;
    tmp_bmap = g_nhtypes_supported_bmap;
    while (tmp_bmap) {
        if (tmp_bmap & 0x1) {
            for (uint32_t j = 0; j < num_routes_per_type; j++) {
                spec->route_info->routes[route_index].attrs.prefix =
                    route_pfx;
                // updating route_priority based on route_index
                if(spec->route_info->priority_en){
                    spec->route_info->routes[route_index].attrs.priority =
                        route_index;
                }
                spec_fill((pds_nh_type_e)index, spec, route_index);
                ip_prefix_ip_next(&route_pfx, &route_addr);
                route_pfx.addr = route_addr;
                route_index++;
            }
        }
        index++;
        tmp_bmap >>= 1;
    }

    while (route_index < num_routes) {
        spec->route_info->routes[route_index].attrs.prefix = route_pfx;
        // updating route_priority based on route_index
        if(spec->route_info->priority_en){
            spec->route_info->routes[route_index].attrs.priority = route_index;
        }
        spec_fill(k_rt_def_nh_type, spec, route_index);
        ip_prefix_ip_next(&route_pfx, &route_addr);
        route_pfx.addr = route_addr;
        route_index++;
    }
}

void
route_table_create (route_table_feeder& feeder) {
    pds_batch_ctxt_t bctxt = batch_start();

    SDK_ASSERT_RETURN_VOID(
        (SDK_RET_OK == many_create<route_table_feeder>(bctxt, feeder)));
    batch_commit(bctxt);
}

void
route_table_read (route_table_feeder& feeder, sdk_ret_t exp_result) {
    SDK_ASSERT_RETURN_VOID(
        (SDK_RET_OK == many_read<route_table_feeder>(feeder, exp_result)));
}

static void
route_table_attr_update (route_table_feeder& feeder,
                         pds_route_table_spec_t *spec,
                         uint64_t chg_bmap) {
    if (bit_isset(chg_bmap, ROUTE_TABLE_ATTR_AF)) {
        if(spec->route_info) {
            feeder.spec.route_info->af = spec->route_info->af;
        }
    }
    if (bit_isset(chg_bmap, ROUTE_TABLE_ATTR_ROUTES)) {
        if(spec->route_info) {
            feeder.spec.route_info->num_routes = spec->route_info->num_routes;
        }
    }
    if (bit_isset(chg_bmap, ROUTE_TABLE_ATTR_PRIORITY_EN)) {
        if(spec->route_info) {
            feeder.spec.route_info->priority_en = spec->route_info->priority_en;
        }
    }

}

void
route_table_update (route_table_feeder& feeder, pds_route_table_spec_t *spec,
                    uint64_t chg_bmap, sdk_ret_t exp_result) {
	pds_batch_ctxt_t bctxt = batch_start();
	route_table_attr_update(feeder, spec, chg_bmap);
	SDK_ASSERT_RETURN_VOID(
			(SDK_RET_OK == many_update<route_table_feeder>(bctxt, feeder)));
	//if expected result is err, batch commit should fail
	if (exp_result == SDK_RET_ERR) {
        batch_commit_fail(bctxt);
    } else {
        batch_commit(bctxt);
    }
}

void
route_table_delete (route_table_feeder& feeder) {
    pds_batch_ctxt_t bctxt = batch_start();
    SDK_ASSERT_RETURN_VOID(
        (SDK_RET_OK == many_delete<route_table_feeder>(bctxt, feeder)));
    batch_commit(bctxt);
}

//----------------------------------------------------------------------------
// Misc routines
//----------------------------------------------------------------------------

// do not modify these sample values as rest of system is sync with these
static route_table_feeder k_route_table_feeder;

void sample_route_table_setup(
    pds_batch_ctxt_t bctxt, const string base_route_pfx, uint8_t af,
    uint32_t num_routes, uint32_t num_route_tables, uint32_t id) {
    // setup and teardown parameters should be in sync
    k_route_table_feeder.init(base_route_pfx, af, num_routes,
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
