//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// datapath implementation of route table
///
//----------------------------------------------------------------------------

#include "nic/sdk/lib/utils/utils.hpp"
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
#include "nic/apollo/api/impl/apulu/vpc_impl.hpp"
#include "nic/apollo/api/impl/apulu/nexthop_group_impl.hpp"

#define PDS_IMPL_NH_VAL_SET_METER_EN(nh_val_)                                \
            ((nh_val_) |= ROUTE_RESULT_METER_EN_MASK)
#define PDS_IMPL_NH_VAL_SET_SNAT_TYPE(nh_val_, snat_type_)                   \
            ((nh_val_) |= ((snat_type_ << ROUTE_RESULT_SNAT_TYPE_SHIFT)      \
                                        & ROUTE_RESULT_SNAT_TYPE_MASK))
#define PDS_IMPL_NH_VAL_SET_DNAT_INFO(nh_val_, dnat_en_, dnat_idx_)          \
{                                                                            \
    if ((dnat_en_)) {                                                        \
        (nh_val_) |= (ROUTE_RESULT_DNAT_EN_MASK |                            \
                      (((dnat_idx_) << ROUTE_RESULT_DNAT_IDX_SHIFT)          \
                       & ROUTE_RESULT_DNAT_IDX_MASK));                       \
    } else {                                                                 \
        (nh_val_) &= ~(ROUTE_RESULT_DNAT_EN_MASK|ROUTE_RESULT_DNAT_IDX_MASK);\
    }                                                                        \
}
#define PDS_IMPL_NH_VAL_SET_NH_INFO(nh_val_, type_, nh_id_)                  \
            (nh_val_) |= (((type_ << ROUTE_RESULT_NHTYPE_SHIFT)              \
                                   & ROUTE_RESULT_NHTYPE_MASK) |             \
                          ((nh_id_ << ROUTE_RESULT_NEXTHOP_SHIFT)            \
                                    & ROUTE_RESULT_NEXTHOP_MASK));

