// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package nodewatcher

import (
	"context"
	"fmt"
	"math"
	"time"

	"github.com/shirou/gopsutil/cpu"
	"github.com/shirou/gopsutil/disk"
	"github.com/shirou/gopsutil/mem"
	"github.com/shirou/gopsutil/net"

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
	MemTotal         api.Gauge
	MemAvailable     api.Gauge
	MemFree          api.Gauge
	MemUsed          api.Gauge
	MemUsedPercent   api.Gauge
	CPUUsedPercent   api.Gauge
	DiskFree         api.Gauge
	DiskUsed         api.Gauge
	DiskUsedPercent  api.Gauge
	DiskTotal        api.Gauge
	InterfaceName    api.String
	InterfaceRxBytes api.Gauge
	InterfaceTxBytes api.Gauge
}

// nodewatcher monitors system resources. It can run on Venice Nodes or on NAPLES.
type nodewatcher struct {
	frequency int
	table     tsdb.Obj
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
		return fmt.Errorf("need a resolver")
	}

	if frequency < minFrequency {
		return fmt.Errorf("minimum frequency is %v", minFrequency)
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
	table, err := tsdb.NewVeniceObj(obj, metricObj, nil)
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
			w.logger.Info("Metrics watcher context cancelled, exiting")
			return
		case <-time.After(time.Duration(w.frequency) * time.Second):
			// memory
			vmstat, err := mem.VirtualMemory()
			if err != nil || vmstat == nil {
				w.logger.Errorf("Node Watcher: failed to read virtual memory info, error: %v", err)
				continue
			}
			w.metricObj.MemAvailable.Set(float64(vmstat.Available >> 20))
			w.metricObj.MemFree.Set(float64(vmstat.Free >> 20))
			w.metricObj.MemUsed.Set(float64(vmstat.Used >> 20))
			w.metricObj.MemTotal.Set(float64(vmstat.Total >> 20))
			w.metricObj.MemUsedPercent.Set(math.Floor(vmstat.UsedPercent*100) / 100)

			// cpu
			cpuPercent, err := cpu.Percent(0, false)
			if err != nil {
				w.logger.Errorf("Node Watcher: failed to read cpu percent, error: %v", err)
				continue
			}
			w.metricObj.CPUUsedPercent.Set(math.Ceil(cpuPercent[0]*100) / 100)
			w.logger.Debugf("Node Watcher: recording new metrics, mem used %v%%, cpu used %v%%", vmstat.UsedPercent, cpuPercent[0])

			// disk
			part, err := disk.PartitionsWithContext(ctx, false)
			if err != nil {
				w.logger.Errorf("Node Watcher: failed to read disk partitions, error: %v", err)
				continue
			}

			diskFree := uint64(0)
			diskUsed := uint64(0)
			diskTotal := uint64(0)

			for _, p := range part {
				usage, err := disk.UsageWithContext(ctx, p.Mountpoint)
				if err != nil {
					w.logger.Errorf("Node Watcher: failed to read disk %+v, error: %v", p, err)
					continue
				}

				diskFree += usage.Free
				diskUsed += usage.Used
				diskTotal += usage.Total
			}
			w.metricObj.DiskFree.Set(math.Floor(float64(diskFree >> 20)))
			w.metricObj.DiskUsed.Set(math.Ceil(float64(diskUsed >> 20)))
			w.metricObj.DiskTotal.Set(math.Floor(float64(diskTotal >> 20)))
			if diskTotal > 0 {
				w.metricObj.DiskUsedPercent.Set(math.Ceil(float64(diskUsed*10000)/float64(diskTotal)) / 100)
			}

			// network
			stat, err := net.IOCountersWithContext(ctx, true)
			if err != nil {
				w.logger.Errorf("Node Watcher: failed to read net stats, error: %v", err)
				continue
			}

			// report metrics for eth0, TODO: report multiple network interfaces
			phyIntf := map[string]bool{
				"eth0": true,
			}

			for _, s := range stat {
				if _, ok := phyIntf[s.Name]; ok {
					w.metricObj.InterfaceName.Set(s.Name, time.Time{})
					w.metricObj.InterfaceRxBytes.Set(float64(s.BytesRecv))
					w.metricObj.InterfaceTxBytes.Set(float64(s.BytesSent))
					// fill metrics only for one interface
					break
				}
			}
		}
	}
}
