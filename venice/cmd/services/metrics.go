// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package services

import (
	"context"
	"fmt"
	"sync"
	"time"

	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/venice/cmd/types"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/nodemetrics"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/tsdb"
)

var (
	// MetricsSendInterval is the send interval for node and cluster metrics
	MetricsSendInterval     = 30 * time.Second
	connectionRetryInterval = 500 * time.Millisecond
	dbName                  = "default"
)

type metricsService struct {
	sync.Mutex
	running     bool
	nodeID      string
	clusterID   string
	tsdbOpts    *tsdb.Opts
	table       tsdb.Obj
	ctx         context.Context
	cancelFn    context.CancelFunc
	metrics     *types.ClusterMetrics
	nodeMetrics nodemetrics.NodeInterface
}

// NewMetricsService returns a new metricsService instance
// this is supposed to be a singleton
func NewMetricsService(nodeID, clusterID string, rc resolver.Interface) types.MetricsService {
	opts := &tsdb.Opts{
		ClientName:              nodeID,
		ResolverClient:          rc,
		Collector:               globals.Collector,
		DBName:                  dbName,
		SendInterval:            MetricsSendInterval,
		ConnectionRetryInterval: connectionRetryInterval,
	}
	ms := &metricsService{
		nodeID:    nodeID,
		clusterID: clusterID,
		tsdbOpts:  opts,
	}
	return ms
}

// Start intializes the metrics reporting service
func (ms *metricsService) Start() error {
	ms.Lock()
	defer ms.Unlock()

	ms.ctx, ms.cancelFn = context.WithCancel(context.Background())
	tsdb.Init(ms.ctx, ms.tsdbOpts)
	log.Infof("TSDB initialized, options: %+v", ms.tsdbOpts)

	clusterMeta := &cluster.Cluster{}
	clusterMeta.Defaults("all")
	clusterMeta.Name = ms.clusterID
	metrics := &types.ClusterMetrics{}
	table, err := tsdb.NewVeniceObj(clusterMeta, metrics, nil)
	if err != nil {
		return fmt.Errorf("Error creating TSDB table for object %v: %v", clusterMeta, err)
	}
	ms.table = table
	ms.metrics = metrics

	nodeMeta := &cluster.Node{}
	nodeMeta.Defaults("all")
	nodeMeta.Name = ms.nodeID
	nm, err := nodemetrics.NewNodeMetrics(ms.ctx, nodeMeta, ms.tsdbOpts.SendInterval, log.WithContext("pkg", "nodemetrics"))
	if err != nil {
		return fmt.Errorf("Error starting node watcher: %v", err)
	}

	ms.nodeMetrics = nm
	ms.running = true
	log.Infof("Started metrics service")
	return nil
}

// Stop terminates metrics reporting
func (ms *metricsService) Stop() {
	ms.Lock()
	defer ms.Unlock()
	ms.running = false
	log.Infof("Stopping metrics service")
	ms.nodeMetrics.Close()
	if ms.cancelFn != nil {
		ms.cancelFn()
	}
	tsdb.Cleanup()
}

// UpdateMetrics updates a subset of the metrics
func (ms *metricsService) UpdateCounters(m map[string]int64) {
	ms.Lock()
	defer ms.Unlock()
	if !ms.running {
		return
	}
	ms.table.AtomicBegin(time.Now())
	for k, v := range m {
		ms.table.Gauge(k).Set(float64(v))
	}
	ms.table.AtomicEnd()
}

// IsRunning returns true if the service is running
func (ms *metricsService) IsRunning() bool {
	ms.Lock()
	defer ms.Unlock()
	return ms.running
}
