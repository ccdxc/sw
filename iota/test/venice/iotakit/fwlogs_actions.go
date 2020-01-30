package iotakit

import (
	"context"
	"fmt"

	"github.com/pensando/sw/api"
)

// GetFwLogObjectCount gets the object count for firewall logs under the bucket with the given name
func (act *ActionCtx) GetFwLogObjectCount(tenantName string, bucketName string) (int, error) {
	opts := api.ListWatchOptions{
		ObjectMeta: api.ObjectMeta{
			Tenant:    tenantName,
			Namespace: bucketName,
		},
	}

	ctx, err := act.model.VeniceLoggedInCtx(context.TODO())
	if err != nil {
		return 0, err
	}

	restClients, err := act.model.VeniceRestClient()
	if err != nil {
		return 0, err
	}

	ctx2, err := act.model.VeniceLoggedInCtx(ctx)
	if err != nil {
		return 0, err
	}

	for _, restClient := range restClients {
		list, err := restClient.ObjstoreV1().Object().List(ctx2, &opts)
		if err != nil {
			return 0, err
		}
		if len(list) != 0 {
			return len(list), nil
		}
	}

	return 0, fmt.Errorf("internal error while getting object count under default.fwlogs bucket")
}
