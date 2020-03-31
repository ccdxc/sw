// {C} Copyright 2020 Pensando Systems Inc. All rights reserved.

// +build apulu

package apulu

import (
	"context"
	"io"
	"time"

	"github.com/pkg/errors"

	"github.com/pensando/sw/nic/agent/dscagent/pipeline/utils"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	halapi "github.com/pensando/sw/nic/apollo/agent/gen/pds"
	"github.com/pensando/sw/venice/utils/log"
)

func queryAlerts(stream halapi.OperSvc_AlertsGetClient) {
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
		log.Infof("Received alert %+v", alert)
	}
	return
}

// HandleAlerts handles collecting and reporting of alerts
func HandleAlerts(client halapi.OperSvcClient) {
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
		ticker := time.NewTicker(time.Minute * 1)

		for {
			select {
			case <-ticker.C:
				log.Infof("Querying Alerts")
				if utils.IsHalUp() == false {
					// HAL is not up yet, skip querying
					continue
				}
				queryAlerts(stream)
			}
		}
	}(alertsStream)
	return
}
