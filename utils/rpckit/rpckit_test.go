// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package rpckit_test

import (
	"errors"
	"fmt"
	"net"
	"strconv"
	"testing"
	"time"

	"github.com/pensando/sw/utils/log"
	"github.com/pensando/sw/utils/rpckit/tlsproviders"
	"golang.org/x/net/context"
	"google.golang.org/grpc"
	"google.golang.org/grpc/credentials"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/cmd/grpc/service"
	"github.com/pensando/sw/cmd/services/mock"
	"github.com/pensando/sw/cmd/types"
	"github.com/pensando/sw/utils/balancer"
	"github.com/pensando/sw/utils/resolver"
	. "github.com/pensando/sw/utils/rpckit"
	. "github.com/pensando/sw/utils/testutils"
)

// testRPCHandler is the grpc handler
type testRPCHandler struct {
	reqMsg  string
	respMsg string
}

// test rpc call handler
func (tst *testRPCHandler) TestRPC(ctx context.Context, req *TestReq) (*TestResp, error) {
	log.Infof("Test rpc handler got request: %+v", req)

	// save the request
	tst.reqMsg = req.ReqMsg

	// response message
	testResp := &TestResp{
		RespMsg: tst.respMsg,
	}

	return testResp, nil
}

// test err rpc call handler
func (tst *testRPCHandler) TestRPCErr(ctx context.Context, req *TestReq) (*TestResp, error) {
	return nil, errors.New("Test RPC Error response")
}

// newRPCServerClient create an RPC server and client
func newRPCServerClient(t *testing.T) (*RPCServer, *RPCClient) {
	// create an RPC server
	rpcServer, err := NewRPCServer("grpc.local", "localhost:0", WithTracerEnabled(true))
	if err != nil {
		t.Fatalf("Failed to create listener, error: %v", err)
	}

	// create an RPC client
	rpcClient, err := NewRPCClient("grpc.local", rpcServer.GetListenURL(), WithTracerEnabled(true))
	if err != nil {
		t.Fatalf("Failed to create client with error: %v", err)
	}

	return rpcServer, rpcClient
}

// stopRPCServerClient closes connections and stops RPC server
func stopRPCServerClient(rpcServer *RPCServer, rpcClient *RPCClient) {
	rpcClient.Close()
	rpcServer.Stop()
	time.Sleep(time.Millisecond)
}

// test basic RPC
func TestRPCBasic(t *testing.T) {
	// create an rpc handler object
	testHandler := &testRPCHandler{
		reqMsg:  "dummy message",
		respMsg: "test response",
	}

	// create an RPC server and client
	rpcServer, rpcClient := newRPCServerClient(t)

	// close client connection and stop the server
	defer stopRPCServerClient(rpcServer, rpcClient)

	// register the handlers
	RegisterTestServer(rpcServer.GrpcServer, testHandler)
	testClient := NewTestClient(rpcClient.ClientConn)

	// make sure server with no URL or client with no URL fails
	_, err := NewRPCServer("grpc.local", "")
	Assert(t, (err != nil), "server with no URL succeded while expecting it to fail")
	_, err = NewRPCClient("grpc.local", "")
	Assert(t, (err != nil), "client with no URL succeded while expecting it to fail")
	_, err = NewRPCServer("grpc.local", "google.com")
	Assert(t, (err != nil), "server with no URL succeded while expecting it to fail")

	// make an RPC call
	resp, err := testClient.TestRPC(context.Background(), &TestReq{ReqMsg: "test request"})
	AssertOk(t, err, "RPC error")
	Assert(t, (testHandler.reqMsg == "test request"), "Unexpected req msg", testHandler)
	Assert(t, (resp.RespMsg == "test response"), "Unexpected resp msg", resp)

	// make test err rpc and confirm we get an error
	_, err = testClient.TestRPCErr(context.Background(), &TestReq{ReqMsg: "test request"})
	Assert(t, (err != nil), "TestRPCErr succeded while expecting it to fail")

	// verify RPC stats got incremented
	serverStats := rpcServer.GetRPCStats()
	Assert(t, (serverStats["Server-/rpckit.Test/TestRPC:Req"] == 1), "Unexpected RPC stats", serverStats)
	Assert(t, (serverStats["Server-/rpckit.Test/TestRPC:Resp"] == 1), "Unexpected RPC stats", serverStats)
	Assert(t, (serverStats["Server-/rpckit.Test/TestRPCErr:Resp:Error"] == 1), "Unexpected RPC stats", serverStats)

	clientStats := rpcClient.GetRPCStats()
	Assert(t, (clientStats["Client-/rpckit.Test/TestRPC:Req"] == 1), "Unexpected RPC stats", clientStats)
	Assert(t, (clientStats["Client-/rpckit.Test/TestRPC:Resp"] == 1), "Unexpected RPC stats", clientStats)
	Assert(t, (clientStats["Client-/rpckit.Test/TestRPCErr:Resp:Error"] == 1), "Unexpected RPC stats", clientStats)
}

