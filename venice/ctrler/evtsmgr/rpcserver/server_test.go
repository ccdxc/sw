// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package rpcserver

import (
	"context"
	"fmt"
	"strings"
	"sync"
	"testing"
	"time"

	"github.com/satori/go.uuid"

	"github.com/pensando/sw/api"
	evtsapi "github.com/pensando/sw/api/generated/events"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/cmd/types/protos"
	"github.com/pensando/sw/venice/ctrler/evtsmgr/alertengine"
	"github.com/pensando/sw/venice/ctrler/evtsmgr/memdb"
	emgrpc "github.com/pensando/sw/venice/ctrler/evtsmgr/rpcserver/protos"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/elastic"
	"github.com/pensando/sw/venice/utils/elastic/mock/server"
	"github.com/pensando/sw/venice/utils/events/recorder"
	"github.com/pensando/sw/venice/utils/log"
	mockresolver "github.com/pensando/sw/venice/utils/resolver/mock"
	"github.com/pensando/sw/venice/utils/rpckit"
	. "github.com/pensando/sw/venice/utils/testutils"
	"github.com/pensando/sw/venice/utils/testutils/policygen"
	"github.com/pensando/sw/venice/utils/testutils/serviceutils"
)

var (
	testServerURL = "localhost:0"
	logConfig     = log.GetDefaultConfig(fmt.Sprintf("%s.%s", globals.EvtsMgr, "test"))
	logger        = log.SetConfig(logConfig)

	// create events recorder
	_, _ = recorder.NewRecorder(&recorder.Config{
		Source:        &evtsapi.EventSource{NodeName: utils.GetHostname(), Component: "evtsmgr_test"},
		EvtTypes:      evtsapi.GetEventTypes(),
		BackupDir:     "/tmp",
		SkipEvtsProxy: true}, logger)
)

// setup helper function creates RPC server and client instances
func setup(t *testing.T) (*mock.ElasticServer, apiserver.Server, *RPCServer, *rpckit.RPCClient, alertengine.Interface, *memdb.MemDb) {
	// create elastic mock server
	ms := mock.NewElasticServer()
	ms.Start()

	// create mock resolver
	mr := mockresolver.New()

	// create API server
	apiServer, apiServerURL, err := serviceutils.StartAPIServer("", t.Name(), logger)
	AssertOk(t, err, "failed to start API server")
	Assert(t, !utils.IsEmpty(apiServerURL), "empty API server URL")

	// update mock resolver
	mr.AddServiceInstance(&types.ServiceInstance{
		TypeMeta: api.TypeMeta{
			Kind: "ServiceInstance",
		},
		ObjectMeta: api.ObjectMeta{
			Name: globals.APIServer,
		},
		Service: globals.APIServer,
		URL:     apiServerURL,
	})

	// create elastic client
	esClient, err := elastic.NewClient(ms.GetElasticURL(), nil, logger)
	AssertOk(t, err, "failed to create elastic client")

	memDb := memdb.NewMemDb()

	// create alert engine
	alertEngine, err := alertengine.NewAlertEngine(memDb, logger, mr)
	AssertOk(t, err, "failed to create alert engine")

	// create grpc server
	evtsRPCServer, err := NewRPCServer(globals.EvtsMgr, testServerURL, esClient, alertEngine, memDb)
	AssertOk(t, err, "failed to create rpc server")
	testServerURL := evtsRPCServer.GetListenURL()

	// create grpc client
	evtsRPCClient, err := rpckit.NewRPCClient(globals.EvtsMgr, testServerURL)
	AssertOk(t, err, "failed to create rpc client")

	return ms, apiServer, evtsRPCServer, evtsRPCClient, alertEngine, memDb
}

