//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// datapath implementation of route table
///
//----------------------------------------------------------------------------

#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/api/route.hpp"
#include "nic/apollo/api/impl/route_impl.hpp"
#include "nic/apollo/api/impl/pds_impl_state.hpp"
#include "nic/apollo/api/tep.hpp"
#include "nic/apollo/api/impl/tep_impl.hpp"
#include "nic/apollo/lpm/lpm.hpp"

namespace api {
namespace impl {

/// \defgroup PDS_ROUTE_TABLE_IMPL - route table datapath implementation
/// \ingroup PDS_ROUTE
/// \@{

route_table_impl *
route_table_impl::factory(pds_route_table_spec_t *spec) {
    route_table_impl    *impl;

    // TODO: move to slab later
    impl = (route_table_impl *)SDK_CALLOC(SDK_MEM_ALLOC_PDS_ROUTE_TABLE_IMPL,
                                          sizeof(route_table_impl));
    new (impl) route_table_impl();
    return impl;
}

void
route_table_impl::destroy(route_table_impl *impl) {
    impl->~route_table_impl();
    SDK_FREE(SDK_MEM_ALLOC_PDS_ROUTE_TABLE_IMPL, impl);
}

sdk_ret_t
route_table_impl::reserve_resources(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    uint32_t                  lpm_block_id;
    pds_route_table_spec_t    *spec;

    spec = &obj_ctxt->api_params->route_table_spec;
    // allocate free lpm slab for this route table
    if (spec->af == IP_AF_IPV4) {
        if (route_table_impl_db()->v4_idxr()->alloc(&lpm_block_id) !=
                sdk::lib::indexer::SUCCESS) {
            return sdk::SDK_RET_NO_RESOURCE;
        }
        lpm_root_addr_ =
            route_table_impl_db()->v4_region_addr() +
                (route_table_impl_db()->v4_table_size() * lpm_block_id);
    } else if (spec->af == IP_AF_IPV6) {
        if (route_table_impl_db()->v6_idxr()->alloc(&lpm_block_id) !=
                sdk::lib::indexer::SUCCESS) {
            return sdk::SDK_RET_NO_RESOURCE;
        }
        lpm_root_addr_ =
            route_table_impl_db()->v6_region_addr() +
                (route_table_impl_db()->v6_table_size() * lpm_block_id);
    }
    return SDK_RET_OK;
}

sdk_ret_t
route_table_impl::release_resources(api_base *api_obj) {
    uint32_t       lpm_block_id;
    route_table    *rtable = (route_table *)api_obj;

    // just free the lpm block we are using
    if (lpm_root_addr_ != 0xFFFFFFFFFFFFFFFFUL) {
        if (rtable->af() == IP_AF_IPV4) {
            lpm_block_id =
                (lpm_root_addr_ - route_table_impl_db()->v4_region_addr())/route_table_impl_db()->v4_table_size();
            route_table_impl_db()->v4_idxr()->free(lpm_block_id);
        } else {
            lpm_block_id =
                (lpm_root_addr_ - route_table_impl_db()->v6_region_addr())/route_table_impl_db()->v6_table_size();
            route_table_impl_db()->v6_idxr()->free(lpm_block_id);
        }
    }
    return SDK_RET_OK;
}

sdk_ret_t
route_table_impl::nuke_resources(api_base *api_obj) {
    // just free the lpm block we are using
    return this->release_resources(api_obj);
}

sdk_ret_t
route_table_impl::program_hw(api_base *api_obj, obj_ctxt_t *obj_ctxt) {
    sdk_ret_t                 ret;
    pds_route_table_spec_t    *spec;
    route_table_t             *rtable;
    pds_tep_key_t             tep_key;
    api::tep_entry            *tep;

    spec = &obj_ctxt->api_params->route_table_spec;
    if (spec->num_routes == 0) {
        PDS_TRACE_WARN("Route table %u doesn't have any routes", spec->key.id);
        return SDK_RET_OK;
    }

    // allocate memory for the library to build route table
    rtable =
        (route_table_t *)
            SDK_MALLOC(PDS_MEM_ALLOC_ID_ROUTE_TABLE,
                       sizeof(route_table_t) +
                           (spec->num_routes * sizeof(route_t)));
    if (rtable == NULL) {
        return sdk::SDK_RET_OOM;
    }
    rtable->af = spec->af;
    rtable->default_nhid = PDS_DROP_NEXTHOP_HW_ID;
    if (rtable->af == IP_AF_IPV4) {
        rtable->max_routes = route_table_impl_db()->v4_max_routes();
    } else {
        rtable->max_routes = route_table_impl_db()->v6_max_routes();
    }
    rtable->num_routes = spec->num_routes;
    for (uint32_t i = 0; i < rtable->num_routes; i++) {
        rtable->routes[i].prefix = spec->routes[i].prefix;
        tep_key.ip_addr = spec->routes[i].nh_ip.addr.v4_addr;
        tep = tep_db()->find(&tep_key);
        SDK_ASSERT(tep != NULL);
        rtable->routes[i].nhid = ((tep_impl *)(tep->impl()))->nh_id();
        PDS_TRACE_DEBUG("Processing route table %u, route %s -> nh %u, TEP %s",
                        spec->key.id, ippfx2str(&rtable->routes[i].prefix),
                        rtable->routes[i].nhid,
                        ipv4addr2str(tep->ip()));
    }
    ret = lpm_tree_create(rtable, lpm_root_addr_,
                          (spec->af == IP_AF_IPV4) ?
                          route_table_impl_db()->v4_table_size() :
                          route_table_impl_db()->v6_table_size());
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to build LPM route table, err : %u", ret);
    }
    SDK_FREE(PDS_MEM_ALLOC_ID_ROUTE_TABLE, rtable);
    return ret;
}

sdk_ret_t
route_table_impl::activate_hw(api_base *api_obj, pds_epoch_t epoch,
                              api_op_t api_op, obj_ctxt_t *obj_ctxt)
{
    switch (api_op) {
    case api::API_OP_CREATE:
    case api::API_OP_DELETE:
        // for route table create, there is no stage 0 programming
        // for route table delete, since all objects (e.g., subnets)
        // referring to this route table are already modified to point
        // to other routing table(s) or deleted (agent is expected to
        // ensure this), there is no need to program any tables during
        // activation stage
        break;

    case api::API_OP_UPDATE:
         // TODO:
         // need to walk all vnics AND subnets to see which of them are using
         // this routing table and then walk all the vnics that are part of the
         // vcns and subnets and write new epoch data
        return SDK_RET_ERR;
        break;

    default:
        break;
    }
    return SDK_RET_OK;
}

/// \@}    // end of PDS_ROUTE_TABLE_IMPL

}    // namespace impl
}    // namespace api