namespace api {
namespace impl {

/// \defgroup PDS_ROUTE_TABLE_IMPL - route table datapath implementation
/// \ingroup PDS_ROUTE
/// \@{

route_table_impl *
route_table_impl::factory(pds_route_table_spec_t *spec) {
    route_table_impl *impl;

    if (spec->route_info->af == IP_AF_IPV4) {
        if (spec->route_info->num_routes >
            route_table_impl_db()->v4_table_size()) {
            PDS_TRACE_ERR("No. of IPv4 routes %u in the route table %s "
                          "exceeded max supported scale %u",
                          spec->route_info->num_routes, spec->key.str(),
                          route_table_impl_db()->v4_table_size());
            return NULL;
        }
    } else {
        if (spec->route_info->num_routes >
            route_table_impl_db()->v6_table_size()) {
            PDS_TRACE_ERR("No. of IPv6 routes %u in the route table %s "
                          "exceeded max supported scale %u",
                          spec->route_info->num_routes, spec->key.str(),
                          route_table_impl_db()->v6_table_size());
            return NULL;
        }
    }
    impl = route_table_impl_db()->alloc();
    new (impl) route_table_impl();
    return impl;
}

void
route_table_impl::destroy(route_table_impl *impl) {
    impl->~route_table_impl();
    route_table_impl_db()->free(impl);
}

impl_base *
route_table_impl::clone(void) {
    route_table_impl*cloned_impl;

    cloned_impl = route_table_impl_db()->alloc();
    new (cloned_impl) route_table_impl();
    // deep copy is not needed as we don't store pointers
    *cloned_impl = *this;
    return cloned_impl;
}

sdk_ret_t
route_table_impl::free(route_table_impl *impl) {
    destroy(impl);
    return SDK_RET_OK;
}

sdk_ret_t
route_table_impl::update_route_table_spec_(pds_route_table_spec_t *spec,
                                           api_obj_ctxt_t *obj_ctxt) {
    uint32_t i;
    bool found;
    pds_obj_key_t key;
    api_obj_ctxt_t *octxt;

    for (auto it = obj_ctxt->clist.begin(); it != obj_ctxt->clist.end(); it++) {
        octxt = *it;
        if (octxt->api_op == API_OP_CREATE) {
            // add the route to the end of the table
            spec->route_info->routes[spec->route_info->num_routes] =
                octxt->api_params->route_spec.route;
            spec->route_info->num_routes++;
        } else {
            // either DEL or UPD operation
            if (octxt->api_op == API_OP_DELETE) {
                key = octxt->api_params->key;
            } else {
                // update case
                key = octxt->api_params->route_spec.key;
            }
            // search and find the object to delete or modify
            found = false;
            for (i = 0; i < spec->route_info->num_routes; i++) {
                if (key == spec->route_info->routes[i].key) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                PDS_TRACE_ERR("route %s not found in route table %s to "
                              "perform api op %u",
                              key.str(), spec->key.str(), octxt->api_op);
                return SDK_RET_INVALID_ARG;
            }
            if (octxt->api_op == API_OP_DELETE) {
                spec->route_info->routes[i] =
                    spec->route_info->routes[spec->route_info->num_routes - 1];
                spec->route_info->num_routes--;
            } else {
                // update case
                spec->route_info->routes[i] =
                    octxt->api_params->route_spec.route;
            }
        }
    }
    return SDK_RET_OK;
}

sdk_ret_t
route_table_impl::compute_updated_spec_(route_table *new_rtable,
                                        route_table *orig_rtable,
                                        api_obj_ctxt_t *obj_ctxt) {
    sdk_ret_t ret;
    uint32_t num_routes;
    pds_route_table_spec_t *spec;
    route_info_t *new_route_info;

    if (obj_ctxt->clist.size() == 0) {
        SDK_ASSERT((obj_ctxt->upd_bmap & (PDS_ROUTE_TABLE_UPD_ROUTE_ADD |
                                          PDS_ROUTE_TABLE_UPD_ROUTE_DEL |
                                          PDS_ROUTE_TABLE_UPD_ROUTE_UPD)) == 0);
        PDS_TRACE_DEBUG("Processing route table %s create/update with no "
                        "individual route updates in this batch",
                        new_rtable->key2str().c_str());
        // in this case, spec can be used as-is from the object context
        return SDK_RET_OK;
    }

    // we have two cases to handle here:
    // 1. route table object itself is being added/updated (some other
    //    attributes modifications and/or with new set of routes combined with
    //    individual route add/del/upd operations, all in this batch
    //    and in this case, we need to create a new spec that "combines"
    //    incoming spec with individual route add/del/upd operations
    // 2. route table object modification is solely because of individual
    //    route add/del/updates in this batch and in this case we need to create
    //    a new spec that "combines" previously persisted set of routes with
    //    individual route add/del/upd operations
    // in both cases, we need to form new spec
    spec = &obj_ctxt->api_params->route_table_spec;
    if ((obj_ctxt->api_op == API_OP_CREATE) ||
        (obj_ctxt->upd_bmap & ~(PDS_ROUTE_TABLE_UPD_ROUTE_ADD |
                                PDS_ROUTE_TABLE_UPD_ROUTE_DEL |
                                PDS_ROUTE_TABLE_UPD_ROUTE_UPD))) {
        // case 1 : both container and contained objects are being modified
        //          (ADD/DEL/UPD), in this we can fully ignore the set of routes
        //          that are persisted in kvstore
        // number of new routes in the worst case will be total of what we have
        // currently plus size of clist (assuming all contained objs are being
        // added)
        // TODO:
        // keep track of this counter in obj_ctxt itself in API engine so we can
        // catch errors where total capacity exceeds max. supported without any
        // extra processing
        // allocate new route info so it has enough room to handle other route
        // API operations
        num_routes = new_rtable->num_routes() + obj_ctxt->clist.size();
        new_route_info =
            (route_info_t *)SDK_MALLOC(PDS_MEM_ALLOC_ID_ROUTE_TABLE,
                                       ROUTE_INFO_SIZE(num_routes));
        if (!new_route_info) {
            PDS_TRACE_ERR("Failed to allocate memory for %u routes for "
                          "route table %s update processing", num_routes,
                          spec->key.str());
            return SDK_RET_OOM;
        }
        // copy all routes from the spec into this new route info
        memcpy(new_route_info, spec->route_info,
               ROUTE_INFO_SIZE(new_rtable->num_routes()));
        // free the old route info and replace the spec with new one
        if (spec->route_info) {
            SDK_FREE(PDS_MEM_ALLOC_ID_ROUTE_TABLE, spec->route_info);
            spec->route_info = NULL;
        }
        spec->route_info = new_route_info;
    } else {
        // case 2 : only contained objects are being modified (ADD/DEL/UPD),
        //          form new spec that consists of current set of (preserved)
        //          routes and individual route updates
        // number of new routes in the worst case will be total of what we have
        // currently plus size of clist (assuming all contained objs are being
        // added)
        // TODO:
        // keep track of this counter in obj_ctxt itself in API engine so we can
        // catch errors where total capacity exceeds max. supported without any
        // extra processing
        // in this case, this object has been promoted from aol to dol list
        // but with no spec at all
        spec->key = new_rtable->key();
        num_routes = orig_rtable->num_routes() + obj_ctxt->clist.size();
        SDK_ASSERT(spec->route_info == NULL);
        spec->route_info =
            (route_info_t *)SDK_MALLOC(PDS_MEM_ALLOC_ID_ROUTE_TABLE,
                                       ROUTE_INFO_SIZE(num_routes));
        if (!spec->route_info) {
            PDS_TRACE_ERR("Failed to allocate memory for %u routes for "
                          "route table %s update processing", num_routes,
                          spec->key.str());
            return SDK_RET_OOM;
        }
        spec->route_info->num_routes = orig_rtable->num_routes();
        ret = route_table_db()->retrieve_routes(&spec->key, spec->route_info);
        if (ret != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to retrieve routes from kvstore for route "
                          "table %s, err %u", spec->key.str(), ret);
            goto error;
        }
    }

