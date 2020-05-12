//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//------------------------------------------------------------------------------

#include "nic/sdk/lib/operd/operd.hpp"
#include "nic/operd/alerts/alerts.hpp"
#include "nic/operd/consumers/pen_oper/core/state.hpp"
#include "alerts.hpp"

static void inline
oper_alerts_to_resp (const operd::Alert *alert, AlertsGetResponse &rsp)
{
    operd::Alert *rsp_alert = rsp.mutable_response();

    *rsp_alert = *alert;
    rsp.set_apistatus(types::ApiStatus::API_STATUS_OK);
}

static inline bool
alert_send_cb (sdk::lib::event_id_t event_id_t, void *alert_ctxt, void *ctxt)
{
    AlertsGetResponse *resp = (AlertsGetResponse *)alert_ctxt;
    ServerWriter<AlertsGetResponse> *stream =
        (ServerWriter<AlertsGetResponse> *)ctxt;

    if (!stream->Write(*resp)) {
        // client closed connection, return false so that listener gets removed
        fprintf(stderr, "alerts write failed\n");
        return false;
    }
    return true;
}

void
publish_alert (const operd::Alert *alert)
{
    AlertsGetResponse alerts_rsp;

    oper_alerts_to_resp(alert, alerts_rsp);
    // send notification to all clients
    core::pen_oper_state::state()->event_mgr()->notify_event(
        PENOPER_INFO_TYPE_ALERTS, &alerts_rsp, alert_send_cb);
}

Status
AlertsSvcImpl::AlertsGet(ServerContext *context, const types::Empty *req,
                         ServerWriter<AlertsGetResponse> *stream) {
    core::pen_oper_state::state()->event_mgr()->subscribe(
        PENOPER_INFO_TYPE_ALERTS, stream);

    while (true) {
        if (!core::pen_oper_state::state()->event_mgr()->is_listener_active(stream)) {
            // client no more active
            break;
        }
        // spinning for alerts
        pthread_yield();
    }
    return Status::OK;
}
