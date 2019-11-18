//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
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
#include "nic/apollo/framework/api_params.hpp"
#include "nic/apollo/api/route.hpp"
#include "nic/apollo/api/tep.hpp"
#include "nic/apollo/api/nexthop_group.hpp"
#include "nic/apollo/api/impl/apulu/pds_impl_state.hpp"
#include "nic/apollo/api/impl/lpm/lpm.hpp"
#include "nic/apollo/api/impl/apulu/apulu_impl.hpp"
#include "nic/apollo/api/impl/apulu/route_impl.hpp"
#include "nic/apollo/api/impl/apulu/tep_impl.hpp"
#include "nic/apollo/api/impl/apulu/vnic_impl.hpp"
#include "nic/apollo/api/impl/apulu/nexthop_group_impl.hpp"

#define PDS_IMPL_ROUTE_NH_ID(type_, val_)                        \
            (((type_) << ROUTE_RESULT_NHTYPE_SHIFT) | (val_))

#define PDS_IMPL_SRC_NAT_TYPE_SHIFT        4

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

// NOTE: reserve_resources() logic is same for both API_OP_CREATE and
//       API_OP_UPDATE as update doesn't reuse any of the existing resources
//       for this object
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
    pds_vpc_key_t             vpc_key;
    route_table_t             *rtable;
    nexthop_group             *nh_group;
    tep_entry                 *tep;
    vpc_entry                 *vpc;
    vnic_entry                *vnic;

    spec = &obj_ctxt->api_params->route_table_spec;
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
    rtable->default_nhid = PDS_IMPL_SYSTEM_DROP_NEXTHOP_HW_ID;
    if (rtable->af == IP_AF_IPV4) {
        rtable->max_routes = route_table_impl_db()->v4_max_routes();
    } else {
        rtable->max_routes = route_table_impl_db()->v6_max_routes();
    }
    rtable->num_routes = spec->num_routes;
    for (uint32_t i = 0; i < rtable->num_routes; i++) {
        rtable->routes[i].prefix = spec->routes[i].prefix;
        if (!rtable->routes[i].prio) {
            rtable->routes[i].prio = 128 - spec->routes[i].prefix.len;
        }
        switch (spec->routes[i].nh_type) {
        case PDS_NH_TYPE_BLACKHOLE:
            rtable->routes[i].nhid = PDS_IMPL_SYSTEM_DROP_NEXTHOP_HW_ID;
            PDS_TRACE_DEBUG("Processing route table %u, route %s -> blackhole "
                            "nh id %u, ", spec->key.id,
                            ippfx2str(&rtable->routes[i].prefix),
                            rtable->routes[i].nhid);
            break;

        case PDS_NH_TYPE_OVERLAY:
            // non vpc peering case
            tep = tep_db()->find(&spec->routes[i].tep);
            if (tep == NULL) {
                PDS_TRACE_ERR("TEP %u not found while processing route %s in "
                              "route table %u", spec->routes[i].tep.id,
                              ippfx2str(&spec->routes[i].prefix), spec->key.id);
                ret = SDK_RET_INVALID_ARG;
                goto cleanup;
            }
            rtable->routes[i].nhid =
                PDS_IMPL_ROUTE_NH_ID(NEXTHOP_TYPE_TUNNEL,
                                     ((tep_impl *)(tep->impl()))->hw_id());
            PDS_TRACE_DEBUG("Processing route table %u, route %s -> TEP %s, "
                            "nh id 0x%x", spec->key.id,
                            ippfx2str(&rtable->routes[i].prefix),
                            tep->key2str().c_str(), rtable->routes[i].nhid);
            break;

        case PDS_NH_TYPE_OVERLAY_ECMP:
            nh_group = nexthop_group_db()->find(&spec->routes[i].nh_group);
            if (nh_group == NULL) {
                PDS_TRACE_ERR("nexthop group %u not found while processing "
                              "route %s in route table %u",
                              spec->routes[i].nh_group.id,
                              ippfx2str(&rtable->routes[i].prefix),
                              spec->key.id);
                ret = SDK_RET_INVALID_ARG;
                goto cleanup;
            }
            rtable->routes[i].nhid =
                PDS_IMPL_ROUTE_NH_ID(NEXTHOP_TYPE_ECMP,
                    ((nexthop_group_impl *)nh_group->impl())->hw_id());
            break;

        case PDS_NH_TYPE_PEER_VPC:
            vpc = vpc_db()->find(&spec->routes[i].vpc);
            if (vpc == NULL) {
                PDS_TRACE_ERR("vpc %u not found while processing route %s in "
                              "route table %u", spec->routes[i].vpc.id,
                              ippfx2str(&spec->routes[i].prefix),
                              spec->key.id);
                ret = SDK_RET_INVALID_ARG;
                goto cleanup;
            }
            rtable->routes[i].nhid = PDS_IMPL_ROUTE_NH_ID(NEXTHOP_TYPE_VPC,
                                                          vpc->hw_id());
            PDS_TRACE_DEBUG("Processing route table %u, route %s -> vpc hw "
                            "id 0x%x, ", spec->key.id,
                            ippfx2str(&rtable->routes[i].prefix),
                            rtable->routes[i].nhid);
            break;

        case PDS_NH_TYPE_NAT:
            rtable->routes[i].nhid =
                PDS_IMPL_ROUTE_NH_ID(NEXTHOP_TYPE_NAT,
                    ((spec->routes[i].nat.src_nat << PDS_IMPL_SRC_NAT_TYPE_SHIFT) |
                     spec->routes[i].nat.dst_nat));
            PDS_TRACE_DEBUG("Processing route table %u, route %s -> NAT, "
                            "id 0x%x, ", spec->key.id,
                            ippfx2str(&rtable->routes[i].prefix),
                            rtable->routes[i].nhid);
            break;

        case PDS_NH_TYPE_VNIC:
            vnic = vnic_db()->find(&spec->routes[i].vnic);
            if (vnic == NULL) {
                 PDS_TRACE_ERR("vnic %u not found while processing route %s in "
                               "route table %u", spec->routes[i].vnic.id,
                               ippfx2str(&spec->routes[i].prefix),
                               spec->key.id);
                 break;
            }
            rtable->routes[i].nhid =
                PDS_IMPL_ROUTE_NH_ID(NEXTHOP_TYPE_NEXTHOP,
                                     ((vnic_impl *)vnic->impl())->nh_idx());
            break;

        default:
            PDS_TRACE_ERR("Unknown nh type %u while processing route %s in "
                          "route table %u", spec->routes[i].nh_type,
                          ippfx2str(&spec->routes[i].prefix), spec->key.id);
            ret = SDK_RET_INVALID_ARG;
            goto cleanup;
            break;
        }
    }
    ret = lpm_tree_create(rtable,
                          (spec->af == IP_AF_IPV4) ? ITREE_TYPE_IPV4 :
                                                     ITREE_TYPE_IPV6,
                          lpm_root_addr_,
                          (spec->af == IP_AF_IPV4) ?
                          route_table_impl_db()->v4_table_size() :
                          route_table_impl_db()->v6_table_size());
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to build LPM route table, err %u", ret);
    }

