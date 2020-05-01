// {C} Copyright 2020 Pensando Systems Inc. All rights reserved.

// +build apulu

package apulu

import (
	"context"
	"fmt"
	"io"
	"time"

	gogoproto "github.com/gogo/protobuf/types"
	"github.com/pkg/errors"
	"github.com/satori/go.uuid"

	"github.com/pensando/sw/api"
	evtsapi "github.com/pensando/sw/api/generated/events"
	"github.com/pensando/sw/events/generated/eventtypes"
	"github.com/pensando/sw/nic/agent/dscagent/pipeline/utils"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	halapi "github.com/pensando/sw/nic/apollo/agent/gen/pds"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/events"
	"github.com/pensando/sw/venice/utils/log"
)

// helper function to convert HAL alert to venice alert
func convertToVeniceAlert(nEvt *halapi.Alert) *evtsapi.Event {
	uuid := uuid.NewV4().String()
	// TODO: Timestamp is not populated by pdsagent
	// until then use current time instead of nEvt.GetTimestamp()
	ts := gogoproto.TimestampNow()

	evtType := eventtypes.EventType_value[nEvt.GetName()]
	eTypeAttrs := eventtypes.GetEventTypeAttrs(eventtypes.EventType(evtType))
	vAlert := &evtsapi.Event{
		TypeMeta: api.TypeMeta{Kind: "Event"},
		ObjectMeta: api.ObjectMeta{
			Name: uuid,
			UUID: uuid,
			CreationTime: api.Timestamp{
				Timestamp: *ts,
			},
			ModTime: api.Timestamp{
				Timestamp: *ts,
			},
			Tenant:    globals.DefaultTenant,
			Namespace: globals.DefaultNamespace,
			SelfLink:  fmt.Sprintf("/events/v1/events/%s", uuid),
			Labels:    map[string]string{"_category": globals.Kind2Category("Event")},
		},
		EventAttributes: evtsapi.EventAttributes{
			Type:     eTypeAttrs.EType,
			Severity: eTypeAttrs.Severity,
			Message:  nEvt.GetMessage(),
			Category: eTypeAttrs.Category,
			// nodename will be populated by Dispatcher if Source is unpopulated
			// Source:   &evtsapi.EventSource{Component: "DSC"},
			Count: 1,
		},
	}

	return vAlert
}

func queryAlerts(evtsDispatcher events.Dispatcher, stream halapi.OperSvc_AlertsGetClient) {
	for {
		resp, err := stream.Recv()
		if err == io.EOF {
			break
		}
		if err != nil {
			log.Error(errors.Wrapf(types.ErrAlertsRecv,
				"Error receiving alerts | Err %v", err))
			continue
		}
		if resp.GetApiStatus() != halapi.ApiStatus_API_STATUS_OK {
			log.Error(errors.Wrapf(types.ErrAlertsRecv,
				"Alerts response failure, | Err %v",
				resp.GetApiStatus().String()))
			continue
		}
		// process the alerts
		alert := resp.GetResponse()
		// convert to venice alert format
		vAlert := convertToVeniceAlert(alert)
		// send to dispatcher
		if err := evtsDispatcher.Action(*vAlert); err != nil {
			log.Error(errors.Wrapf(types.ErrAlertsFwd,
				"Error forwarding alert {%+v} to venice, err: %v", alert, err))
		} else {
			log.Infof("Forwarded alert %+v to venice", alert)
		}
	}
	return
}

// HandleAlerts handles collecting and reporting of alerts
func HandleAlerts(evtsDispatcher events.Dispatcher, client halapi.OperSvcClient) {
	emptyStruct := &halapi.Empty{}
	// create a stream for alerts
	alertsStream, err := client.AlertsGet(context.Background(), emptyStruct)
	if err != nil {
		log.Error(errors.Wrapf(types.ErrAlertsGet,
			"AlertsGet failure | Err %v", err))
		return
	}

	// periodically query for alerts from PDS agent
	go func(stream halapi.OperSvc_AlertsGetClient) {
		ticker := time.NewTicker(time.Second * 5)

		for {
			select {
			case <-ticker.C:
				pdsAgentURL := fmt.Sprintf("127.0.0.1:%s", types.PDSGRPCDefaultPort)
				if utils.IsHalUp(pdsAgentURL) == false {
					// HAL is not up yet, skip querying
					continue
				}
				queryAlerts(evtsDispatcher, stream)
			}
		}
	}(alertsStream)
	return
}
