//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/core/subnet.hpp"
#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/apollo/agent/trace.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_subnet.hpp"

namespace core {

static inline sdk_ret_t
subnet_create_validate (pds_subnet_spec_t *spec)
{
    pds_vpc_spec_t *vpc_spec;
    mac_addr_t zero_mac = {0};

    // verify VPC exists
    if ((vpc_spec = agent_state::state()->find_in_vpc_db(&spec->vpc)) == NULL) {
        PDS_TRACE_ERR("Failed to create subnet {}, vpc {} not found",
                      spec->key.id, spec->vpc.id);
        return SDK_RET_INVALID_ARG;
    }
    // IPv4 prefix for subnet must be within VPC prefix
    if (!ipv4_prefix_within_prefix(&spec->v4_prefix, &vpc_spec->v4_prefix)) {
        PDS_TRACE_ERR("Failed to create subnet {}, IPv4 prefix invalid",
                      spec->key.id);
        return SDK_RET_INVALID_ARG;
    }
    // IPv6 prefix for subnet must be within VPC prefix
    if ((spec->v6_prefix.addr.af == IP_AF_IPV6) &&
        (vpc_spec->v6_prefix.addr.af == IP_AF_IPV6) &&
        !ip_prefix_within_prefix(&spec->v6_prefix, &vpc_spec->v6_prefix)) {
        PDS_TRACE_ERR("Failed to create subnet {}, IPv6 prefix invalid",
                      spec->key.id);
        return SDK_RET_INVALID_ARG;
    }
    // validate VR IP
    if ((spec->v4_vr_ip == 0) && (ip_addr_is_zero(&spec->v6_vr_ip))) {
        PDS_TRACE_ERR("Failed to create subnet {}, virtual router IP invalid",
                      spec->key.id);
        return SDK_RET_INVALID_ARG;
    }
    // validate VR MAC
    if (memcmp(&spec->vr_mac, &zero_mac, sizeof(spec->vr_mac)) == 0) {
        PDS_TRACE_ERR("Failed to create subnet {}, VR MAC invalid",
                      spec->key.id);
        return SDK_RET_INVALID_ARG;
    }
    return SDK_RET_OK;
}

sdk_ret_t
subnet_create (pds_obj_key_t *key, pds_subnet_spec_t *spec,
               pds_batch_ctxt_t bctxt)
{
    sdk_ret_t ret;

    if (agent_state::state()->find_in_subnet_db(key) != NULL) {
        PDS_TRACE_ERR("Failed to create subnet {}, subnet already exists",
                      spec->key.id);
        return SDK_RET_ENTRY_EXISTS;
    }
    if ((ret = subnet_create_validate(spec)) != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to create subnet {}, err {}", spec->key.id, ret);
        return ret;
    }

    if (agent_state::state()->device()->overlay_routing_en) {
        // call the metaswitch api
        if ((ret = pds_ms::subnet_create(spec, bctxt)) != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to create subnet {}, err {}",
                          spec->key.id, ret);
            return ret;
        }
    } else if (!agent_state::state()->pds_mock_mode()) {
        if ((ret = pds_subnet_create(spec, bctxt)) != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to create subnet {}, err {}",
                          spec->key.id, ret);
            return ret;
        }
    }
    if ((ret = agent_state::state()->add_to_subnet_db(key, spec)) !=
            SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to add subnet {} to db, err {}",
                      spec->key.id, ret);
        return ret;
    }
    return SDK_RET_OK;
}

static inline sdk_ret_t
subnet_update_validate (pds_subnet_spec_t *spec)
{
    pds_vpc_spec_t *vpc_spec;
    mac_addr_t zero_mac = {0};

    // verify VPC exists
    if ((vpc_spec = agent_state::state()->find_in_vpc_db(&spec->vpc)) == NULL) {
        PDS_TRACE_ERR("Failed to update subnet {}, vpc {} not found",
                      spec->key.id, spec->vpc.id);
        return SDK_RET_INVALID_ARG;
    }
    // IPv4 prefix for subnet must be within VPC prefix
    if (!ipv4_prefix_within_prefix(&spec->v4_prefix, &vpc_spec->v4_prefix)) {
        PDS_TRACE_ERR("Failed to create subnet {}, IPv4 prefix invalid",
                      spec->key.id);
        return SDK_RET_INVALID_ARG;
    }
    // IPv6 prefix for subnet must be within VPC prefix
    if (!ip_prefix_within_prefix(&spec->v6_prefix, &vpc_spec->v6_prefix)) {
        PDS_TRACE_ERR("Failed to create subnet {}, IPv6 prefix invalid",
                      spec->key.id);
        return SDK_RET_INVALID_ARG;
    }
    // validate VR IP
    if ((spec->v4_vr_ip == 0) && (ip_addr_is_zero(&spec->v6_vr_ip))) {
        PDS_TRACE_ERR("Failed to update subnet {}, virtual router IP invalid",
                      spec->key.id);
        return SDK_RET_INVALID_ARG;
    }
    // validate VR MAC
    if (memcmp(&spec->vr_mac, &zero_mac, sizeof(spec->vr_mac)) == 0) {
        PDS_TRACE_ERR("Failed to update subnet {}, VR MAC invalid",
                      spec->key.id);
        return SDK_RET_INVALID_ARG;
    }
    return SDK_RET_OK;
}

