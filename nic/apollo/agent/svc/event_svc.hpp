//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines protobuf API for event object
///
//----------------------------------------------------------------------------

#ifndef __AGENT_SVC_EVENT_SVC_HPP__
#define __AGENT_SVC_EVENT_SVC_HPP__

#include "nic/apollo/agent/svc/specs.hpp"
#include "nic/apollo/agent/svc/interface_svc.hpp"
#include "nic/apollo/agent/svc/port_svc.hpp"
#include "nic/apollo/agent/svc/event.hpp"
#include "nic/apollo/agent/core/event.hpp"

static inline pds::EventId
pds_event_id_api_to_proto_event_id (pds_event_id_t event_id)
{
    switch (event_id) {
    case PDS_EVENT_ID_LIF_CREATE:
        return pds::EVENT_ID_LIF_CREATE;
    case PDS_EVENT_ID_LIF_UPDATE:
        return pds::EVENT_ID_LIF_UPDATE;
    case PDS_EVENT_ID_LIF_UP:
        return pds::EVENT_ID_LIF_UP;
    case PDS_EVENT_ID_LIF_DOWN:
        return pds::EVENT_ID_LIF_DOWN;
    case PDS_EVENT_ID_PORT_CREATE:
        return pds::EVENT_ID_PORT_CREATE;
    case PDS_EVENT_ID_PORT_UP:
        return pds::EVENT_ID_PORT_UP;
    case PDS_EVENT_ID_PORT_DOWN:
        return pds::EVENT_ID_PORT_DOWN;
    default:
        break;
    }
    return pds::EVENT_ID_NONE;
}

static inline pds_event_id_t
pds_proto_event_id_to_api_event_id (pds::EventId proto_event_id)
{
    switch (proto_event_id) {
    case pds::EVENT_ID_PORT_CREATE:
        return PDS_EVENT_ID_PORT_CREATE;
    case pds::EVENT_ID_PORT_UP:
        return PDS_EVENT_ID_PORT_UP;
    case pds::EVENT_ID_PORT_DOWN:
        return PDS_EVENT_ID_PORT_DOWN;
    case pds::EVENT_ID_LIF_CREATE:
        return PDS_EVENT_ID_LIF_CREATE;
    case pds::EVENT_ID_LIF_UPDATE:
        return PDS_EVENT_ID_LIF_UPDATE;
    case pds::EVENT_ID_LIF_UP:
        return PDS_EVENT_ID_LIF_UP;
    case pds::EVENT_ID_LIF_DOWN:
        return PDS_EVENT_ID_LIF_DOWN;
    default:
        break;
    }
    return PDS_EVENT_ID_NONE;
}

static inline sdk_ret_t
pds_event_spec_proto_to_api_spec (pds_event_spec_t *api_spec,
                                  const pds::EventRequest_EventSpec& proto_spec)
{
    api_spec->event_id =
        pds_proto_event_id_to_api_event_id(proto_spec.eventid());
    switch (proto_spec.action()) {
    case pds::EVENT_OP_SUBSCRIBE:
        api_spec->event_op = PDS_EVENT_OP_SUBSCRIBE;
        break;
    case pds::EVENT_OP_UNSUBSCRIBE:
        api_spec->event_op = PDS_EVENT_OP_UNSUBSCRIBE;
        break;
    default:
        PDS_TRACE_ERR("Unknown event {} operation {}",
                      proto_spec.eventid(), proto_spec.action());
        return SDK_RET_INVALID_ARG;
        break;
    }
    return SDK_RET_OK;
}

static inline sdk_ret_t
pds_event_to_proto_event_response (pds::EventResponse *proto_rsp,
                                   const pds_event_t *event)
{
    proto_rsp->set_status(types::ApiStatus::API_STATUS_OK);
    proto_rsp->set_eventid(pds_event_id_api_to_proto_event_id(event->event_id));
    switch (event->event_id) {
    case PDS_EVENT_ID_LIF_CREATE:
    case PDS_EVENT_ID_LIF_UPDATE:
    case PDS_EVENT_ID_LIF_UP:
    case PDS_EVENT_ID_LIF_DOWN:
        pds_lif_api_spec_to_proto_spec(proto_rsp->mutable_lifeventinfo()->mutable_spec(),
                                       &event->lif_info.spec);
        pds_lif_api_status_to_proto(proto_rsp->mutable_lifeventinfo()->mutable_status(),
                                    &event->lif_info.status);
        return SDK_RET_OK;
    default:
    case PDS_EVENT_ID_PORT_CREATE:
    case PDS_EVENT_ID_PORT_UP:
    case PDS_EVENT_ID_PORT_DOWN:
        pds::PortSpec *spec = proto_rsp->mutable_porteventinfo()->mutable_spec();
        pds::PortStatus *status = proto_rsp->mutable_porteventinfo()->mutable_status();
        pds_port_spec_to_proto(spec, &event->port_info.info);
        pds_port_status_to_proto(status, &event->port_info.info);
        return SDK_RET_OK;
    }
    return SDK_RET_INVALID_ARG;
}

#endif    //__AGENT_SVC_EVENT_SVC_HPP__