// TestEvtsMgrRPCServer tests events manager server
func TestEvtsMgrRPCServer(t *testing.T) {
	mockElasticServer, apiServer, rpcServer, rpcClient, _, _ := setup(t)
	defer apiServer.Stop()
	defer rpcServer.Stop()
	defer rpcClient.ClientConn.Close()
	defer mockElasticServer.Stop()

	client := emgrpc.NewEvtsMgrAPIClient(rpcClient.ClientConn)

	evts := []*evtsapi.Event{
		{
			TypeMeta:   api.TypeMeta{Kind: "event"},
			ObjectMeta: api.ObjectMeta{Name: "evt1", UUID: uuid.NewV4().String(), Tenant: "default"},
			EventAttributes: evtsapi.EventAttributes{
				Severity:  "INFO",
				Type:      "DUMMYEVENT",
				ObjectRef: &api.ObjectRef{Kind: "dummy", Namespace: "default", Name: "d1"},
				Source:    &evtsapi.EventSource{Component: "xxx", NodeName: "yyy"},
				Count:     2,
			},
		},
	}

	ctx := context.Background()

	// send single event
	_, err := client.SendEvents(ctx, &evtsapi.EventList{Items: evts})
	AssertOk(t, err, "failed to send event")

	// send bulk events
	// multiple index operations on a single index ID will result in an overwrite (update)
	evts = []*evtsapi.Event{
		{
			TypeMeta:   api.TypeMeta{Kind: "event"},
			ObjectMeta: api.ObjectMeta{Name: "evt2", UUID: uuid.NewV4().String(), Tenant: "default"},
			EventAttributes: evtsapi.EventAttributes{
				Severity:  "INFO",
				Type:      "DUMMYEVENT",
				ObjectRef: &api.ObjectRef{Kind: "dummy", Namespace: "default", Name: "d2"},
				Source:    &evtsapi.EventSource{Component: "xxx", NodeName: "yyy"},
			},
		},
		{
			TypeMeta:   api.TypeMeta{Kind: "event"},
			ObjectMeta: api.ObjectMeta{Name: "evt3", UUID: uuid.NewV4().String(), Tenant: "default"},
			EventAttributes: evtsapi.EventAttributes{
				Severity:  "CIRITICAL",
				Type:      "DUMMYEVENT",
				ObjectRef: &api.ObjectRef{Kind: "dummy", Namespace: "default", Name: "d3"},
				Source:    &evtsapi.EventSource{Component: "xxx", NodeName: "yyy"},
			},
		},
	}

	// send bulk events
	_, err = client.SendEvents(ctx, &evtsapi.EventList{Items: evts})
	AssertOk(t, err, "failed to bulk events")

	// send empty events list
	_, err = client.SendEvents(ctx, &evtsapi.EventList{})
	AssertOk(t, err, "failed to send event")
}

// TestEvtsMgrRPCServerShutdown tests the graceful shutdown
func TestEvtsMgrRPCServerShutdown(t *testing.T) {
	mockElasticServer, apiServer, rpcServer, rpcClient, _, _ := setup(t)

	go func(mockElasticServer *mock.ElasticServer, apiServer apiserver.Server, rpcServer *RPCServer, rpcClient *rpckit.RPCClient) {
		// buffer to have `Done()` called before the stopping the server
		time.Sleep(60)
		rpcClient.ClientConn.Close()
		mockElasticServer.Stop()
		rpcServer.Stop()
		apiServer.Stop()
	}(mockElasticServer, apiServer, rpcServer, rpcClient)

	log.Info("waiting for the shutdown signal")
	<-rpcServer.Done()
	log.Infof("server stopped, exiting")
	return
}

// TestEvtsMgrRPCServerInstantiation tests the RPC server instantiation cases
func TestEvtsMgrRPCServerInstantiation(t *testing.T) {
	mockElasticServer, apiServer, rpcServer, rpcClient, alertEngine, policyDb := setup(t)
	defer apiServer.Stop()
	defer rpcServer.Stop()
	defer rpcClient.ClientConn.Close()
	defer mockElasticServer.Stop()

	esClient := &elastic.Client{}

	// empty listenURL name
	_, err := NewRPCServer(t.Name(), "", esClient, nil, policyDb)
	Assert(t, err != nil && strings.Contains(err.Error(), "all parameters are required"), "expected failure, RPCServer init succeeded")

	// empty server name
	_, err = NewRPCServer("", testServerURL, esClient, nil, policyDb)
	Assert(t, err != nil && strings.Contains(err.Error(), "all parameters are required"), "expected failure, RPCServer init succeeded")

	// nil elastic client
	_, err = NewRPCServer(t.Name(), testServerURL, nil, nil, policyDb)
	Assert(t, err != nil && strings.Contains(err.Error(), "all parameters are required"), "expected failure, RPCServer init succeeded")

	// nil alert engine
	_, err = NewRPCServer(t.Name(), testServerURL, esClient, nil, policyDb)
	Assert(t, err != nil && strings.Contains(err.Error(), "all parameters are required"), "expected failure, RPCServer init succeeded")

	// empty mem db
	_, err = NewRPCServer(t.Name(), testServerURL, esClient, alertEngine, nil)
	Assert(t, err != nil && strings.Contains(err.Error(), "all parameters are required"), "expected failure, RPCServer init succeeded")

	// invalid listen-url
	_, err = NewRPCServer(t.Name(), "invalid-url", esClient, alertEngine, policyDb)
	Assert(t, err != nil && strings.Contains(err.Error(), "error creating rpc server"), "expected failure, RPCServer init succeeded")
}