// test rpc client connection closing and reconnecting
func TestRPCClientReconnect(t *testing.T) {
	// create an rpc handler object
	testHandler := &testRPCHandler{
		reqMsg:  "dummy message",
		respMsg: "test response",
	}

	// create an RPC server and
	rpcServer, rpcClient := newRPCServerClient(t)

	// close client connection and stop the server
	defer stopRPCServerClient(rpcServer, rpcClient)

	// register the handlers
	RegisterTestServer(rpcServer.GrpcServer, testHandler)
	testClient := NewTestClient(rpcClient.ClientConn)

	// make an RPC call
	_, err := testClient.TestRPC(context.Background(), &TestReq{ReqMsg: "test request"})
	AssertOk(t, err, "RPC error")

	// close the client connection
	t.Logf("Disconnecting rpc client connection")
	rpcClient.Close()

	// make an RPC call on closed connection
	_, err = testClient.TestRPC(context.Background(), &TestReq{ReqMsg: "test request"})
	Assert(t, (err != nil), "RPC succeded while expecting it to fail")

	// reconnect the client
	t.Logf("Reconnecting to RPC server")
	err = rpcClient.Reconnect()
	AssertOk(t, err, "RPC reconnect error")

	// make sure an RPC call succeeds after reconnecting
	testClient = NewTestClient(rpcClient.ClientConn)
	_, err = testClient.TestRPC(context.Background(), &TestReq{ReqMsg: "test request"}, grpc.FailFast(false))
	AssertOk(t, err, "RPC error")

	// verify RPC stats got incremented
	serverStats := rpcServer.GetRPCStats()
	Assert(t, (serverStats["Server-/rpckit.Test/TestRPC:Req"] == 2), "Unexpected RPC stats", serverStats)
	Assert(t, (serverStats["Server-/rpckit.Test/TestRPC:Resp"] == 2), "Unexpected RPC stats", serverStats)
	Assert(t, (serverStats["Server-/rpckit.Test/TestRPC:Resp:Error"] == 0), "Unexpected RPC stats", serverStats)

	clientStats := rpcClient.GetRPCStats()
	Assert(t, (clientStats["Client-/rpckit.Test/TestRPC:Req"] == 3), "Unexpected RPC stats", clientStats)
	Assert(t, (clientStats["Client-/rpckit.Test/TestRPC:Resp"] == 3), "Unexpected RPC stats", clientStats)
	Assert(t, (clientStats["Client-/rpckit.Test/TestRPC:Resp:Error"] == 1), "Unexpected RPC stats", clientStats)
}

