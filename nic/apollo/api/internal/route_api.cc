//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file implements internal route related APIs
///
//----------------------------------------------------------------------------

#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/include/pds_batch.hpp"
#include "nic/apollo/api/include/pds_route.hpp"
#include "nic/apollo/api/internal/pds_route.hpp"
#include "nic/apollo/api/pds_state.hpp"

void pds_route_table_spec_s::deepcopy_(const pds_route_table_spec_t& route_table) {
    // self-assignment guard
    if (this == &route_table) {
        return;
    }

    if (route_table.route_info == NULL) {
        if (route_info) {
            // incorrect usage !!
            SDK_ASSERT(FALSE);
        } else {
            // no routes to copy, just copy the key
             memcpy(this, &route_table, sizeof(pds_route_table_spec_t));
             return;
        }
    }

    // free internally allocated memory, if any
    if (route_info && priv_mem_) {
        PDS_TRACE_VERBOSE("Freeing internally allocated route info");
        SDK_FREE(PDS_MEM_ALLOC_ID_ROUTE_TABLE, route_info);
    }
    PDS_TRACE_VERBOSE("Deep copying route table spec");
    route_info =
        (route_info_t *)SDK_MALLOC(PDS_MEM_ALLOC_ID_ROUTE_TABLE,
                            ROUTE_INFO_SIZE(route_table.route_info->num_routes));
    SDK_ASSERT(route_info != NULL);
    priv_mem_ = true;
    key = route_table.key;
    memcpy(route_info, route_table.route_info,
           ROUTE_INFO_SIZE(route_table.route_info->num_routes));
}

void pds_route_table_spec_s::move_(pds_route_table_spec_t&& route_table) {
    // self-assignment guard
    if (this == &route_table) {
        return;
    }

    if (route_table.route_info == NULL) {
        if (route_info) {
            // incorrect usage !!
            SDK_ASSERT(FALSE);
        }
        // fall through
    }
    // free internally allocated memory, if any
    if (route_info && priv_mem_) {
        PDS_TRACE_VERBOSE("Freeing internally allocated route info");
        SDK_FREE(PDS_MEM_ALLOC_ID_ROUTE_TABLE, route_info);
    }
    // shallow copy the spec
    PDS_TRACE_VERBOSE("Shallow copying route table spec");
    memcpy(this, &route_table, sizeof(pds_route_table_spec_t));
    // transfer the ownership of the memory
    route_table.route_info = NULL;
}

namespace api {

#define PDS_MAX_UNDERLAY_ROUTES                   1
#define PDS_UNDERLAY_ROUTE_API_EPOCH_START        0x00800000
uint32_t g_num_routes = 0;
typedef struct route_entry_s {
    uint8_t valid:1;
    uint8_t rsvd:7;
    pds_route_spec_t spec;
} route_entry_t;
static route_entry_t g_route_db[PDS_MAX_UNDERLAY_ROUTES];
static uint32_t g_route_api_epoch_ = PDS_UNDERLAY_ROUTE_API_EPOCH_START;

#if 0
sdk_ret_t
pds_underlay_route_create (_In_ pds_route_spec_t *spec)
{
    if (g_num_routes >= PDS_MAX_UNDERLAY_ROUTES) {
        PDS_TRACE_ERR("Failed to create route %s, underlay route table is full",
                      spec->key.str());
        return SDK_RET_NO_RESOURCE;
    }
    if (route_table_db()->find(&spec->route_table) == NULL) {
        PDS_TRACE_ERR("Failed to create route %s, route table %s not found",
                      spec->key.str(), spec->route_table.str());
        return SDK_RET_INVALID_ARG;
    }
    g_route_db[g_num_routes].valid = TRUE;
    g_num_routes++;

    return SDK_RET_OK;
}
#endif

static bool
tep_upd_walk_cb_ (void *obj, void *ctxt) {
    sdk_ret_t ret;
    pds_tep_spec_t spec;
    tep_entry *tep = (tep_entry *)obj;
    pds_batch_ctxt_t bctxt = (pds_batch_ctxt_t)ctxt;

    if (tep->nh_type() == PDS_NH_TYPE_OVERLAY) {
        // tunnel pointing to another tunnel, skip this
        return false;
    }
    spec.key = tep->key();
    spec.remote_ip = tep->ip();
    memcpy(spec.mac, tep->mac(), ETH_ADDR_LEN);
    spec.type = tep->type();
    spec.encap = tep->encap();

    if (g_num_routes == 0) {
        // TEP walk triggered by route delete
        spec.nh_type = PDS_NH_TYPE_NONE;
    } else {
        // TEP walk triggered by route add/update,
        // 1st route is the best route !!
        spec.nh_type = g_route_db[0].spec.route.nh_type;
        if (spec.nh_type == PDS_NH_TYPE_UNDERLAY_ECMP) {
            spec.nh_group = g_route_db[0].spec.route.nh_group;
        } else if (spec.nh_type == PDS_NH_TYPE_UNDERLAY) {
            spec.nh = g_route_db[0].spec.route.nh;
        }
    }
    ret = pds_tep_update(&spec, bctxt);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to update TEP %s, err %u", spec.key.str(), ret);
        // update other TEPs still !!
    }
    return false;
}

static sdk_ret_t
pds_update_teps (void)
{
    pds_batch_ctxt_t bctxt;
    pds_batch_params_t batch_params = { 0 };

    batch_params.epoch = g_route_api_epoch_;
    batch_params.async = false;
    bctxt = pds_batch_start(&batch_params);
    tep_db()->walk(tep_upd_walk_cb_, (void *)bctxt);
    return pds_batch_commit(bctxt);
}

sdk_ret_t
pds_underlay_route_update (_In_ pds_route_spec_t *spec)
{
    bool found = false;

    for (uint32_t i = 0; i < g_num_routes; i++) {
        if (g_route_db[i].valid &&
            (ip_prefix_is_equal(&g_route_db[i].spec.route.prefix,
                                &spec->route.prefix))) {
            PDS_TRACE_DEBUG("Updating underlay route %s",
                            ippfx2str(&spec->route.prefix));
            g_route_db[i].spec = *spec;
            found = true;
            break;
        }
    }
    if (!found) {
        PDS_TRACE_DEBUG("Creating underlay route %s",
                        ippfx2str(&spec->route.prefix));
        if (g_num_routes >= PDS_MAX_UNDERLAY_ROUTES) {
            PDS_TRACE_ERR("Failed to create route %s, underlay route table "
                          "is full", ippfx2str(&spec->route.prefix));
            return SDK_RET_NO_RESOURCE;
        }
        g_route_db[g_num_routes].valid = TRUE;
        g_num_routes++;
    }
    return pds_update_teps();
}

sdk_ret_t
pds_underlay_route_delete (_In_ ip_prefix_t *prefix)
{
    for (uint32_t i = 0; i < g_num_routes; i++) {
        if (g_route_db[i].valid &&
            (ip_prefix_is_equal(&g_route_db[i].spec.route.prefix, prefix))) {
            // replace this with the last valid route and fill this slot
            g_route_db[i] = g_route_db[g_num_routes - 1];
            g_route_db[g_num_routes - 1].valid = FALSE;
            g_num_routes--;
            return pds_update_teps();
        }
    }
    PDS_TRACE_ERR("Failed to delete route %s, route not found",
                  ippfx2str(prefix));
    return SDK_RET_INVALID_OP;
}

#if 0
pds_obj_key_t
pds_underlay_nexthop (_In_ ip_addr_t ip_addr)
{
    // TODO
    return k_pds_obj_key_invalid;
}
#endif

}    // namespace api
