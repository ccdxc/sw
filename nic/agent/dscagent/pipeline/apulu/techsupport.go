// {C} Copyright 2020 Pensando Systems Inc. All rights reserved.

// +build apulu

package apulu

import (
	"context"
	"fmt"

	"github.com/pensando/sw/nic/agent/dscagent/pipeline/apulu/utils"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	halapi "github.com/pensando/sw/nic/apollo/agent/gen/pds"
	"github.com/pensando/sw/venice/utils/log"
)

func HandleTechSupport(client halapi.OperSvcClient, skipCores bool, instanceID string) (string, error) {
	techSupportReq := &halapi.TechSupportRequest{
		Request: &halapi.TechSupportSpec{
			SkipCores: skipCores,
		},
	}
	resp, err := client.TechSupportCollect(context.Background(), techSupportReq)
	if resp != nil {
		if err := utils.HandleErr(types.Create, resp.ApiStatus, err, fmt.Sprintf("TechSupport: %s Create Failed for", instanceID)); err != nil {
			return "", err
		}
	}
	log.Infof("TechSupport ID: %s Create returned | Status: %s | Response: %+v", instanceID, resp.ApiStatus, resp.Response)
	return resp.Response.FilePath, nil
}