// test RPC server restart
func TestRPCServerRestart(t *testing.T) {
	// create an rpc handler object
	testHandler := &testRPCHandler{
		reqMsg:  "dummy message",
		respMsg: "test response",
	}

	// create an RPC server and
	rpcServer, rpcClient := newRPCServerClient(t)

	// close client connection and stop the server
	defer stopRPCServerClient(rpcServer, rpcClient)

	// register the handlers
	RegisterTestServer(rpcServer.GrpcServer, testHandler)
	testClient := NewTestClient(rpcClient.ClientConn)

	// make an RPC call
	_, err := testClient.TestRPC(context.Background(), &TestReq{ReqMsg: "test request"})
	AssertOk(t, err, "RPC error")

	// stop the old server and wait for a while to make sure connectin closes
	t.Logf("Stopping RPC server")
	rpcServer.Stop()
	time.Sleep(time.Millisecond * 100)

	// make an RPC call on closed connection make sure it fails
	_, err = testClient.TestRPC(context.Background(), &TestReq{ReqMsg: "test request"})
	Assert(t, (err != nil), "RPC succeded while expecting it to fail")

	// start a new server
	t.Logf("Restarting RPC server")
	rpcServer, err = NewRPCServer("grpc.local", rpcServer.GetListenURL())
	AssertOk(t, err, "RPC server restart error")
	RegisterTestServer(rpcServer.GrpcServer, testHandler)
	defer func() {
		rpcServer.Stop()
		time.Sleep(time.Millisecond)
	}()

	// make an RPC call succeeds with new server
	_, err = testClient.TestRPC(context.Background(), &TestReq{ReqMsg: "test request"}, grpc.FailFast(false))
	AssertOk(t, err, "RPC error")

	// verify RPC stats got incremented
	serverStats := rpcServer.GetRPCStats()
	Assert(t, (serverStats["Server-/rpckit.Test/TestRPC:Req"] == 1), "Unexpected RPC stats", serverStats)
	Assert(t, (serverStats["Server-/rpckit.Test/TestRPC:Resp"] == 1), "Unexpected RPC stats", serverStats)
	Assert(t, (serverStats["Server-/rpckit.Test/TestRPC:Resp:Error"] == 0), "Unexpected RPC stats", serverStats)

	clientStats := rpcClient.GetRPCStats()
	Assert(t, (clientStats["Client-/rpckit.Test/TestRPC:Req"] == 3), "Unexpected RPC stats", clientStats)
	Assert(t, (clientStats["Client-/rpckit.Test/TestRPC:Resp"] == 3), "Unexpected RPC stats", clientStats)
	Assert(t, (clientStats["Client-/rpckit.Test/TestRPC:Resp:Error"] == 1), "Unexpected RPC stats", clientStats)
}

// test multiple RPC handlers on same server
func TestRPCMultipleHandlers(t *testing.T) {
	// create an rpc handler object
	testHandler := &testRPCHandler{
		reqMsg:  "dummy message",
		respMsg: "test response",
	}

	// create another rpc handler object
	test2Handler := &testRPCHandler{
		reqMsg:  "dummy message",
		respMsg: "test2 response",
	}

	// create an RPC server and
	rpcServer, rpcClient := newRPCServerClient(t)

	// close client connection and stop the server
	defer stopRPCServerClient(rpcServer, rpcClient)

	// register the handlers
	RegisterTestServer(rpcServer.GrpcServer, testHandler)
	testClient := NewTestClient(rpcClient.ClientConn)
	RegisterTest2Server(rpcServer.GrpcServer, test2Handler)
	test2Client := NewTest2Client(rpcClient.ClientConn)

	// make an RPC call
	resp, err := testClient.TestRPC(context.Background(), &TestReq{ReqMsg: "test request"})
	AssertOk(t, err, "RPC error")
	Assert(t, (testHandler.reqMsg == "test request"), "Unexpected req msg", testHandler)
	Assert(t, (resp.RespMsg == "test response"), "Unexpected resp msg", resp)

	// make RPC call to second handler
	resp, err = test2Client.TestRPC(context.Background(), &TestReq{ReqMsg: "test2 request"})
	AssertOk(t, err, "RPC error")
	Assert(t, (test2Handler.reqMsg == "test2 request"), "Unexpected req msg", testHandler)
	Assert(t, (resp.RespMsg == "test2 response"), "Unexpected resp msg", resp)
}

