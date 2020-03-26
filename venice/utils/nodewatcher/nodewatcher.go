// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package nodewatcher

import (
	"context"
	"fmt"
	"math"
	"strings"
	"sync"
	"time"

	"github.com/shirou/gopsutil/cpu"
	"github.com/shirou/gopsutil/disk"
	"github.com/shirou/gopsutil/mem"
	"github.com/shirou/gopsutil/net"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/events/generated/eventtypes"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/events/recorder"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
	"github.com/pensando/sw/venice/utils/tsdb"
)

var (
	minFrequency = 10 * time.Second // minimum send interval
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
	InterfaceRxBytes api.Gauge
	InterfaceTxBytes api.Gauge
}

// nodewatcher monitors system resources. It can run on Venice Nodes or on NAPLES.
type nodewatcher struct {
	ctx       context.Context
	cancel    context.CancelFunc
	wg        sync.WaitGroup
	frequency time.Duration
	table     tsdb.Obj
	metricObj *nodeMetrics
	logger    log.Logger
}

// NodeInterface provides functions to manage nodewatcher
type NodeInterface interface {
	Close()
}

// NewNodeWatcher starts a watcher that monitors system resources.
// TSDB must have been initialized with tsdb.Init() before calling this function
func NewNodeWatcher(pctx context.Context, obj runtime.Object, frequency time.Duration, logger log.Logger) (NodeInterface, error) {
	if frequency < minFrequency {
		return nil, fmt.Errorf("minimum frequency is %v, got %v", minFrequency, frequency)
	}

	logger.Infof("Creating new table")
	metricObj := &nodeMetrics{}
	table, err := tsdb.NewVeniceObj(obj, metricObj, nil)
	if err != nil {
		return nil, err
	}

	ctx, cancel := context.WithCancel(pctx)
	w := &nodewatcher{
		ctx:       ctx,
		cancel:    cancel,
		frequency: frequency,
		table:     table,
		metricObj: metricObj,
		logger:    logger,
	}

	if obj.GetObjectKind() == string(cluster.KindDistributedServiceCard) {
		globals.ThresholdEventConfig = false
	}

	w.wg.Add(1)
	go w.periodicUpdate(ctx)
	return w, nil
}

// Close stops node watcher
func (w *nodewatcher) Close() {
	w.cancel()
	w.wg.Wait()
}

// periodically updates the system metrics.
func (w *nodewatcher) periodicUpdate(ctx context.Context) {
	defer w.wg.Done()
	for {
		select {
		case <-ctx.Done():
			w.logger.Info("Metrics watcher context cancelled, exiting")
			return
		case <-time.After(w.frequency):
			// memory
			vmstat, err := mem.VirtualMemory()
			if err != nil || vmstat == nil {
				w.logger.Errorf("Node Watcher: failed to read virtual memory info, error: %v", err)
				continue
			}
			memUsedPercent := math.Floor(vmstat.UsedPercent*100) / 100
			w.metricObj.MemAvailable.Set(float64(vmstat.Available))
			w.metricObj.MemFree.Set(float64(vmstat.Free))
			w.metricObj.MemUsed.Set(float64(vmstat.Used))
			w.metricObj.MemTotal.Set(float64(vmstat.Total))
			w.metricObj.MemUsedPercent.Set(memUsedPercent)

			// cpu
			cpuPercent, err := cpu.Percent(0, false)
			if err != nil || len(cpuPercent) == 0 {
				w.logger.Errorf("Node Watcher: failed to read cpu percent, num-results: %d, error: %v", len(cpuPercent), err)
				continue
			}

			cpuUsedPercent := math.Ceil(cpuPercent[0]*100) / 100
			w.metricObj.CPUUsedPercent.Set(cpuUsedPercent)
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
			w.metricObj.DiskFree.Set(float64(diskFree))
			w.metricObj.DiskUsed.Set(float64(diskUsed))
			w.metricObj.DiskTotal.Set(float64(diskTotal))
			if diskTotal > 0 {
				diskUsedPercent := math.Ceil(float64(diskUsed*10000)/float64(diskTotal)) / 100
				w.metricObj.DiskUsedPercent.Set(diskUsedPercent)

				if globals.ThresholdEventConfig {
					if diskUsedPercent > globals.DiskHighThreshold {
						if !globals.DiskHighThresholdEventStatus {
							recorder.Event(eventtypes.DISK_THRESHOLD_EXCEEDED,
								fmt.Sprintf("%s, current usage: %v%%", globals.DiskHighThresholdMessage, diskUsedPercent), nil)
							globals.DiskHighThresholdEventStatus = true
						}
					} else {
						globals.DiskHighThresholdEventStatus = false
					}
				}
			}

			// network
			stat, err := net.IOCountersWithContext(ctx, true)
			if err != nil {
				w.logger.Errorf("Node Watcher: failed to read net stats, error: %v", err)
				continue
			}

			// aggregate traffic across all eth... and en... network interfaces
			var byteRecv uint64
			var byteSent uint64

			for _, s := range stat {
				if strings.HasPrefix(s.Name, "eth") || strings.HasPrefix(s.Name, "en") {
					byteRecv += s.BytesRecv
					byteSent += s.BytesSent
				}
			}
			w.metricObj.InterfaceRxBytes.Set(float64(byteRecv))
			w.metricObj.InterfaceTxBytes.Set(float64(byteSent))
		}
	}
}
