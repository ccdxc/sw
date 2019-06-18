// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package services

import (
	"testing"
	"time"

	"github.com/pensando/sw/venice/citadel/collector/rpcserver/metric"
	"github.com/pensando/sw/venice/utils/nodewatcher"
	"github.com/pensando/sw/venice/utils/rpckit"
	. "github.com/pensando/sw/venice/utils/testutils"
	"github.com/pensando/sw/venice/utils/tsdb"
	tmock "github.com/pensando/sw/venice/utils/tsdb/mock"
)

var (
	testSendInterval = 500 * time.Millisecond
)

func TestMetricsService(t *testing.T) {
	tsdb.Cleanup()
	MetricsSendInterval = testSendInterval
	dbName = tmock.DbName
	nodewatcher.SetMinimumFrequency(testSendInterval)

	ms := NewMetricsService("testNode", "testCluster", nil) // no resolver
	Assert(t, !ms.IsRunning(), "MS should not be running before Start() is called")

	srv, err := rpckit.NewRPCServer("fake-collector", ":0", rpckit.WithLoggerEnabled(true))
	AssertOk(t, err, "Error creating collector server")
	collector := &tmock.Collector{}
	metric.RegisterMetricApiServer(srv.GrpcServer, collector)
	srv.Start()

	ms.(*metricsService).tsdbOpts.Collector = srv.GetListenURL()
	err = ms.Start()
	AssertOk(t, err, "Error starting metrics service")
	Assert(t, ms.IsRunning(), "MS should be running after Start() is called")

	time.Sleep(3 * testSendInterval)
	Assert(t, collector.ValidateSendInterval(testSendInterval), "Error validating send interval")

	clusterCounters := map[string]int64{
		"AdmittedNICs":       0,
		"PendingNICs":        1,
		"RejectedNICs":       2,
		"DecommissionedNICs": 3,
		"HealthyNICs":        4,
		"UnhealthyNICs":      5,
		"DisconnectedNICs":   6,
	}
	ms.UpdateCounters(clusterCounters)
	time.Sleep(testSendInterval)

	clusterTags := map[string]string{"Tenant": "default", "Namespace": "default", "Kind": "Cluster", "Name": "testCluster"}
	ok := collector.ValidateCount("test", clusterTags, 1, len(clusterCounters), 0, 0, 0)
	Assert(t, ok, "Cluster metrics validation failed")

	// we need to sleep because collector's Validate* functions do a ClearMetrics() at the end
	time.Sleep(testSendInterval)

	nodeTags := map[string]string{"Tenant": "default", "Namespace": "default", "Kind": "Node", "Name": "testNode"}
	// these numbers must match the nodeMetrics definition in sw/venice/utils/nodewatcher/nodewatcher.go
	numInts := 0
	numFloats := 12
	numBools := 0
	numStrings := 1
	ok = collector.ValidateCount("test", nodeTags, 1, numInts, numFloats, numBools, numStrings)
	Assert(t, ok, "Node metrics validation failed")

	// do a stop/start
	ms.Stop()
	Assert(t, !ms.IsRunning(), "MS should be running after Stop() is called")
	err = ms.Start()
	AssertOk(t, err, "Error restarting metrics service")
	Assert(t, ms.IsRunning(), "MS should be running after Start() is called")

	// After restart everything should work as before
	time.Sleep(3 * testSendInterval)
	Assert(t, collector.ValidateSendInterval(testSendInterval), "Error validating send interval")

	ms.UpdateCounters(clusterCounters)
	time.Sleep(testSendInterval)
	ok = collector.ValidateCount("test", clusterTags, 1, len(clusterCounters), 0, 0, 0)
	Assert(t, ok, "Cluster metrics validation failed")

	time.Sleep(testSendInterval)
	ok = collector.ValidateCount("test", nodeTags, 1, numInts, numFloats, numBools, numStrings)
	Assert(t, ok, "Node metrics validation failed")
}
