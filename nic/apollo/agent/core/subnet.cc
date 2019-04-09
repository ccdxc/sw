//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/core/subnet.hpp"
#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/apollo/agent/trace.hpp"

namespace core {

static inline sdk_ret_t
subnet_create_validate (pds_subnet_spec_t *spec)
{
    pds_vcn_spec_t *vpc_spec;
    ip_addr_t vpc_ip_hi, vpc_ip_lo;
    ip_addr_t subnet_ip_hi, subnet_ip_lo;
    mac_addr_t zero_mac = {0};

    // verify VCN exists
    if ((vpc_spec = agent_state::state()->find_in_vpc_db(&spec->vcn)) == NULL) {
        PDS_TRACE_ERR("Failed to create subnet %u, vpc %u not found", spec->key.id, spec->vcn.id);
        return SDK_RET_INVALID_ARG;
    }
    // IPv4 prefix for subnet must be within VPC prefix
    if (!ipv4_prefix_is_equal(&spec->v4_pfx, &vpc_spec->v4_pfx)) {
        ipv4_prefix_ip_low(&spec->v4_pfx, &subnet_ip_lo);
        ipv4_prefix_ip_high(&spec->v4_pfx, &subnet_ip_hi);
        ipv4_prefix_ip_low(&vpc_spec->v4_pfx, &vpc_ip_lo);
        ipv4_prefix_ip_high(&vpc_spec->v4_pfx, &vpc_ip_hi);
        if (!((ip_addr_is_equal(&vpc_ip_lo, &subnet_ip_lo) ||
               ip_addr_is_lessthan(&vpc_ip_lo, &subnet_ip_lo)) &&
              (ip_addr_is_equal(&vpc_ip_hi, &subnet_ip_hi) ||
               ip_addr_is_greaterthan(&vpc_ip_hi, &subnet_ip_hi)))) {
            PDS_TRACE_ERR("Failed to create subnet %u, IPv4 prefix invalid", spec->key.id);
            return SDK_RET_INVALID_ARG;
        }
    }
    // IPv6 prefix for subnet must be within VPC prefix
    if (!ip_prefix_is_equal(&spec->v6_pfx, &vpc_spec->v6_pfx)) {
        ip_prefix_ip_low(&spec->v6_pfx, &subnet_ip_lo);
        ip_prefix_ip_high(&spec->v6_pfx, &subnet_ip_hi);
        ip_prefix_ip_low(&vpc_spec->v6_pfx, &vpc_ip_lo);
        ip_prefix_ip_high(&vpc_spec->v6_pfx, &vpc_ip_hi);
        if (!((ip_addr_is_equal(&vpc_ip_lo, &subnet_ip_lo) ||
               ip_addr_is_lessthan(&vpc_ip_lo, &subnet_ip_lo)) &&
              (ip_addr_is_equal(&vpc_ip_hi, &subnet_ip_hi) ||
               ip_addr_is_greaterthan(&vpc_ip_hi, &subnet_ip_hi)))) {
            PDS_TRACE_ERR("Failed to create subnet %u, IPv6 prefix invalid", spec->key.id);
            return SDK_RET_INVALID_ARG;
        }
    }
    // validate VR IP
    if ((spec->v4_vr_ip == 0) && (ip_addr_is_zero(&spec->v6_vr_ip))) {
        PDS_TRACE_ERR("Failed to create subnet %u, virtual router IP invalid", spec->key.id);
        return SDK_RET_INVALID_ARG;
    }

    // validate VR MAC
    if (memcmp(&spec->vr_mac, &zero_mac, sizeof(spec->vr_mac)) == 0) {
        PDS_TRACE_ERR("Failed to create subnet %u, VR MAC invalid", spec->key.id);
        return SDK_RET_INVALID_ARG;
    }
    return SDK_RET_OK;
}

sdk_ret_t
subnet_create (pds_subnet_key_t *key, pds_subnet_spec_t *spec)
{
    sdk_ret_t ret;

    if (agent_state::state()->find_in_subnet_db(key) != NULL) {
        PDS_TRACE_ERR("Failed to create subnet %u, subnet already exists", spec->key.id);
        return SDK_RET_ENTRY_EXISTS;
    }
    if ((ret = subnet_create_validate(spec)) != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to create subnet %u, err %u", spec->key.id, ret);
        return ret;
    } 
    
    if (!agent_state::state()->pds_mock_mode()) {
        if ((ret = pds_subnet_create(spec)) != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to create subnet %u, err %u", spec->key.id, ret);
            return ret;
        }
    }
    if ((ret = agent_state::state()->add_to_subnet_db(key, spec)) != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to add subnet %u to db, err %u", spec->key.id, ret);
        return ret;
    }
    return SDK_RET_OK;
}

sdk_ret_t
subnet_delete (pds_subnet_key_t *key)
{
    sdk_ret_t ret;

    if (agent_state::state()->find_in_subnet_db(key) == NULL) {
        PDS_TRACE_ERR("Failed to find subnet %u in db", key->id);
        return SDK_RET_ENTRY_NOT_FOUND;
    }
    if (!agent_state::state()->pds_mock_mode()) {
        if ((ret = pds_subnet_delete(key)) != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to delete subnet %u, err %u", key->id, ret);
            return ret;
        }
    }
    if (agent_state::state()->del_from_subnet_db(key) == false) {
        PDS_TRACE_ERR("Failed to delete subnet %u from db", key->id);
        return SDK_RET_ERR;
    }
    return SDK_RET_OK;
}

sdk_ret_t
subnet_get (pds_subnet_key_t *key, pds_subnet_info_t *info)
{
    sdk_ret_t ret;
    pds_subnet_spec_t *spec;

    spec = agent_state::state()->find_in_subnet_db(key);
    if (spec == NULL) {
        PDS_TRACE_ERR("Failed to find subnet %u in db", key->id);
        return SDK_RET_ENTRY_NOT_FOUND;
    }
    memcpy(&info->spec, spec, sizeof(pds_subnet_spec_t));
    if (!agent_state::state()->pds_mock_mode()) {
        ret = pds_subnet_read(key, info);
    }
    return ret;
}

static inline sdk_ret_t
subnet_get_all_cb (pds_subnet_spec_t *spec, void *ctxt)
{
    sdk_ret_t ret;
    pds_subnet_info_t info;
    subnet_db_cb_ctxt_t *cb_ctxt = (subnet_db_cb_ctxt_t *)ctxt;

    memcpy(&info.spec, spec, sizeof(pds_subnet_spec_t));
    if (!agent_state::state()->pds_mock_mode()) {
        ret = pds_subnet_read(&spec->key, &info);
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
