// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package cache

import (
	"context"
	"fmt"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/venice/utils/log"
)

// GetCluster returns the Cluster object
// StateMgr does not cache it
func (sm *Statemgr) GetCluster() (*cluster.Cluster, error) {
	for i := 0; i < maxAPIServerWriteRetries; i++ {
		ctx, cancel := context.WithTimeout(context.Background(), apiServerRPCTimeout)
		cl := sm.APIClient()
		if cl == nil {
			cancel()
			log.Errorf("API Server not reachable or down")
			time.Sleep(apiClientRetryInterval)
			continue
		}
		opts := api.ListWatchOptions{}
		clusterObjs, err := cl.Cluster().List(ctx, &opts)
		if err == nil && len(clusterObjs) > 0 {
			cancel()
			return clusterObjs[0], nil
		}
		log.Errorf("Error getting cluster objct: %v", err)
		cancel()
	}
	return nil, fmt.Errorf("Unable to get Cluster object, number of retries exhausted")
}

// UpdateClusterStatus updates the Leader and Quorum parts of cluster object
func (sm *Statemgr) UpdateClusterStatus(clusterObj *cluster.Cluster) error {
	for i := 0; i < maxAPIServerWriteRetries; i++ {
		if !sm.isLeader() {
			log.Infof("CMD instance is no longer leader, exiting UpdateClusterStatus after %d tries", i)
			return nil
		}
		ctx, cancel := context.WithTimeout(context.Background(), apiServerRPCTimeout)
		cl := sm.APIClient()
		if cl == nil {
			cancel()
			log.Errorf("API Server not reachable or down")
			time.Sleep(apiClientRetryInterval)
			continue
		}
		_, err := sm.APIClient().Cluster().UpdateStatus(ctx, clusterObj)
		if err == nil {
			cancel()
			log.Infof("Updated Cluster object in ApiServer: %+v", clusterObj)
			return nil
		}
		log.Errorf("Error updating Cluster object: %v", err)
		cancel()
	}
	return fmt.Errorf("Unable to update Cluster object, number of retries exhausted")
}
