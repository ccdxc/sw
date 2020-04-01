//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/core/service.hpp"
#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/apollo/agent/trace.hpp"

namespace core {

static inline sdk_ret_t
service_get_all_cb (pds_svc_mapping_spec_t *spec, void *ctxt)
{
    sdk_ret_t ret = SDK_RET_OK;
    pds_svc_mapping_info_t info;
    service_db_cb_ctxt_t *cb_ctxt = (service_db_cb_ctxt_t *)ctxt;

    memcpy(&info.spec, spec, sizeof(pds_svc_mapping_spec_t));
    if (!agent_state::state()->pds_mock_mode()) {
        ret = pds_svc_mapping_read(&spec->key, &info);
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
service_get_all (service_get_cb_t service_get_cb, void *ctxt)
{
    service_db_cb_ctxt_t cb_ctxt;

    cb_ctxt.cb = service_get_cb;
    cb_ctxt.ctxt = ctxt;
    return agent_state::state()->service_db_walk(service_get_all_cb, &cb_ctxt);
}

}    // namespace core
