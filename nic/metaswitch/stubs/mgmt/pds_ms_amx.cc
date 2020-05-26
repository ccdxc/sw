
#include "nic/metaswitch/stubs/mgmt/pds_ms_mgmt_utils.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_mgmt_state.hpp"
#include "nic/apollo/core/trace.hpp"
extern "C" {
#include <amxpenapi.h>
}

namespace pds_ms {

types::ApiStatus
amx_control (const AMXPortSpec   *req,
             AMXControlResponse     *resp)
{
    NBB_BOOL ret;
    if (req->open()) {
        if (mgmt_state_t::thread_context().state()->is_amx_open()) {
            return types::API_STATUS_OK;
        }
        PDS_TRACE_DEBUG("Open AMX socket");
        ret = amx_pen_open_socket();
    } else {
        if (!mgmt_state_t::thread_context().state()->is_amx_open()) {
            return types::API_STATUS_OK;
        }
        PDS_TRACE_DEBUG("Close AMX socket");
        ret = amx_pen_close_socket();
    }
    if (ret != 1) {
        return types::API_STATUS_ERR;
    }
    if (req->open()) {
        mgmt_state_t::thread_context().state()->set_amx_open(true);
    } else {
        mgmt_state_t::thread_context().state()->set_amx_open(false);
    }
    return types::API_STATUS_OK;
}

}