sdk_ret_t
subnet_update (pds_obj_key_t *key, pds_subnet_spec_t *spec,
               pds_batch_ctxt_t bctxt)
{
    sdk_ret_t ret;

    if (agent_state::state()->find_in_subnet_db(key) == NULL) {
        PDS_TRACE_ERR("Failed to update subnet {}, subnet doesn't exist",
                      spec->key.id);
        return SDK_RET_ENTRY_NOT_FOUND;
    }

    if ((ret = subnet_update_validate(spec)) != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to update subnet {}, err {}", spec->key.id, ret);
        return ret;
    }

    if (agent_state::state()->device()->overlay_routing_en) {
        // call the metaswitch api
        if ((ret = pds_ms::subnet_update(spec, bctxt)) != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to update subnet {}, err {}",
                          spec->key.id, ret);
            return ret;
        }
    } else if (!agent_state::state()->pds_mock_mode()) {
        if ((ret = pds_subnet_update(spec, bctxt)) != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to update subnet {}, err {}",
                          spec->key.id, ret);
            return ret;
        }
    }

    if (agent_state::state()->del_from_subnet_db(key) == false) {
        PDS_TRACE_ERR("Failed to delete subnet {} from db", key->id);
    }

    if ((ret = agent_state::state()->add_to_subnet_db(key, spec)) !=
            SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to add subnet {} to db, err {}",
                      spec->key.id, ret);
        return ret;
    }
    return SDK_RET_OK;
}

sdk_ret_t
subnet_delete (pds_obj_key_t *key, pds_batch_ctxt_t bctxt)
{
    pds_subnet_spec_t *spec;
    sdk_ret_t ret;

    if ((spec = agent_state::state()->find_in_subnet_db(key)) == NULL) {
        PDS_TRACE_ERR("Failed to find subnet {} in db", key->id);
        return SDK_RET_ENTRY_NOT_FOUND;
    }
    if (agent_state::state()->device()->overlay_routing_en) {
        // call the metaswitch api
        if ((ret = pds_ms::subnet_delete(spec, bctxt)) != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to delete subnet {}, err {}",
                          key->id, ret);
            return ret;
        }
    } else if (!agent_state::state()->pds_mock_mode()) {
        if ((ret = pds_subnet_delete(key, bctxt)) != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to delete subnet {}, err {}", key->id, ret);
            return ret;
        }
    }
    if (agent_state::state()->del_from_subnet_db(key) == false) {
        PDS_TRACE_ERR("Failed to delete subnet {} from db", key->id);
        return SDK_RET_ERR;
    }
    return SDK_RET_OK;
}

sdk_ret_t
subnet_get (pds_obj_key_t *key, pds_subnet_info_t *info)
{
    sdk_ret_t ret = SDK_RET_OK;
    pds_subnet_spec_t *spec;

    spec = agent_state::state()->find_in_subnet_db(key);
    if (spec == NULL) {
        PDS_TRACE_ERR("Failed to find subnet {} in db", key->id);
        return SDK_RET_ENTRY_NOT_FOUND;
    }
    memcpy(&info->spec, spec, sizeof(pds_subnet_spec_t));
    if (!agent_state::state()->pds_mock_mode()) {
        ret = pds_subnet_read(key, info);
    } else {
        memset(&info->stats, 0, sizeof(info->stats));
        memset(&info->status, 0, sizeof(info->status));
    }
    return ret;
}

static inline sdk_ret_t
subnet_get_all_cb (pds_subnet_spec_t *spec, void *ctxt)
{
    sdk_ret_t ret = SDK_RET_OK;
    pds_subnet_info_t info;
    subnet_db_cb_ctxt_t *cb_ctxt = (subnet_db_cb_ctxt_t *)ctxt;

    memcpy(&info.spec, spec, sizeof(pds_subnet_spec_t));
    if (!agent_state::state()->pds_mock_mode()) {
        ret = pds_subnet_read(&spec->key, &info);
    } else {
        memset(&info.stats, 0, sizeof(info.stats));
        memset(&info.status, 0, sizeof(info.status));
    }
    if (ret == SDK_RET_OK) {
        cb_ctxt->cb(&info, cb_ctxt->ctxt);
    }
    return ret;
}

sdk_ret_t
subnet_get_all (subnet_get_cb_t subnet_get_cb, void *ctxt)
{
    subnet_db_cb_ctxt_t cb_ctxt;

    cb_ctxt.cb = subnet_get_cb;
    cb_ctxt.ctxt = ctxt;
    return agent_state::state()->subnet_db_walk(subnet_get_all_cb, &cb_ctxt);
}

}    // namespace core