// test middleware for testing middleware callbacks
type testMiddleware struct {
	rpcStats map[string]int64
}

// ReqInterceptor implements request interception
func (s *testMiddleware) ReqInterceptor(ctx context.Context, role, srvName, method string, req interface{}) context.Context {
	s.rpcStats[fmt.Sprintf("%s-%s:Req", role, method)]++

	return ctx
}

// RespInterceptor handles responses
func (s *testMiddleware) RespInterceptor(ctx context.Context, role, srvName, method string, req, reply interface{}, err error) context.Context {
	s.rpcStats[fmt.Sprintf("%s-%s:Resp", role, method)]++

	return ctx
}

// test RPC server restart
func TestRPCMiddlewares(t *testing.T) {
	// create an rpc handler object
	testHandler := &testRPCHandler{
		reqMsg:  "dummy message",
		respMsg: "test response",
	}

	tstmdl := &testMiddleware{
		rpcStats: make(map[string]int64),
	}

	// create an RPC server
	rpcServer, err := NewRPCServer("grpc.local", ":9900", WithMiddleware(tstmdl))
	AssertOk(t, err, "Error creating RPC server")

	// create an RPC client
	rpcClient, err := NewRPCClient("grpc.local", "localhost:9900", WithMiddleware(tstmdl))
	AssertOk(t, err, "Error creating RPC client")

	// close client connection and stop the server
	defer stopRPCServerClient(rpcServer, rpcClient)

	// register the handlers
	RegisterTestServer(rpcServer.GrpcServer, testHandler)
	testClient := NewTestClient(rpcClient.ClientConn)

	// make an RPC call
	_, err = testClient.TestRPC(context.Background(), &TestReq{ReqMsg: "test request"})
	AssertOk(t, err, "RPC error")

	// verify test middleware got called by checking the stats
	Assert(t, (tstmdl.rpcStats["Server-/rpckit.Test/TestRPC:Req"] == 1), "Unexpected middleware stats", tstmdl)
	Assert(t, (tstmdl.rpcStats["Server-/rpckit.Test/TestRPC:Resp"] == 1), "Unexpected middleware stats", tstmdl)
	Assert(t, (tstmdl.rpcStats["Client-/rpckit.Test/TestRPC:Req"] == 1), "Unexpected middleware stats", tstmdl)
	Assert(t, (tstmdl.rpcStats["Client-/rpckit.Test/TestRPC:Resp"] == 1), "Unexpected middleware stats", tstmdl)
}

