// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package rpcserver

import (
	"context"
	"fmt"
	"testing"
	"time"

	uuid "github.com/satori/go.uuid"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/events"
	emgrpc "github.com/pensando/sw/venice/ctrler/evtsmgr/rpcserver/evtsmgrproto"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/elastic"
	mock "github.com/pensando/sw/venice/utils/elastic/mock/server"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/rpckit"
	tu "github.com/pensando/sw/venice/utils/testutils"
)

var (
	testServerURL = "localhost:0"
	logConfig     = log.GetDefaultConfig(fmt.Sprintf("%s.%s", globals.EvtsMgr, "test"))
	logger        = log.SetConfig(logConfig)
)

// setup helper function creates RPC server and client instances
func setup(t *testing.T) (*mock.ElasticServer, *RPCServer, *rpckit.RPCClient) {
	// create elastic mock server
	ms := mock.NewElasticServer()
	ms.Start()

	// create elastic client
	esClient, err := elastic.NewClient(ms.GetElasticURL(), logger)
	tu.AssertOk(t, err, "failed to create elastic client")

	// create grpc server
	rpcServer, err := NewRPCServer(globals.EvtsMgr, testServerURL, esClient)
	tu.AssertOk(t, err, "failed to create rpc server")
	testServerURL := rpcServer.GetListenURL()

	// create grpc client
	rpcClient, err := rpckit.NewRPCClient(globals.EvtsMgr, testServerURL)

	tu.AssertOk(t, err, "failed to create rpc client")

	return ms, rpcServer, rpcClient
}

// TestEvtsMgrRPCServer tests events manager server
func TestEvtsMgrRPCServer(t *testing.T) {
	mockElasticServer, rpcServer, rpcClient := setup(t)
	defer rpcServer.Stop()
	defer rpcClient.ClientConn.Close()
	defer mockElasticServer.Stop()

	client := emgrpc.NewEventsAPIClient(rpcClient.ClientConn)

	evts := []events.Event{
		events.Event{
			TypeMeta:   api.TypeMeta{Kind: "event"},
			ObjectMeta: api.ObjectMeta{Name: "evt1", UUID: uuid.NewV4().String(), Tenant: "default"},
			EventAttributes: events.EventAttributes{
				Severity:  "INFO",
				Type:      "DUMMYEVENT",
				ObjectRef: &api.ObjectRef{Kind: "dummy", Namespace: "default", Name: "d1"},
				Source:    &events.EventSource{Component: "xxx", NodeName: "yyy"},
			},
		},
	}

	ctx := context.Background()

	// send single event
	_, err := client.SendEvents(ctx, &emgrpc.EventList{Events: evts})
	tu.AssertOk(t, err, "failed to send event")

	// send bulk events
	// multiple index operations on a single index ID will result in an overwrite (update)
	evts = []events.Event{
		events.Event{
			TypeMeta:   api.TypeMeta{Kind: "event"},
			ObjectMeta: api.ObjectMeta{Name: "evt2", UUID: uuid.NewV4().String(), Tenant: "default"},
			EventAttributes: events.EventAttributes{
				Severity:  "INFO",
				Type:      "DUMMYEVENT",
				ObjectRef: &api.ObjectRef{Kind: "dummy", Namespace: "default", Name: "d2"},
				Source:    &events.EventSource{Component: "xxx", NodeName: "yyy"},
			},
		},
		events.Event{
			TypeMeta:   api.TypeMeta{Kind: "event"},
			ObjectMeta: api.ObjectMeta{Name: "evt3", UUID: uuid.NewV4().String(), Tenant: "default"},
			EventAttributes: events.EventAttributes{
				Severity:  "CIRITICAL",
				Type:      "DUMMYEVENT",
				ObjectRef: &api.ObjectRef{Kind: "dummy", Namespace: "default", Name: "d3"},
				Source:    &events.EventSource{Component: "xxx", NodeName: "yyy"},
			},
		},
	}

	// send bulk events
	_, err = client.SendEvents(ctx, &emgrpc.EventList{Events: evts})
	tu.AssertOk(t, err, "failed to bulk events")

	// send empty events list
	_, err = client.SendEvents(ctx, &emgrpc.EventList{})
	tu.AssertOk(t, err, "failed to send event")

	rpcClient.ClientConn.Close()
	rpcServer.Stop()
}

// TestEvtsMgrRPCServerShutdown tests the graceful shutdown
func TestEvtsMgrRPCServerShutdown(t *testing.T) {
	mockElasticServer, rpcServer, rpcClient := setup(t)

	go func() {
		// buffer to have `Done()` called before the stopping the server
		time.Sleep(60)
		rpcClient.ClientConn.Close()
		mockElasticServer.Stop()
		rpcServer.Stop()
	}()

	log.Info("waiting for the shutdown signal")
	<-rpcServer.Done()
	log.Infof("server stopped, exiting")
	return
}

// TestEvtsMgrRPCServerInstantiation tests the RPC server instantiation cases
func TestEvtsMgrRPCServerInstantiation(t *testing.T) {
	esClient := &elastic.Client{}

	// no listenURL name
	_, err := NewRPCServer("server-name", "", esClient)
	tu.Assert(t, err != nil, "expected failure, RPCServer init succeeded")

	// no server name
	_, err = NewRPCServer("", "listen-rul", esClient)
	tu.Assert(t, err != nil, "expected failure, RPCServer init succeeded")

	// no elastic client
	_, err = NewRPCServer("server-name", "listen-url", nil)
	tu.Assert(t, err != nil, "expected failure, RPCServer init succeeded")
}
