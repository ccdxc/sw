//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/agent/trace.hpp"
#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/core/tunnel.hpp"

namespace core {

static inline sdk_ret_t
tep_create_validate (pds_tep_spec_t *spec)
{
    if (spec->type == PDS_TEP_TYPE_SERVICE) {
        // service TEPs must have vxlan encap
        if ((spec->encap.type != PDS_ENCAP_TYPE_VXLAN) ||
            (spec->encap.val.vnid == 0)) {
            PDS_TRACE_ERR("Service tunnel {}, tunnel ip {} has invalid "
                          "vxlan encap ({}, {})",
                          spec->key.str(),
                          ipaddr2str(&spec->remote_ip),
                          spec->encap.type, spec->encap.val.vnid);
            return sdk::SDK_RET_INVALID_ARG;
        }
    }
    return SDK_RET_OK;
}

sdk_ret_t
tep_create (pds_obj_key_t *key, pds_tep_spec_t *spec, pds_batch_ctxt_t bctxt)
{
    sdk_ret_t ret;

    if ((ret = tep_create_validate(spec)) != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to create tunnel {}, tunnel ip {}, err {}",
                      spec->key.str(), ipaddr2str(&spec->remote_ip), ret);
        return ret;
    }
    if (!agent_state::state()->pds_mock_mode()) {
        if ((ret = pds_tep_create(spec, bctxt)) != SDK_RET_OK) {
            return ret;
        }
    }
    return SDK_RET_OK;
}

sdk_ret_t
tep_update (pds_obj_key_t *key, pds_tep_spec_t *spec, pds_batch_ctxt_t bctxt)
{
    sdk_ret_t ret;

    if (!agent_state::state()->pds_mock_mode()) {
        if ((ret = pds_tep_update(spec, bctxt)) != SDK_RET_OK) {
            return ret;
        }
    }
    return SDK_RET_OK;
}

sdk_ret_t
tep_delete (pds_obj_key_t *key, pds_batch_ctxt_t bctxt)
{
    sdk_ret_t ret;

    if (!agent_state::state()->pds_mock_mode()) {
        if ((ret = pds_tep_delete(key, bctxt)) != SDK_RET_OK) {
            return ret;
        }
    }
    return SDK_RET_OK;
}

sdk_ret_t
tep_get (pds_obj_key_t *key, pds_tep_info_t *info)
{
    sdk_ret_t ret = SDK_RET_OK;

    if (!agent_state::state()->pds_mock_mode()) {
        ret = pds_tep_read(key, info);
    } else {
        memset(&info->spec, 0, sizeof(info->spec));
        memset(&info->stats, 0, sizeof(info->stats));
        memset(&info->status, 0, sizeof(info->status));
    }
    return ret;
}

sdk_ret_t
tep_get_all (tep_read_cb_t tep_read_cb, void *ctxt)
{
    return pds_tep_read_all(tep_read_cb, ctxt);
}

}    // namespace core