cleanup:
    SDK_FREE(PDS_MEM_ALLOC_ID_ROUTE_TABLE, rtable);
    return ret;
}

sdk_ret_t
route_table_impl::activate_hw(api_base *api_obj, pds_epoch_t epoch,
                              api_op_t api_op, obj_ctxt_t *obj_ctxt)
{
    switch (api_op) {
    case API_OP_CREATE:
    case API_OP_DELETE:
        // for route table create, there is no stage 0 programming
        // for route table delete, since all objects (e.g., subnets)
        // referring to this route table are already modified to point
        // to other routing table(s) or deleted (agent is expected to
        // ensure this), there is no need to program any tables during
        // activation stage
    case API_OP_UPDATE:
        // affected objects like affected vpc, subnet and vnic objects
        // are in the dependent object list by now and will be reprogrammed
    default:
        break;
    }
    return SDK_RET_OK;
}

void
route_table_impl::fill_status_(pds_route_table_status_t *status) {
    status->route_table_base_addr = lpm_root_addr_;
}

sdk_ret_t
route_table_impl::read_hw(api_base *api_obj, obj_key_t *key, obj_info_t *info) {
    pds_route_table_info_t *route_table_info = (pds_route_table_info_t *)info;

    fill_status_(&route_table_info->status);

    return SDK_RET_OK;
}

/// \@}

}    // namespace impl
}    // namespace api
