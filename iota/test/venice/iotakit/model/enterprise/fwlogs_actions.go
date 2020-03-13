package enterprise

import (
	"context"
	"strings"

	"github.com/pensando/sw/api"
)

// GetFwLogObjectCount gets the object count for firewall logs under the bucket with the given name
func (sm *SysModel) GetFwLogObjectCount(
	tenantName string, bucketName string, objectKeyPrefix string) (int, error) {
	opts := api.ListWatchOptions{
		ObjectMeta: api.ObjectMeta{
			Tenant:    tenantName,
			Namespace: bucketName,
		},
	}

	ctx, err := sm.VeniceLoggedInCtx(context.Background())
	if err != nil {
		return 0, err
	}

	restClients, err := sm.VeniceRestClient()
	if err != nil {
		return 0, err
	}

	count := 0
	for _, restClient := range restClients {
		list, err := restClient.ObjstoreV1().Object().List(ctx, &opts)
		if err != nil {
			return 0, err
		}

		if len(list) != 0 {
			if objectKeyPrefix == "" {
				return len(list), nil
			}

			for _, o := range list {
				if strings.Contains(o.Name, objectKeyPrefix) {
					count++
				}
			}

			return count, nil
		}
	}

	return 0, nil
}
