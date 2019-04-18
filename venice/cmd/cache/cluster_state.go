// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package cache

import (
	"context"
	"fmt"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
)

// GetCluster returns the Cluster object
// StateMgr does not cache it
func (sm *Statemgr) GetCluster() (*cluster.Cluster, error) {
	cl := sm.APIClient()
	if cl == nil {
		return nil, fmt.Errorf("API Server not reachable or down")
	}
	opts := api.ListWatchOptions{}
	clusterObjs, err := cl.Cluster().List(context.Background(), &opts)
	if err != nil || len(clusterObjs) == 0 {
		return nil, fmt.Errorf("Unable to get cluster object, err: %v", err)
	}

	// There should be only one Cluster object
	return clusterObjs[0], nil
}
