// {C} Copyright 2020 Pensando Systems Inc. All rights reserved.

// +build apulu

package apulu

import (
	"context"
	"fmt"

	"github.com/pkg/errors"

	"github.com/pensando/sw/nic/agent/dscagent/pipeline/apulu/utils"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	operdapi "github.com/pensando/sw/nic/operd/daemon/gen/operd"
	"github.com/pensando/sw/venice/utils/log"
)

func HandleTechSupport(client operdapi.OperSvcClient, skipCores bool, instanceID string) (string, error) {
	techSupportReq := &operdapi.TechSupportRequest{
		Request: &operdapi.TechSupportSpec{
			SkipCores: skipCores,
		},
	}
	resp, err := client.TechSupportCollect(context.Background(), techSupportReq)
	if err != nil || resp == nil {
		log.Errorf("TechSupport ID: %s Create returned | Error: %s", instanceID, err)
		return "", errors.Wrapf(err, "failed to collect tech support from Operd")
	}
	if err := utils.HandleErr(types.Create, resp.ApiStatus, err, fmt.Sprintf("TechSupport: %s Create Failed for", instanceID)); err != nil {
		return "", err
	}
	log.Infof("TechSupport ID: %s Create returned | Status: %s | Response: %+v", instanceID, resp.ApiStatus, resp.Response)
	return resp.Response.FilePath, nil
}
