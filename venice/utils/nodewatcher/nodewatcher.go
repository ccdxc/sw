// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package nodewatcher

import (
	"context"
	"fmt"
	"time"

	"github.com/minio/minio/cmd/logger"
	"github.com/shirou/gopsutil/mem"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	tsdb "github.com/pensando/sw/venice/utils/ntsdb"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/runtime"
)

const (
	minFrequency = 10 // minimum send interval in seconds
)

// Metrics reported for a Venice Node or NAPLES.
// TODO: transition to using NodeMetrics schema once its defined.
type nodeMetrics struct {
	MemTotal       api.Gauge
	MemAvailable   api.Gauge
	MemFree        api.Gauge
	MemUsed        api.Gauge
	MemUsedPercent api.Gauge
}

// nodewatcher monitors system resources. It can run on Venice Nodes or on NAPLES.
type nodewatcher struct {
	frequency int
	table     tsdb.Table
	metricObj *nodeMetrics
	logger    log.Logger
}

// NewNodeWatcher starts a watcher that monitors system resources.
func NewNodeWatcher(ctx context.Context, obj runtime.Object, resolver resolver.Interface, frequency int, logger log.Logger) error {
	meta, err := runtime.GetObjectMeta(obj)
	if err != nil {
		return err
	}

	if resolver == nil {
		return fmt.Errorf("Need a resolver")
	}

	if frequency < minFrequency {
		return fmt.Errorf("Min frequency is %v", minFrequency)
	}

	opts := &tsdb.Opts{
		ClientName:              fmt.Sprintf("%v-%v", obj.GetObjectKind(), meta.Name),
		ResolverClient:          resolver,
		Collector:               globals.Collector,
		DBName:                  "default",
		SendInterval:            time.Duration(frequency) * time.Second,
		ConnectionRetryInterval: 100 * time.Millisecond,
	}

	// Init the TSDB
	tsdb.Init(ctx, opts)

	logger.Infof("Creating new table")
	metricObj := &nodeMetrics{}
	table, err := tsdb.NewOTable(obj, metricObj, &tsdb.TableOpts{})
	if err != nil {
		return err
	}
	w := &nodewatcher{
		frequency: frequency,
		table:     table,
		metricObj: metricObj,
		logger:    logger,
	}
	go w.periodicUpdate(ctx)
	return nil
}

// periodically updates the system metrics.
func (w *nodewatcher) periodicUpdate(ctx context.Context) {
	for {
		select {
		case <-ctx.Done():
			logger.Info("Metrics watcher context cancelled, exiting")
			return
		case <-time.After(time.Duration(w.frequency) * time.Second):
			t := time.Now()
			vmstat, err := mem.VirtualMemory()
			if err != nil {
				w.logger.Errorf("Node Watcher: failed to read virtual memory info, error: %v", err)
				continue
			}
			w.logger.Infof("Node Watcher: sending new metrics %+v", vmstat)
			w.metricObj.MemAvailable.Set(float64(vmstat.Available), t)
			w.metricObj.MemFree.Set(float64(vmstat.Free), t)
			w.metricObj.MemUsed.Set(float64(vmstat.Used), t)
			w.metricObj.MemTotal.Set(float64(vmstat.Total), t)
			w.metricObj.MemUsedPercent.Set(float64(vmstat.UsedPercent), t)
		}
	}
}
