// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package rpcserver

import (
	"context"
	"fmt"
	"testing"
	"time"

	uuid "github.com/satori/go.uuid"

	"github.com/pensando/sw/api"
	evtsapi "github.com/pensando/sw/api/generated/events"
	"github.com/pensando/sw/venice/apiserver"
	types "github.com/pensando/sw/venice/cmd/types/protos"
	"github.com/pensando/sw/venice/ctrler/evtsmgr/alertengine"
	"github.com/pensando/sw/venice/ctrler/evtsmgr/memdb"
	emgrpc "github.com/pensando/sw/venice/ctrler/evtsmgr/rpcserver/evtsmgrproto"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/elastic"
	mock "github.com/pensando/sw/venice/utils/elastic/mock/server"
	"github.com/pensando/sw/venice/utils/events/recorder"
	"github.com/pensando/sw/venice/utils/log"
	mockresolver "github.com/pensando/sw/venice/utils/resolver/mock"
	"github.com/pensando/sw/venice/utils/rpckit"
	tu "github.com/pensando/sw/venice/utils/testutils"
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
		SkipEvtsProxy: true})
)

// setup helper function creates RPC server and client instances
func setup(t *testing.T) (*mock.ElasticServer, apiserver.Server, *RPCServer, *rpckit.RPCClient) {
	// create elastic mock server
	ms := mock.NewElasticServer()
	ms.Start()

	// create mock resolver
	mr := mockresolver.New()

	// create API server
	apiServer, apiServerURL, err := serviceutils.StartAPIServer("", t.Name(), logger)
	tu.AssertOk(t, err, "failed to start API server")

	// udpate mock resolver
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
	tu.AssertOk(t, err, "failed to create elastic client")

	// create alert engine
	alertEngine, err := alertengine.NewAlertEngine(memdb.NewMemDb(), logger, mr)
	tu.AssertOk(t, err, "failed to create alert engine")

	// create grpc server
	evtsRPCServer, err := NewRPCServer(globals.EvtsMgr, testServerURL, esClient, alertEngine)
	tu.AssertOk(t, err, "failed to create rpc server")
	testServerURL := evtsRPCServer.GetListenURL()

	// create grpc client
	evtsRPCClient, err := rpckit.NewRPCClient(globals.EvtsMgr, testServerURL)
	tu.AssertOk(t, err, "failed to create rpc client")

	return ms, apiServer, evtsRPCServer, evtsRPCClient
}

// TestEvtsMgrRPCServer tests events manager server
func TestEvtsMgrRPCServer(t *testing.T) {
	mockElasticServer, apiServer, rpcServer, rpcClient := setup(t)
	defer apiServer.Stop()
	defer rpcServer.Stop()
	defer rpcClient.ClientConn.Close()
	defer mockElasticServer.Stop()

	client := emgrpc.NewEvtsMgrAPIClient(rpcClient.ClientConn)

	evts := []*evtsapi.Event{
		&evtsapi.Event{
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
	tu.AssertOk(t, err, "failed to send event")

	// send bulk events
	// multiple index operations on a single index ID will result in an overwrite (update)
	evts = []*evtsapi.Event{
		&evtsapi.Event{
			TypeMeta:   api.TypeMeta{Kind: "event"},
			ObjectMeta: api.ObjectMeta{Name: "evt2", UUID: uuid.NewV4().String(), Tenant: "default"},
			EventAttributes: evtsapi.EventAttributes{
				Severity:  "INFO",
				Type:      "DUMMYEVENT",
				ObjectRef: &api.ObjectRef{Kind: "dummy", Namespace: "default", Name: "d2"},
				Source:    &evtsapi.EventSource{Component: "xxx", NodeName: "yyy"},
			},
		},
		&evtsapi.Event{
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
	tu.AssertOk(t, err, "failed to bulk events")

	// send empty events list
	_, err = client.SendEvents(ctx, &evtsapi.EventList{})
	tu.AssertOk(t, err, "failed to send event")
}

// TestEvtsMgrRPCServerShutdown tests the graceful shutdown
func TestEvtsMgrRPCServerShutdown(t *testing.T) {
	mockElasticServer, apiServer, rpcServer, rpcClient := setup(t)

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
	esClient := &elastic.Client{}

	// no listenURL name
	_, err := NewRPCServer("server-name", "", esClient, nil)
	tu.Assert(t, err != nil, "expected failure, RPCServer init succeeded")

	// no server name
	_, err = NewRPCServer("", "listen-rul", esClient, nil)
	tu.Assert(t, err != nil, "expected failure, RPCServer init succeeded")

	// no elastic client
	_, err = NewRPCServer("server-name", "listen-url", nil, nil)
	tu.Assert(t, err != nil, "expected failure, RPCServer init succeeded")

	// no alert engine
	_, err = NewRPCServer("server-name", "listen-url", esClient, nil)
	tu.Assert(t, err != nil, "expected failure, RPCServer init succeeded")
}