    // compute the udpated spec now
    ret = update_route_table_spec_(spec, obj_ctxt);
    if (ret != SDK_RET_OK) {
        return ret;
    }
    PDS_TRACE_DEBUG("Route table %s route count changed from %u to %u",
                    spec->key.str(), orig_rtable->num_routes(),
                    spec->route_info->num_routes);
    return SDK_RET_OK;

error:

    if (spec->route_info) {
        SDK_FREE(PDS_MEM_ALLOC_ID_ROUTE_TABLE, spec->route_info);
        spec->route_info = NULL;
    }
    return ret;
}

// NOTE: reserve_resources() logic is same for both API_OP_CREATE and
//       API_OP_UPDATE as update doesn't reuse any of the existing resources
//       for this object
sdk_ret_t
route_table_impl::reserve_resources(api_base *api_obj, api_base *orig_obj,
                                    api_obj_ctxt_t *obj_ctxt) {
    sdk_ret_t ret;
    uint32_t lpm_block_id;
    pds_route_t *route_spec;
    pds_route_table_spec_t *spec;

    if ((obj_ctxt->api_op == API_OP_UPDATE) || obj_ctxt->clist.size()) {
        // if this is CREATE batched with individual route add/del/update
        // operations or update of any kind, we may have to update the spec
        ret = compute_updated_spec_((route_table *)api_obj,
                                    (route_table *)orig_obj, obj_ctxt);
        if (ret != SDK_RET_OK) {
            return ret;
        }
    }

    spec = &obj_ctxt->api_params->route_table_spec;
    // record the fact that resource reservation was attempted
    // NOTE: even if we partially acquire resources and fail eventually,
    //       this will ensure that proper release of resources will happen
    api_obj->set_rsvd_rsc();

    for (uint32_t i = 0; i < spec->route_info->num_routes; i++) {
        route_spec = &spec->route_info->routes[i];
        if ((route_spec->nat.dst_nat_ip.af == IP_AF_IPV4) ||
            (route_spec->nat.dst_nat_ip.af == IP_AF_IPV6)) {
            num_dnat_entries_++;
        }
    }
    if (num_dnat_entries_) {
        ret = apulu_impl_db()->dnat_idxr()->alloc_block(&dnat_base_idx_,
                                                        num_dnat_entries_,
                                                        false);
        if (ret != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to reserve %u entries in DNAT table for "
                          "route table %s, err %u", spec->key.str(), ret);
            return ret;
        }
    }

