// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package tlsproviders_test

import (
	"context"
	"crypto/tls"
	"fmt"
	"net"
	"testing"
	"time"

	"github.com/pensando/sw/api"
	certsrv "github.com/pensando/sw/venice/cmd/grpc/server/certificates/mock"
	"github.com/pensando/sw/venice/cmd/grpc/service"
	svc "github.com/pensando/sw/venice/cmd/services/mock"
	"github.com/pensando/sw/venice/cmd/types/protos"
	"github.com/pensando/sw/venice/utils/balancer"
	"github.com/pensando/sw/venice/utils/keymgr"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
	. "github.com/pensando/sw/venice/utils/rpckit/tlsproviders"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestCMDBasedProviderInit(t *testing.T) {
	// NEGATIVE TEST-CASES

	// create a real KeyMgr
	be, err := keymgr.NewDefaultBackend("rpckit_cmd_test")
	AssertOk(t, err, "Error instantiating KeyMgr backend")
	defer be.Close()
	km, err := keymgr.NewKeyMgr(be)
	AssertOk(t, err, "Error instantiating KeyMgr")

	// null CMD Endpoint URL
	_, err = NewCMDBasedProvider("", "ep", km)
	Assert(t, err != nil, "CMDBasedProvider instantiation succceeded while expected to fail")

	// null endpoint ID
	_, err = NewCMDBasedProvider("localhost:0", "", km)
	Assert(t, err != nil, "CMDBasedProvider instantiation succceeded while expected to fail")

	// invalid CMD Endpoint
	_, err = NewCMDBasedProvider("foo", "ep", km)
	Assert(t, err != nil, "CMDBasedProvider instantiation succceeded while expected to fail")

	// unavailable CMD Endpoint
	_, err = NewCMDBasedProvider("localhost:123", "ep", km, WithConnTimeout(20*time.Millisecond), WithConnRetryInterval(20*time.Millisecond))
	Assert(t, err != nil, "CMDBasedProvider instantiation succceeded while expected to fail")

	// good CMD but nil KeyMgr
	srv, err := certsrv.NewCertSrv("localhost:0", "testcerts/testServer.crt", "testcerts/testServer.key", "testcerts/testCA.crt")
	defer srv.Stop()
	AssertOk(t, err, "Error creating CMD controller at localhost:0")
	_, err = NewCMDBasedProvider(srv.GetListenURL(), "ep", nil)
	Assert(t, err != nil, "CMDBasedProvider instantiation succceeded while expected to fail")
}

func TestCMDBasedProviderRPC(t *testing.T) {
	// create a mock CMD endpoint
	srv, err := certsrv.NewCertSrv("localhost:0", "testcerts/testServer.crt", "testcerts/testServer.key", "testcerts/testCA.crt")
	defer srv.Stop()
	AssertOk(t, err, "Error creating CMD controller at localhost:0")

	// create KeyMgr
	be, err := keymgr.NewDefaultBackend("rpckit_cmd_test")
	AssertOk(t, err, "Error instantiating KeyMgr backend")
	defer be.Close()
	km, err := keymgr.NewKeyMgr(be)
	AssertOk(t, err, "Error instantiating KeyMgr")

	// create TLS provider
	tlsProvider, err := NewCMDBasedProvider(srv.GetListenURL(), "test", km)
	AssertOk(t, err, "TLS provider initialization failed")

	// create server
	rpcServer, err := rpckit.NewRPCServer("testServer", "localhost:0", rpckit.WithTLSProvider(tlsProvider))
	AssertOk(t, err, "Error creating test server at localhost:0")
	defer rpcServer.Stop()
	testHandler := rpckit.NewTestRPCHandler("dummy message", "test TLS response")
	rpckit.RegisterTestServer(rpcServer.GrpcServer, testHandler)
	rpcServer.Start()

	// create client
	rpcClient, err := rpckit.NewRPCClient("testClient", rpcServer.GetListenURL(), rpckit.WithTLSProvider(tlsProvider), rpckit.WithTracerEnabled(false), rpckit.WithRemoteServerName("test"))
	defer rpcClient.Close()
	AssertOk(t, err, "Error creating test client")
	testClient := rpckit.NewTestClient(rpcClient.ClientConn)

	// Perform RPC call
	resp, err := testClient.TestRPC(context.Background(), &rpckit.TestReq{ReqMsg: "test TLS request"})
	Assert(t, (testHandler.ReqMsg == "test TLS request"), "Unexpected req msg", testHandler)
	Assert(t, (resp.RespMsg == "test TLS response"), "Unexpected resp msg", resp)
	AssertOk(t, err, "RPC error")
	t.Log("Successfully performed RPC")
}

func TestRPCBalancing(t *testing.T) {
	// start two mock CMD endpoints
	srv1, err := certsrv.NewCertSrv("localhost:0", "testcerts/testServer.crt", "testcerts/testServer.key", "testcerts/testCA.crt")
	defer srv1.Stop()
	AssertOk(t, err, "Error creating CMD controller at localhost:0")
	_, portStr1, err := net.SplitHostPort(srv1.GetListenURL())
	AssertOk(t, err, fmt.Sprintf("Error getting srv1 port from URL: %s", srv1.GetListenURL()))
	AssertOk(t, err, "Failed to convert port")

	srv2, err := certsrv.NewCertSrv("localhost:0", "testcerts/testServer.crt", "testcerts/testServer.key", "testcerts/testCA.crt")
	defer srv2.Stop()
	AssertOk(t, err, "Error creating CMD controller at localhost:0")
	_, portStr2, err := net.SplitHostPort(srv2.GetListenURL())
	AssertOk(t, err, fmt.Sprintf("Error getting srv2 port from URL: %s", srv1.GetListenURL()))
	AssertOk(t, err, "Failed to convert port")

	// Now create a mock resolver
	m := svc.NewResolverService()
	resolverHandler := service.NewRPCHandler(m)
	resolverServer, err := rpckit.NewRPCServer("resolver", "localhost:0")
	AssertOk(t, err, "Failed to create resolver server")
	defer resolverServer.Stop()
	types.RegisterServiceAPIServer(resolverServer.GrpcServer, resolverHandler)
	resolverServer.Start()

	// populate the mock resolver with the two servers
	si1 := types.ServiceInstance{
		TypeMeta: api.TypeMeta{
			Kind: "ServiceInstance",
		},
		ObjectMeta: api.ObjectMeta{
			Name: "t1",
		},
		Service: "certsrv",
		Node:    "localhost",
		URL:     fmt.Sprintf("localhost:%s", portStr1),
	}
	si2 := si1
	si2.Name = "t2"
	si2.URL = fmt.Sprintf("localhost:%s", portStr2)
	m.AddServiceInstance(&si1)
	m.AddServiceInstance(&si2)

	// Now create a rpc client with a balancer
	r := resolver.New(&resolver.Config{Name: "certsrv", Servers: []string{resolverServer.GetListenURL()}})
	b := balancer.New(r)

	// create KeyMgr
	be, err := keymgr.NewDefaultBackend("rpckit_cmd_test")
	AssertOk(t, err, "Error instantiating KeyMgr backend")
	defer be.Close()
	km, err := keymgr.NewKeyMgr(be)
	AssertOk(t, err, "Error instantiating KeyMgr")

	// create TLS provider
	tlsProvider, err := NewCMDBasedProvider("certsrv", "test", km, WithBalancer(b))
	AssertOk(t, err, "Error instantiating CMDBasedProvider")
	defer tlsProvider.Close()

	// Wait until initial connections are down
	AssertEventually(t, func() (bool, interface{}) {
		return srv1.GetActiveConnCount() == 0 && srv2.GetActiveConnCount() == 0, nil
	}, fmt.Sprintf("Unexpected up connections: SRV1: %v, SRV2: %v", srv1.GetActiveConnCount(), srv2.GetActiveConnCount()))

	// Clear the counters for the 2 RPCs that were triggered by NewCMDBasedProvider
	srv1.ClearRPCCounts()
	srv2.ClearRPCCounts()

	// getServerCertificate() is the callback that is invoked by the server to get a certificate
	// to present to the client during the TLS handshake.
	// Directly invoke getServerCertificate() and check that the requests are load-balanced.
	numRPCs := uint64(20)
	for i := uint64(0); i < numRPCs; i++ {
		_, err = tlsProvider.GetServerCertificate(&tls.ClientHelloInfo{ServerName: fmt.Sprintf("Hello-%d", i)})
		AssertOk(t, err, "Error getting certificate from CMD")
		// Verify that connections to both backends are closed
		AssertEventually(t, func() (bool, interface{}) {
			return srv1.GetActiveConnCount() == 0 && srv2.GetActiveConnCount() == 0, nil
		}, fmt.Sprintf("Unexpected up backend connections: SRV1: %v, SRV2: %v", srv1.GetActiveConnCount(), srv2.GetActiveConnCount()))
	}

	// Connections to certsrv are established/torn down before/after each RPC.
	// Even if we use the same balancer for all RPCs, the distribution of the RPCs depends on the timing of connection
	// establishment, so it will not be exactly 50% - %50%.
	srv1RPCCount := srv1.GetRPCSuccessCount()
	srv2RPCCount := srv2.GetRPCSuccessCount()
	Assert(t, (srv1RPCCount >= 2) && (srv2RPCCount >= 2) && (srv1RPCCount+srv2RPCCount == numRPCs),
		fmt.Sprintf("Unexpected number of RPC calls. SRV1: %d SRV2: %d", srv1RPCCount, srv2RPCCount))

	srv1ConnCount := srv1.GetCompletedConnCount()
	srv2ConnCount := srv2.GetCompletedConnCount()
	Assert(t, srv1ConnCount >= 2, fmt.Sprintf("SRV1 got only %d connections", srv1ConnCount))
	Assert(t, srv2ConnCount >= 2, fmt.Sprintf("SRV2 got only %d connections", srv2ConnCount))
}
