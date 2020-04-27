// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package services

import (
	"strings"
	"testing"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/citadel/collector/rpcserver/metric"
	"github.com/pensando/sw/venice/cmd/types/protos"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/nodemetrics"
	mr "github.com/pensando/sw/venice/utils/resolver/mock"
	"github.com/pensando/sw/venice/utils/rpckit"
	. "github.com/pensando/sw/venice/utils/testutils"
	"github.com/pensando/sw/venice/utils/tsdb"
	tmock "github.com/pensando/sw/venice/utils/tsdb/mock"
)

var (
	testSendInterval = 300 * time.Millisecond
)

func TestMetricsService(t *testing.T) {
	tsdb.Cleanup()
	MetricsSendInterval = testSendInterval
	dbName = tmock.DbName
	nodemetrics.SetMinimumFrequency(testSendInterval)

	srv, err := rpckit.NewRPCServer(globals.Collector, ":0", rpckit.WithLoggerEnabled(true))
	AssertOk(t, err, "Error creating collector server")
	collector := &tmock.Collector{}
	metric.RegisterMetricApiServer(srv.GrpcServer, collector)
	srv.Start()
	v := strings.Split(srv.GetListenURL(), ":")
	mc := mr.New()

	mc.AddServiceInstance(&types.ServiceInstance{
		TypeMeta: api.TypeMeta{
			Kind: "ServiceInstance",
		},
		ObjectMeta: api.ObjectMeta{
			Name: globals.Collector,
		},
		Service: globals.Collector,
		URL:     "localhost:" + v[len(v)-1],
	})

	ms := NewMetricsService("testNode", "testCluster", mc) // no resolver
	Assert(t, !ms.IsRunning(), "MS should not be running before Start() is called")

	err = ms.Start()
	AssertOk(t, err, "Error starting metrics service")
	Assert(t, ms.IsRunning(), "MS should be running after Start() is called")

	// ValidateSendInterval requires at least 2 successful writes to report meaningful results.
	// This may actually take more than 2 send intervals if the machine is busy.
	time.Sleep(5 * testSendInterval)
	Assert(t, collector.ValidateSendInterval(testSendInterval), "Error validating send interval")

	// clusterCounters are gauges, so we collect them as int64 but report them as float64
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
	time.Sleep(2 * testSendInterval)

	clusterTags := map[string]string{"Tenant": "default", "Namespace": "default", "Kind": "Cluster", "Name": "testCluster"}
	ok := collector.ValidateCount("test", clusterTags, 1, 0, len(clusterCounters), 0, 0)
	Assert(t, ok, "Cluster metrics validation failed")

	// we need to sleep because collector's Validate* functions do a ClearMetrics() at the end
	time.Sleep(2 * testSendInterval)

	nodeTags := map[string]string{"Tenant": "default", "Namespace": "default", "Kind": "Node", "Name": "testNode"}
	// these numbers must match the nodeMetrics definition in sw/venice/utils/nodemetrics/nodemetrics.go
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
	time.Sleep(5 * testSendInterval)
	Assert(t, collector.ValidateSendInterval(testSendInterval), "Error validating send interval")

	ms.UpdateCounters(clusterCounters)
	time.Sleep(2 * testSendInterval)
	ok = collector.ValidateCount("test", clusterTags, 1, 0, len(clusterCounters), 0, 0)
	Assert(t, ok, "Cluster metrics validation failed")

	time.Sleep(2 * testSendInterval)
	ok = collector.ValidateCount("test", nodeTags, 1, numInts, numFloats, numBools, numStrings)
	Assert(t, ok, "Node metrics validation failed")
}