// TestEventPolicyWatcher tests the events policy watcher
func TestEventPolicyWatcher(t *testing.T) {
	mockElasticServer, apiServer, rpcServer, rpcClient, _, policyDb := setup(t)
	defer apiServer.Stop()
	defer rpcServer.Stop()
	defer rpcClient.ClientConn.Close()
	defer mockElasticServer.Stop()

	eventPolicyClient := emgrpc.NewEventPolicyAPIClient(rpcClient.ClientConn)
	var wg sync.WaitGroup
	wg.Add(3) // for each client

	// client-1; waits to receive the expected number of policies from the watcher and returns
	epStream1, err := eventPolicyClient.WatchEventPolicy(context.Background(), &api.ObjectMeta{Name: "client-1"})
	AssertOk(t, err, "failed to watch event policy, err: %v", err)
	numPolicies := 15
	go func() {
		defer wg.Done()
		totalPoliciesReceived := 0
		for {
			evt, err := epStream1.Recv()
			if err != nil {
				break
			}
			log.Infof("got event policy watch event {%v}: %v", evt.EventType, evt.Policy)
			totalPoliciesReceived++
			if totalPoliciesReceived == numPolicies {
				return
			}
		}
	}()
	// add few event policies to policy DB
	for i := 0; i < numPolicies; i++ {
		policyDb.AddObject(policygen.CreateEventPolicyObj(globals.DefaultTenant, globals.DefaultNamespace, fmt.Sprintf("ep%d", i),
			monitoring.MonitoringExportFormat_name[int32(monitoring.MonitoringExportFormat_SYSLOG_BSD)], nil, nil))
		time.Sleep(100 * time.Millisecond)
	}

	// client-2; stops on the watch error (due to invalid event policy)
	epStream2, err := eventPolicyClient.WatchEventPolicy(context.Background(), &api.ObjectMeta{Name: "client-2"})
	AssertOk(t, err, "failed to watch event policy, err: %v", err)
	go func() {
		defer wg.Done()
		for {
			_, err := epStream2.Recv()
			if err != nil && strings.Contains(err.Error(), "watch error") {
				return
			}
		}
	}()
	time.Sleep(1 * time.Second) // wait for the watch call to reach server
	// send invalid event policy; watcher should stop with "watch error"
	invalidEp := policygen.CreateEventObj(globals.DefaultTenant, globals.DefaultNamespace, "invalid-ep", "", "", "")
	invalidEp.Kind = "EventPolicy"
	policyDb.AddObject(invalidEp)

	// delete the invalid policy; so that client-3 gets to execute without fail
	policyDb.DeleteObject(invalidEp)

	// client-3; stops on context error
	ctx, cancelFunc := context.WithCancel(context.Background())
	epStream3, err := eventPolicyClient.WatchEventPolicy(ctx, &api.ObjectMeta{Name: "client-3"})
	AssertOk(t, err, "failed to watch event policy, err: %v", err)
	go func() {
		defer wg.Done()
		for {
			_, err := epStream3.Recv()
			if err != nil && strings.Contains(err.Error(), "context canceled") {
				return
			}
		}
	}()
	time.Sleep(time.Second) // wait for the watch call to reach server
	// cancel context
	cancelFunc()
	wg.Wait()

	//client-4; once the server is stopped, watcher should not receive any more events
	rpcServer.Stop()
	time.Sleep(time.Second)
	_, err = eventPolicyClient.WatchEventPolicy(context.Background(), &api.ObjectMeta{Name: "client-4"})
	Assert(t, err != nil && (strings.Contains(err.Error(), "transport is closing") ||
		strings.Contains(err.Error(), "the connection is unavailable")), "RPC server failed to stop, err: %v", err)
}