// Test TLS certs
func TestRPCTlsConnections(t *testing.T) {
	tlsURL := "localhost:9990"
	nontlsURL := "localhost:9900"

	// create tls and non-tls rpc handler object
	testHandler := &testRPCHandler{
		reqMsg:  "dummy message",
		respMsg: "test response",
	}
	tlsTestHandler := &testRPCHandler{
		reqMsg:  "dummy message",
		respMsg: "test TLS response",
	}

	tlsProvider, err := tlsproviders.NewFileBasedProvider(
		"tlsproviders/testcerts/testServer.crt",
		"tlsproviders/testcerts/testServer.key",
		"tlsproviders/testcerts/testCA.crt")
	AssertOk(t, err, "Failed to instantiate TLS provider")

	// create tls & non-tls RPC server
	rpcServer, err := NewRPCServer("grpc.local", nontlsURL)
	AssertOk(t, err, "Error creating non-TLS RPC server")
	tlsRPCServer, err := NewRPCServer("grpc.local", tlsURL, WithTLSProvider(tlsProvider))
	AssertOk(t, err, "Error creating TLS RPC server")

	// create tls & non-tls RPC client
	rpcClient, err := NewRPCClient("grpc.local", nontlsURL)
	AssertOk(t, err, "Error creating non-TLS RPC client")
	tlsRPCClient, err := NewRPCClient("grpc.local", tlsURL, WithTLSProvider(tlsProvider))
	AssertOk(t, err, "Error creating TLS RPC client")

	// close client connection and stop the server
	defer stopRPCServerClient(rpcServer, rpcClient)
	defer stopRPCServerClient(tlsRPCServer, tlsRPCClient)

	// make sure non-tls client can not connect to tls server
	// Note: grpc non-tls clients dont block till certification validation is done.
	//       So, we need to make a dummy rpc call and confirm we are not able to connect
	t.Logf("non-TLS client trying to connect to TLS server")
	nontlsRPCClient, err := NewRPCClient("grpc.local", tlsURL)
	AssertOk(t, err, "Error creating non-TLS RPC client")
	nontlsTestClient := NewTestClient(nontlsRPCClient.ClientConn)
	_, err = nontlsTestClient.TestRPC(context.Background(), &TestReq{ReqMsg: "test request"})
	Assert(t, (err != nil), "non-tls client was able to connect to tls-server")

	// make sure a client with empty credentials can not connect to tls server
	t.Logf("Empty TLS client trying to connect to TLS server")
	_, err = grpc.Dial(tlsURL, grpc.WithBlock(), grpc.WithTimeout(time.Second*3),
		grpc.WithTransportCredentials(credentials.NewClientTLSFromCert(nil, "")))
	Assert(t, (err != nil), "client with empty tls cert was able to connect to tls-server")

	// make sure tls client will not connect to non-tls server
	t.Logf("TLS client trying to connect to non-TLS server")
	_, err = NewRPCClient("grpc.local", nontlsURL, WithTLSProvider(tlsProvider))
	Assert(t, (err != nil), "tls client was able to connect to non-tls server")

	// verify client and servers with different certs will not connect
	tlsProvider2, err := tlsproviders.NewFileBasedProvider(
		"tlsproviders/testcerts/test2Server.crt",
		"tlsproviders/testcerts/test2Server.key",
		"tlsproviders/testcerts/test2CA.crt")
	AssertOk(t, err, "Failed to instantiate TLS provider")

	t.Logf("TLS client with incorrect certificate trying to connect to TLS server")
	_, err = NewRPCClient("grpc.local", tlsURL, WithTLSProvider(tlsProvider2))
	Assert(t, (err != nil), "tls client was able to connect to non-tls server")

	// verify client and servers with different root CA will not connect
	tlsProvider3, err := tlsproviders.NewFileBasedProvider(
		"tlsproviders/testcerts/testClient.crt",
		"tlsproviders/testcerts/testClient.key",
		"tlsproviders/testcerts/test2CA.crt")
	AssertOk(t, err, "Failed to instantiate TLS provider")

	t.Logf("TLS client with incorrect certificate trying to connect to TLS server")
	_, err = NewRPCClient("grpc.local", tlsURL, WithTLSProvider(tlsProvider3))
	Assert(t, (err != nil), "tls client was able to connect to non-tls server")

	// register the handlers
	RegisterTestServer(rpcServer.GrpcServer, testHandler)
	testClient := NewTestClient(rpcClient.ClientConn)
	RegisterTestServer(tlsRPCServer.GrpcServer, tlsTestHandler)
	tlsTestClient := NewTestClient(tlsRPCClient.ClientConn)

	// make an RPC call
	_, err = testClient.TestRPC(context.Background(), &TestReq{ReqMsg: "test request"})
	AssertOk(t, err, "RPC error")
	resp, err := tlsTestClient.TestRPC(context.Background(), &TestReq{ReqMsg: "test TLS request"})
	AssertOk(t, err, "RPC error")
	Assert(t, (tlsTestHandler.reqMsg == "test TLS request"), "Unexpected req msg", tlsTestHandler)
	Assert(t, (resp.RespMsg == "test TLS response"), "Unexpected resp msg", resp)

	// verify reconnect with TLS succeeds
	tlsRPCClient.Close()
	err = tlsRPCClient.Reconnect()
	AssertOk(t, err, "RPC reconnect error")
	tlsTestClient = NewTestClient(tlsRPCClient.ClientConn)
	_, err = tlsTestClient.TestRPC(context.Background(), &TestReq{ReqMsg: "test TLS request"})
	AssertOk(t, err, "RPC error")
}

