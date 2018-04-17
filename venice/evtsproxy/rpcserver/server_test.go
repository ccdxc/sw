// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package rpcserver

import (
	"context"
	"testing"
	"time"

	"github.com/pensando/sw/api/generated/events"
	epgrpc "github.com/pensando/sw/venice/evtsproxy/rpcserver/evtsproxyproto"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/rpckit"
	tu "github.com/pensando/sw/venice/utils/testutils"
)

var (
	testServerURL = "localhost:0"

	logger = log.GetNewLogger(log.GetDefaultConfig("evtsproxy-server-test"))
)

// setup helper function to create RPC server and client instances
func setup(t *testing.T) (*RPCServer, *rpckit.RPCClient) {
	// create grpc server
	rpcServer, err := NewRPCServer(globals.EvtsProxy, testServerURL, logger)
	tu.AssertOk(t, err, "failed to create rpc server")
	testServerURL := rpcServer.GetListenURL()

	// create grpc client
	rpcClient, err := rpckit.NewRPCClient(globals.EvtsProxy, testServerURL)

	tu.AssertOk(t, err, "failed to create rpc client")

	return rpcServer, rpcClient
}

// TestEventsProxyRPCServer tests events proxy server
func TestEventsProxyRPCServer(t *testing.T) {
	rpcServer, rpcClient := setup(t)
	defer rpcServer.Stop()
	defer rpcClient.ClientConn.Close()

	// create events proxy client
	proxyClient := epgrpc.NewEventsProxyAPIClient(rpcClient.ClientConn)
	tu.Assert(t, proxyClient != nil, "failed to created events proxy client")

	ctx := context.Background()
	_, err := proxyClient.ForwardEvent(ctx, &events.Event{})
	tu.AssertOk(t, err, "failed to forward event")

	_, err = proxyClient.ForwardEvents(ctx, &events.EventList{})
	tu.AssertOk(t, err, "failed to forward events")

	// TODO test dispatcher and writers
}

// TestEventsProxyRPCServerShutdown tests the graceful shutdown
func TestEventsProxyRPCServerShutdown(t *testing.T) {
	rpcServer, rpcClient := setup(t)

	go func() {
		// buffer to have `Done()` called before the stopping the server
		time.Sleep(60)
		rpcClient.ClientConn.Close()
		rpcServer.Stop()
	}()

	log.Info("waiting for the shutdown signal")
	<-rpcServer.Done()
	log.Infof("server stopped, exiting")
	return
}

// TestEventsProxyRPCServerInstantiation tests the RPC server instantiation cases
func TestEventsProxyRPCServerInstantiation(t *testing.T) {
	// no listenURL name
	_, err := NewRPCServer("server-name", "", logger)
	tu.Assert(t, err != nil, "expected failure, RPCServer init succeeded")

	// no server name
	_, err = NewRPCServer("", "listen-rul", logger)
	tu.Assert(t, err != nil, "expected failure, RPCServer init succeeded")

	// no server name and listen URL
	_, err = NewRPCServer("", "", logger)
	tu.Assert(t, err != nil, "expected failure, RPCServer init succeeded")
}