    // allocate free lpm slab for this route table
    if (spec->route_info->af == IP_AF_IPV4) {
        if (route_table_impl_db()->v4_idxr()->alloc(&lpm_block_id) !=
                sdk::lib::indexer::SUCCESS) {
            return sdk::SDK_RET_NO_RESOURCE;
        }
        lpm_root_addr_ =
            route_table_impl_db()->v4_region_addr() +
                (route_table_impl_db()->v4_table_size() * lpm_block_id);
    } else if (spec->route_info->af == IP_AF_IPV6) {
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

    if (dnat_base_idx_ != 0xFFFFFFFF) {
        apulu_impl_db()->dnat_idxr()->free(dnat_base_idx_, num_dnat_entries_);
    }
    return SDK_RET_OK;
}

sdk_ret_t
route_table_impl::nuke_resources(api_base *api_obj) {
    // just free the lpm block we are using
    return this->release_resources(api_obj);
}

#define dnat_info    action_u.dnat_dnat
sdk_ret_t
route_table_impl::program_route_table_(pds_route_table_spec_t *spec) {
    sdk_ret_t              ret;
    tep_entry              *tep;
    vpc_entry              *vpc;
    vnic_entry             *vnic;
    uint32_t               nh_val;
    pds_obj_key_t          vpc_key;
    route_table_t          *rtable;
    p4pd_error_t           p4pd_ret;
    nexthop_group          *nh_group;
    pds_route_t            *route_spec;
    dnat_actiondata_t      dnat_data = {0};
    nat2_actiondata_t      nat2_data = {0};

    // allocate memory for the library to build route table
    rtable =
        (route_table_t *)
            SDK_MALLOC(PDS_MEM_ALLOC_ID_ROUTE_TABLE,
                       sizeof(route_table_t) +
                           (spec->route_info->num_routes * sizeof(route_t)));
    if (rtable == NULL) {
        return sdk::SDK_RET_OOM;
    }
    rtable->af = spec->route_info->af;
    rtable->pbr_enabled = spec->route_info->priority_en;
    rtable->default_nhid = 0;
    PDS_IMPL_NH_VAL_SET_NH_INFO(rtable->default_nhid, NEXTHOP_TYPE_NEXTHOP,
                                PDS_IMPL_SYSTEM_DROP_NEXTHOP_HW_ID);
    if (rtable->af == IP_AF_IPV4) {
        rtable->max_routes = route_table_impl_db()->v4_max_routes();
    } else {
        rtable->max_routes = route_table_impl_db()->v6_max_routes();
    }
    rtable->num_routes = spec->route_info->num_routes;
    for (uint32_t i = 0, dnat_idx = 0; i < rtable->num_routes; i++) {
        nh_val = 0;
        route_spec = &spec->route_info->routes[i];
        rtable->routes[i].prefix = route_spec->prefix;
        // if user specified priority explicitly, use it or else use prefix
        // length to compute the priority (to provide longest prefix matching
        // semantics)
        if (rtable->pbr_enabled) {
            rtable->routes[i].prio = route_spec->prio;
        } else {
            rtable->routes[i].prio = 128 - route_spec->prefix.len;
        }
        // if metering is enabled on this route, set the M bit in the
        // corresponding nexthop
        if (route_spec->meter) {
            PDS_IMPL_NH_VAL_SET_METER_EN(nh_val);
        }
        // set the SNAT type
        PDS_IMPL_NH_VAL_SET_SNAT_TYPE(nh_val, route_spec->nat.src_nat_type);
        // if DNAT is required for traffic hitting this route, setup the DNA
        // table entry
        if ((route_spec->nat.dst_nat_ip.af == IP_AF_IPV4) ||
            (route_spec->nat.dst_nat_ip.af == IP_AF_IPV6)) {
            // DNAT is enabled on this route
            PDS_IMPL_NH_VAL_SET_DNAT_INFO(nh_val, TRUE,
                                          dnat_base_idx_ + dnat_idx);

            rtable->routes[i].nhid = nh_val;
            PDS_TRACE_DEBUG("Processing route table %s, route %s prio: %u -> DNAT %s, "
                            "nh id 0x%x", spec->key.str(),
                            ippfx2str(&rtable->routes[i].prefix),
                            rtable->routes[i].prio,
                            ipaddr2str(&route_spec->nat.dst_nat_ip),
                            nh_val);

            // write to DNAT table at this index
            dnat_data.action_id = DNAT_DNAT_ID;
            dnat_data.dnat_info.route_table_hw_id = 0;
            if (route_spec->nat.dst_nat_ip.af == IP_AF_IPV4) {
                memcpy(dnat_data.dnat_info.dnat_address,
                       &route_spec->nat.dst_nat_ip.addr.v4_addr, IP4_ADDR8_LEN);
            } else {
                sdk::lib::memrev(dnat_data.dnat_info.dnat_address,
                                 route_spec->nat.dst_nat_ip.addr.v6_addr.addr8,
                                 IP6_ADDR8_LEN);
            }
            p4pd_ret = p4pd_global_entry_write(P4_P4PLUS_TXDMA_TBL_ID_DNAT,
                                               dnat_base_idx_ + dnat_idx,
                                               NULL, NULL, &dnat_data);
            if (p4pd_ret != P4PD_SUCCESS) {
                PDS_TRACE_ERR("Failed to program DNAT table at %u for route "
                              "%s in route table %s", dnat_base_idx_ + dnat_idx,
                              ippfx2str(&route_spec->prefix), spec->key.str());
                ret = SDK_RET_HW_PROGRAM_ERR;
                goto cleanup;
            }

            PDS_TRACE_DEBUG("Programmed DNAT table at %u for IP %s",
                            dnat_base_idx_ + dnat_idx,
                            ipaddr2str(&route_spec->nat.dst_nat_ip));

            // write to NAT2 table at this index
            nat2_data.action_id = NAT2_NAT2_REWRITE_ID;
            if (route_spec->nat.dst_nat_ip.af == IP_AF_IPV4) {
                memcpy(nat2_data.action_u.nat2_nat2_rewrite.ip,
                       &route_spec->nat.dst_nat_ip.addr.v4_addr, IP4_ADDR8_LEN);
            } else {
                sdk::lib::memrev(nat2_data.action_u.nat2_nat2_rewrite.ip,
                                 route_spec->nat.dst_nat_ip.addr.v6_addr.addr8,
                                 IP6_ADDR8_LEN);
            }
            p4pd_ret = p4pd_global_entry_write(P4TBL_ID_NAT2,
                                               (dnat_base_idx_ + dnat_idx) * 2,
                                               NULL, NULL, &nat2_data);
            if (p4pd_ret != P4PD_SUCCESS) {
                PDS_TRACE_ERR("Failed to program NAT2 table at %u for route "
                              "%s in route table %s", (dnat_base_idx_ + dnat_idx) * 2,
                              ippfx2str(&route_spec->prefix), spec->key.str());
                ret = SDK_RET_HW_PROGRAM_ERR;
                goto cleanup;
            }

            PDS_TRACE_DEBUG("Programmed NAT2 table at %u for IP %s",
                            (dnat_base_idx_ + dnat_idx) * 2,
                            ipaddr2str(&route_spec->nat.dst_nat_ip));

            // program reverse entry in nat2 table
            if (route_spec->prefix.addr.af == IP_AF_IPV4) {
                memcpy(nat2_data.action_u.nat2_nat2_rewrite.ip,
                       &route_spec->prefix.addr.addr.v4_addr, IP4_ADDR8_LEN);
            } else {
                sdk::lib::memrev(nat2_data.action_u.nat2_nat2_rewrite.ip,
                                 route_spec->prefix.addr.addr.v6_addr.addr8,
                                 IP6_ADDR8_LEN);
            }
            p4pd_ret = p4pd_global_entry_write(P4TBL_ID_NAT2,
                                               (dnat_base_idx_ + dnat_idx) * 2 + 1,
                                               NULL, NULL, &nat2_data);
            if (p4pd_ret != P4PD_SUCCESS) {
                PDS_TRACE_ERR("Failed to program NAT2 table at %u for route "
                              "%s in route table %s", (dnat_base_idx_ + dnat_idx) * 2 + 1,
                              ippfx2str(&route_spec->prefix), spec->key.str());
                ret = SDK_RET_HW_PROGRAM_ERR;
                goto cleanup;
            }

            PDS_TRACE_DEBUG("Programmed NAT2 table at %u for IP %s",
                            (dnat_base_idx_ + dnat_idx) * 2 + 1,
                            ipaddr2str(&route_spec->prefix.addr));
            dnat_idx++;
            continue;
        } else {
            PDS_IMPL_NH_VAL_SET_DNAT_INFO(nh_val, FALSE, 0);
        }

        switch (route_spec->nh_type) {
        case PDS_NH_TYPE_BLACKHOLE:
            PDS_IMPL_NH_VAL_SET_NH_INFO(nh_val, NEXTHOP_TYPE_NEXTHOP,
                                        PDS_IMPL_SYSTEM_DROP_NEXTHOP_HW_ID);
            rtable->routes[i].nhid = nh_val;
            PDS_TRACE_DEBUG("Processing route table %s, route %s -> blackhole "
                            "nh id %u, ", spec->key.str(),
                            ippfx2str(&rtable->routes[i].prefix), nh_val);
            break;

        case PDS_NH_TYPE_OVERLAY:
            // non vpc peering case
            tep = tep_db()->find(&spec->route_info->routes[i].tep);
            if (tep == NULL) {
                PDS_TRACE_ERR("TEP %s not found while processing route %s in "
                              "route table %s",
                              spec->route_info->routes[i].tep.str(),
                              ippfx2str(&spec->route_info->routes[i].prefix),
                              spec->key.str());
                ret = SDK_RET_INVALID_ARG;
                goto cleanup;
            }
            PDS_IMPL_NH_VAL_SET_NH_INFO(nh_val, NEXTHOP_TYPE_TUNNEL,
                                        ((tep_impl *)(tep->impl()))->hw_id1());
            rtable->routes[i].nhid = nh_val;
            PDS_TRACE_DEBUG("Processing route table %s, route %s prio: %u -> TEP %s, "
                            "nh id 0x%x", spec->key.str(),
                            ippfx2str(&rtable->routes[i].prefix),
                            rtable->routes[i].prio,
                            tep->key2str().c_str(), nh_val);
            break;

        case PDS_NH_TYPE_OVERLAY_ECMP:
            nh_group =
                nexthop_group_db()->find(&spec->route_info->routes[i].nh_group);
            if (nh_group == NULL) {
                PDS_TRACE_ERR("nexthop group %s not found while processing "
                              "route %s in route table %s",
                              spec->route_info->routes[i].nh_group.str(),
                              ippfx2str(&rtable->routes[i].prefix),
                              spec->key.str());
                ret = SDK_RET_INVALID_ARG;
                goto cleanup;
            }
            PDS_IMPL_NH_VAL_SET_NH_INFO(nh_val, NEXTHOP_TYPE_ECMP,
                ((nexthop_group_impl *)nh_group->impl())->hw_id());
            rtable->routes[i].nhid = nh_val;
            PDS_TRACE_DEBUG("Processing route table %s, route %s -> "
                            "nh group %s, nh id 0x%x", spec->key.str(),
                            ippfx2str(&rtable->routes[i].prefix),
                            nh_group->key2str().c_str(), nh_val);
            break;

        case PDS_NH_TYPE_PEER_VPC:
            vpc = vpc_db()->find(&spec->route_info->routes[i].vpc);
            if (vpc == NULL) {
                PDS_TRACE_ERR("vpc %s not found while processing route %s in "
                              "route table %s",
                              spec->route_info->routes[i].vpc.str(),
                              ippfx2str(&spec->route_info->routes[i].prefix),
                              spec->key.str());
                ret = SDK_RET_INVALID_ARG;
                goto cleanup;
            }
            PDS_IMPL_NH_VAL_SET_NH_INFO(nh_val, NEXTHOP_TYPE_VPC,
                                        ((vpc_impl *)vpc->impl())->hw_id());
            rtable->routes[i].nhid = nh_val;
            PDS_TRACE_DEBUG("Processing route table %s, route %s -> vpc %s, "
                            "nh id 0x%x, ", spec->key.str(),
                            ippfx2str(&rtable->routes[i].prefix),
                            vpc->key2str().c_str(), nh_val);
            break;

        case PDS_NH_TYPE_VNIC:
            vnic = vnic_db()->find(&spec->route_info->routes[i].vnic);
            if (vnic == NULL) {
                 PDS_TRACE_ERR("vnic %s not found while processing route %s in "
                               "route table %s",
                               spec->route_info->routes[i].vnic.str(),
                               ippfx2str(&spec->route_info->routes[i].prefix),
                               spec->key.str());
                 break;
            }
            PDS_IMPL_NH_VAL_SET_NH_INFO(nh_val, NEXTHOP_TYPE_NEXTHOP,
                                        ((vnic_impl *)vnic->impl())->nh_idx());
            rtable->routes[i].nhid = nh_val;
            PDS_TRACE_DEBUG("Processing route table %s, route %s -> vnic %s, "
                            "nh id 0x%x, ", spec->key.str(),
                            ippfx2str(&rtable->routes[i].prefix),
                            vnic->key2str().c_str(), nh_val);
            break;

        default:
            PDS_TRACE_ERR("Unsupported nh type %u while processing route %s in "
                          "route table %s", spec->route_info->routes[i].nh_type,
                          ippfx2str(&spec->route_info->routes[i].prefix),
                          spec->key.str());
            ret = SDK_RET_INVALID_ARG;
            goto cleanup;
            break;
        }
    }
    ret = lpm_tree_create(rtable,
                          (spec->route_info->af == IP_AF_IPV4) ?
                              ITREE_TYPE_IPV4 : ITREE_TYPE_IPV6,
                          lpm_root_addr_,
                          (spec->route_info->af == IP_AF_IPV4) ?
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
route_table_impl::program_hw(api_base *api_obj, api_obj_ctxt_t *obj_ctxt) {
    return program_route_table_(&obj_ctxt->api_params->route_table_spec);
}


sdk_ret_t
route_table_impl::update_hw(api_base *orig_obj, api_base *curr_obj,
                            api_obj_ctxt_t *obj_ctxt) {
    sdk_ret_t ret;

    // program route table in the pipeline
    ret = program_route_table_(&obj_ctxt->api_params->route_table_spec);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to update route table %s update, err %u",
                      obj_ctxt->api_params->route_table_spec.key.str(), ret);
    }
    return ret;
}

sdk_ret_t
route_table_impl::activate_hw(api_base *api_obj, api_base *orig_obj,
                              pds_epoch_t epoch, api_op_t api_op,
                              api_obj_ctxt_t *obj_ctxt)
{
    sdk_ret_t ret;
    pds_route_table_spec_t *spec;

    switch (api_op) {
    case API_OP_CREATE:
        spec = &obj_ctxt->api_params->route_table_spec;
        ret = route_table_db()->persist((route_table *)api_obj, spec);
        break;

    case API_OP_UPDATE:
        spec = &obj_ctxt->api_params->route_table_spec;
        if ((ret = route_table_db()->perish(spec->key)) ==
                SDK_RET_OK) {
            ret = route_table_db()->persist((route_table *)api_obj, spec);
        }
        break;

    case API_OP_DELETE:
        ret = route_table_db()->perish(obj_ctxt->api_params->key);
        break;

    default:
        ret = SDK_RET_INVALID_OP;
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