// test load balancing of RPCs
func TestRPCBalancing(t *testing.T) {
	// create two handler objects
	testHandler1 := &testRPCHandler{
		respMsg: "t1",
	}
	testHandler2 := &testRPCHandler{
		respMsg: "t2",
	}

	// create two rpc servers
	rpcServer1, err := NewRPCServer("testService", "localhost:0", WithTracerEnabled(true))
	AssertOk(t, err, "Failed to create listener")
	defer func() {
		rpcServer1.Stop()
		time.Sleep(time.Millisecond)
	}()
	_, portStr1, err := net.SplitHostPort(rpcServer1.GetListenURL())
	AssertOk(t, err, "Failed to parse port")
	port1, err := strconv.Atoi(portStr1)
	AssertOk(t, err, "Failed to convert port")

	rpcServer2, err := NewRPCServer("testService", "localhost:0", WithTracerEnabled(true))
	AssertOk(t, err, "Failed to create listener")
	defer func() {
		rpcServer2.Stop()
		time.Sleep(time.Millisecond)
	}()
	_, portStr2, err := net.SplitHostPort(rpcServer2.GetListenURL())
	AssertOk(t, err, "Failed to parse port")
	port2, err := strconv.Atoi(portStr2)
	AssertOk(t, err, "Failed to convert port")

	// register handlers with rpc servers
	RegisterTestServer(rpcServer1.GrpcServer, testHandler1)
	RegisterTestServer(rpcServer2.GrpcServer, testHandler2)

	// Now create a mock resolver
	m := mock.NewResolverService()
	resolverHandler := service.NewRPCHandler(m)
	resolverServer, err := NewRPCServer("resolver", "localhost:0", WithTracerEnabled(true))
	AssertOk(t, err, "Failed to create listener")
	types.RegisterServiceAPIServer(resolverServer.GrpcServer, resolverHandler)

	// populate the mock resolver with the two servers
	si1 := types.ServiceInstance{
		TypeMeta: api.TypeMeta{
			Kind: "ServiceInstance",
		},
		ObjectMeta: api.ObjectMeta{
			Name: "t1",
		},
		Service: "testService",
		Node:    "localhost",
		URL:     fmt.Sprintf("localhost:%d", port1),
	}
	si2 := si1
	si2.Name = "t2"
	si2.URL = fmt.Sprintf("localhost:%d", port2)
	m.AddServiceInstance(&si1)
	m.AddServiceInstance(&si2)

	// Now create a rpc client with a balancer
	r := resolver.New(&resolver.Config{Servers: []string{resolverServer.GetListenURL()}})
	b := balancer.New(r)
	client, err := NewRPCClient("RPCBalanceTest", "testService", WithBalancer(b))
	AssertOk(t, err, "Failed to create RPC Client")
	testClient := NewTestClient(client.ClientConn)

	// Send a bunch of RPCs
	respMap := make(map[string]int)
	for ii := 0; ii < 100; ii++ {
		resp, err := testClient.TestRPC(context.Background(), &TestReq{ReqMsg: "test request"})
		AssertOk(t, err, "Failed to send RPC")
		respMap[resp.RespMsg]++
	}
	if respMap["t1"] != 50 || respMap["t2"] != 50 {
		t.Fatalf("Load balancing of RPCs failed, got %v and %v", respMap["t1"], respMap["t2"])
	}
}
