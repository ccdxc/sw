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
    route_table_impl    *impl;

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

// NOTE: reserve_resources() logic is same for both API_OP_CREATE and
//       API_OP_UPDATE as update doesn't reuse any of the existing resources
//       for this object
sdk_ret_t
route_table_impl::reserve_resources(api_base *api_obj,
                                    api_obj_ctxt_t *obj_ctxt) {
    sdk_ret_t ret;
    uint32_t lpm_block_id;
    pds_route_t *route_spec;
    pds_route_table_spec_t *spec;

    spec = &obj_ctxt->api_params->route_table_spec;
    // record the fact that resource reservation was attempted
    // NOTE: even if we partially acquire resources and fail eventually,
    //       this will ensure that proper release of resources will happen
    api_obj->set_rsvd_rsc();

    for (uint32_t i = 0; i < spec->num_routes; i++) {
        route_spec = &spec->routes[i];
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
route_table_impl::program_hw(api_base *api_obj, api_obj_ctxt_t *obj_ctxt) {
    sdk_ret_t              ret;
    tep_entry              *tep;
    vpc_entry              *vpc;
    vnic_entry             *vnic;
    pds_route_table_spec_t *spec;
    uint32_t               nh_val;
    pds_obj_key_t          vpc_key;
    route_table_t          *rtable;
    p4pd_error_t           p4pd_ret;
    nexthop_group          *nh_group;
    pds_route_t            *route_spec;
    dnat_actiondata_t      dnat_data;

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
    rtable->pbr_enabled = spec->enable_pbr;
    rtable->default_nhid = 0;
    PDS_IMPL_NH_VAL_SET_NH_INFO(rtable->default_nhid, NEXTHOP_TYPE_NEXTHOP,
                                PDS_IMPL_SYSTEM_DROP_NEXTHOP_HW_ID);
    if (rtable->af == IP_AF_IPV4) {
        rtable->max_routes = route_table_impl_db()->v4_max_routes();
    } else {
        rtable->max_routes = route_table_impl_db()->v6_max_routes();
    }
    rtable->num_routes = spec->num_routes;
    for (uint32_t i = 0, dnat_idx = 0; i < rtable->num_routes; i++) {
        nh_val = 0;
        route_spec = &spec->routes[i];
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
                ret = SDK_RET_INVALID_ARG;
                goto cleanup;
            }
            dnat_idx++;
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
            tep = tep_db()->find(&spec->routes[i].tep);
            if (tep == NULL) {
                PDS_TRACE_ERR("TEP %s not found while processing route %s in "
                              "route table %s", spec->routes[i].tep.str(),
                              ippfx2str(&spec->routes[i].prefix),
                              spec->key.str());
                ret = SDK_RET_INVALID_ARG;
                goto cleanup;
            }
            PDS_IMPL_NH_VAL_SET_NH_INFO(nh_val, NEXTHOP_TYPE_TUNNEL,
                                        ((tep_impl *)(tep->impl()))->hw_id());
            rtable->routes[i].nhid = nh_val;
            PDS_TRACE_DEBUG("Processing route table %s, route %s prio: %u -> TEP %s, "
                            "nh id 0x%x", spec->key.str(),
                            ippfx2str(&rtable->routes[i].prefix),
                            rtable->routes[i].prio,
                            tep->key2str().c_str(), nh_val);
            break;

        case PDS_NH_TYPE_OVERLAY_ECMP:
            nh_group = nexthop_group_db()->find(&spec->routes[i].nh_group);
            if (nh_group == NULL) {
                PDS_TRACE_ERR("nexthop group %s not found while processing "
                              "route %s in route table %s",
                              spec->routes[i].nh_group.str(),
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
            vpc = vpc_db()->find(&spec->routes[i].vpc);
            if (vpc == NULL) {
                PDS_TRACE_ERR("vpc %s not found while processing route %s in "
                              "route table %s", spec->routes[i].vpc.str(),
                              ippfx2str(&spec->routes[i].prefix),
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
            vnic = vnic_db()->find(&spec->routes[i].vnic);
            if (vnic == NULL) {
                 PDS_TRACE_ERR("vnic %s not found while processing route %s in "
                               "route table %s", spec->routes[i].vnic.str(),
                               ippfx2str(&spec->routes[i].prefix),
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
                          "route table %s", spec->routes[i].nh_type,
                          ippfx2str(&spec->routes[i].prefix), spec->key.str());
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
route_table_impl::activate_hw(api_base *api_obj, api_base *orig_obj,
                              pds_epoch_t epoch, api_op_t api_op,
                              api_obj_ctxt_t *obj_ctxt)
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
