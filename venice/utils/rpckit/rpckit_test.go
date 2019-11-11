// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package rpckit_test

import (
	"fmt"
	"net"
	"strconv"
	"sync"
	"testing"
	"time"

	"golang.org/x/net/context"
	"google.golang.org/grpc"
	"google.golang.org/grpc/credentials"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/cmd/grpc/service"
	"github.com/pensando/sw/venice/cmd/services/mock"
	types "github.com/pensando/sw/venice/cmd/types/protos"
	"github.com/pensando/sw/venice/utils/balancer"
	"github.com/pensando/sw/venice/utils/resolver"
	. "github.com/pensando/sw/venice/utils/rpckit"
	"github.com/pensando/sw/venice/utils/rpckit/tlsproviders"
	. "github.com/pensando/sw/venice/utils/testutils"
	"github.com/pensando/sw/venice/utils/trace"

	zipkin "github.com/openzipkin/zipkin-go-opentracing"
	"github.com/openzipkin/zipkin-go-opentracing/thrift/gen-go/zipkincore"
)

// newRPCServerClient create an RPC server and client
func newRPCServerClient(t *testing.T, instance int) (*RPCServer, *RPCClient) {
	// create an RPC server
	rpcServer, err := NewRPCServer(fmt.Sprintf("testServer-%v", instance), "localhost:0", WithTracerEnabled(true))
	if err != nil {
		t.Fatalf("Failed to create listener, error: %v", err)
	}

	// create an RPC client
	rpcClient, err := NewRPCClient(fmt.Sprintf("testClient-%v", instance), rpcServer.GetListenURL(), WithTracerEnabled(true))
	if err != nil {
		t.Fatalf("Failed to create client with error: %v", err)
	}

	return rpcServer, rpcClient
}

// stopRPCServerClient closes connections and stops RPC server
func stopRPCServerClient(rpcServer *RPCServer, rpcClient *RPCClient) {
	rpcClient.Close()
	rpcServer.Stop()
	time.Sleep(4 * time.Millisecond)
}

// getStat extracts a uint64 value from a key in expvar.Map. Panics if it fails to parse.
func getStat(t *testing.T, key string) uint64 {
	v := Stats().Get(key)
	if v == nil {
		return 0
	}
	vstr := fmt.Sprintf("%v", v)
	val, err := strconv.ParseUint(vstr, 10, 64)
	if err != nil {
		t.Fatalf("Failed to parse var %v with err: %v", v, err)
	}
	return val
}

// validateMethodStats validates that the stats for a method match expected counts.
func validateStats(t *testing.T, svc, prefix, method, suffix string, reqs, resps, errs uint64) {
	reqsKey := fmt.Sprintf("%v/%v/requests%v%v", svc, prefix, method, suffix)
	respsKey := fmt.Sprintf("%v/%v/responses%v%v", svc, prefix, method, suffix)
	errsKey := fmt.Sprintf("%v/%v/errors%v%v", svc, prefix, method, suffix)

	Assert(t, getStat(t, reqsKey) == reqs, "Unexpected rpc requests %+v %+v", getStat(t, reqsKey), reqs)
	Assert(t, getStat(t, respsKey) == resps, "Unexpected rpc responses", getStat(t, respsKey), resps)
	Assert(t, getStat(t, errsKey) == errs, "Unexpected rpc errors", getStat(t, errsKey), errs)
}

// test basic RPC
func TestRPCBasic(t *testing.T) {
	ClearStats()

	SetDefaultClientFactory(NewClientFactory("testnode-uuid"))

	// create an rpc handler object
	testHandler := NewTestRPCHandler("dummy message", "test response")

	// create an RPC server and client
	rpcServer, rpcClient := newRPCServerClient(t, 1)

	// close client connection and stop the server
	defer stopRPCServerClient(rpcServer, rpcClient)

	// register the handlers and start the server
	RegisterTestServer(rpcServer.GrpcServer, testHandler)
	rpcServer.Start()

	testClient := NewTestClient(rpcClient.ClientConn)

	// make sure server with no URL or client with no URL fails
	_, err := NewRPCServer("testServer-foo", "")
	Assert(t, (err != nil), "server with no URL succeeded while expecting it to fail")
	_, err = NewRPCClient("testClient-foo", "")
	Assert(t, (err != nil), "client with no URL succeeded while expecting it to fail")
	_, err = NewRPCServer("testServer-bar", "google.com")
	Assert(t, (err != nil), "server with no URL succeeded while expecting it to fail")

	// make an RPC call
	resp, err := testClient.TestRPC(context.Background(), &TestReq{ReqMsg: "test request"})
	AssertOk(t, err, "RPC error")
	Assert(t, (testHandler.ReqMsg == "test request"), "Unexpected req msg", testHandler)
	Assert(t, (resp.RespMsg == "test response"), "Unexpected resp msg", resp)
	Assert(t, (resp.CallerNodeUUID == "testnode-uuid"), "Unexcpected CallerNodeUUID", resp.CallerNodeUUID)

	// make test err rpc and confirm we get an error
	_, err = testClient.TestRPCErr(context.Background(), &TestReq{ReqMsg: "test request"})
	Assert(t, (err != nil), "TestRPCErr succeeded while expecting it to fail")

	// verify RPC stats got incremented
	method1 := "/rpckit.Test/TestRPC"
	method2 := "/rpckit.Test/TestRPCErr"
	validateStats(t, "testServer-1", "server", method1, "", 1, 1, 0)
	validateStats(t, "testServer-1", "server", method2, "", 1, 1, 1)
	validateStats(t, "testServer-1", "server", method1, "/client/testClient-1", 1, 1, 0)
	validateStats(t, "testServer-1", "server", method2, "/client/testClient-1", 1, 1, 1)
	validateStats(t, "testClient-1", "client", method1, "", 1, 1, 0)
	validateStats(t, "testClient-1", "client", method2, "", 1, 1, 1)
}

// test rpc client connection closing and reconnecting
func TestRPCClientReconnect(t *testing.T) {
	ClearStats()

	// create an rpc handler object
	testHandler := NewTestRPCHandler("dummy message", "test response")

	// create an RPC server and
	rpcServer, rpcClient := newRPCServerClient(t, 2)

	// close client connection and stop the server
	defer stopRPCServerClient(rpcServer, rpcClient)

	// register the handlers and start the server
	RegisterTestServer(rpcServer.GrpcServer, testHandler)
	rpcServer.Start()

	testClient := NewTestClient(rpcClient.ClientConn)

	// make an RPC call
	_, err := testClient.TestRPC(context.Background(), &TestReq{ReqMsg: "test request"})
	AssertOk(t, err, "RPC error")

	// close the client connection
	t.Logf("Disconnecting rpc client connection")
	rpcClient.Close()

	// make an RPC call on closed connection
	_, err = testClient.TestRPC(context.Background(), &TestReq{ReqMsg: "test request"})
	Assert(t, (err != nil), "RPC succeeded while expecting it to fail")

	// reconnect the client
	t.Logf("Reconnecting to RPC server")
	err = rpcClient.Reconnect()
	AssertOk(t, err, "RPC reconnect error")

	// make sure an RPC call succeeds after reconnecting
	testClient = NewTestClient(rpcClient.ClientConn)
	_, err = testClient.TestRPC(context.Background(), &TestReq{ReqMsg: "test request"}, grpc.FailFast(false))
	AssertOk(t, err, "RPC error")

	// verify RPC stats got incremented
	method := "/rpckit.Test/TestRPC"
	validateStats(t, "testServer-2", "server", method, "", 2, 2, 0)
	validateStats(t, "testServer-2", "server", method, "/client/testClient-2", 2, 2, 0)
	validateStats(t, "testClient-2", "client", method, "", 3, 3, 1)
}

// test RPC server restart
func TestRPCServerRestart(t *testing.T) {
	ClearStats()

	// create an rpc handler object
	testHandler := NewTestRPCHandler("dummy message", "test response")

	// create an RPC server and
	rpcServer, rpcClient := newRPCServerClient(t, 3)

	// close client connection and stop the server
	defer stopRPCServerClient(rpcServer, rpcClient)

	// register the handlers and start the server
	RegisterTestServer(rpcServer.GrpcServer, testHandler)
	rpcServer.Start()

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
	Assert(t, (err != nil), "RPC succeeded while expecting it to fail")

	// start a new server
	t.Logf("Restarting RPC server")
	rpcServer, err = NewRPCServer("testServer", rpcServer.GetListenURL())
	AssertOk(t, err, "RPC server restart error")
	RegisterTestServer(rpcServer.GrpcServer, testHandler)
	rpcServer.Start()
	defer func() {
		rpcServer.Stop()
		time.Sleep(time.Millisecond)
	}()

	// make an RPC call succeeds with new server
	_, err = testClient.TestRPC(context.Background(), &TestReq{ReqMsg: "test request"}, grpc.FailFast(false))
	AssertOk(t, err, "RPC error")

	// verify RPC stats got incremented
	method := "/rpckit.Test/TestRPC"
	validateStats(t, "testServer-3", "server", method, "", 1, 1, 0)
	validateStats(t, "testServer-3", "server", method, "/client/testClient-3", 1, 1, 0)
	validateStats(t, "testClient-3", "client", method, "", 3, 3, 1)
}

// test multiple RPC handlers on same server
func TestRPCMultipleHandlers(t *testing.T) {
	// create an rpc handler object
	testHandler := NewTestRPCHandler("dummy message", "test response")

	// create another rpc handler object
	test2Handler := NewTestRPCHandler("dummy message", "test2 response")

	// create an RPC server and
	rpcServer, rpcClient := newRPCServerClient(t, 4)

	// close client connection and stop the server
	defer stopRPCServerClient(rpcServer, rpcClient)

	// register the handlers and start the server
	RegisterTestServer(rpcServer.GrpcServer, testHandler)
	testClient := NewTestClient(rpcClient.ClientConn)
	RegisterTest2Server(rpcServer.GrpcServer, test2Handler)
	test2Client := NewTest2Client(rpcClient.ClientConn)
	rpcServer.Start()

	// make an RPC call
	resp, err := testClient.TestRPC(context.Background(), &TestReq{ReqMsg: "test request"})
	AssertOk(t, err, "RPC error")
	Assert(t, (testHandler.ReqMsg == "test request"), "Unexpected req msg", testHandler)
	Assert(t, (resp.RespMsg == "test response"), "Unexpected resp msg", resp)

	// make RPC call to second handler
	resp, err = test2Client.TestRPC(context.Background(), &TestReq{ReqMsg: "test2 request"})
	AssertOk(t, err, "RPC error")
	Assert(t, (test2Handler.ReqMsg == "test2 request"), "Unexpected req msg", testHandler)
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
	testHandler := NewTestRPCHandler("dummy message", "test response")

	tstmdl := &testMiddleware{
		rpcStats: make(map[string]int64),
	}

	// create an RPC server
	rpcServer, err := NewRPCServer("testServer-mw", ":0", WithMiddleware(tstmdl))
	AssertOk(t, err, "Error creating RPC server")

	// create an RPC client
	rpcClient, err := NewRPCClient("testClient-mw", rpcServer.GetListenURL(), WithMiddleware(tstmdl))
	AssertOk(t, err, "Error creating RPC client")

	// close client connection and stop the server
	defer stopRPCServerClient(rpcServer, rpcClient)

	// register the handlers and start the server
	RegisterTestServer(rpcServer.GrpcServer, testHandler)
	rpcServer.Start()
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
	// create tls and non-tls rpc handler object
	testHandler := NewTestRPCHandler("dummy message", "test response")
	tlsTestHandler := NewTestRPCHandler("dummy message", "test TLS response")

	tlsProvider, err := tlsproviders.NewFileBasedProvider(
		"tlsproviders/testcerts/testServer.crt",
		"tlsproviders/testcerts/testServer.key",
		"tlsproviders/testcerts/testCA.crt")
	AssertOk(t, err, "Failed to instantiate TLS provider")
	defer tlsProvider.Close()

	// create tls & non-tls RPC server
	rpcServer, err := NewRPCServer("testNonTLSServer", "localhost:0")
	AssertOk(t, err, "Error creating non-TLS RPC server")
	nontlsURL := rpcServer.GetListenURL()
	tlsRPCServer, err := NewRPCServer("testServer", "localhost:0", WithTLSProvider(tlsProvider))
	AssertOk(t, err, "Error creating TLS RPC server")
	tlsURL := tlsRPCServer.GetListenURL()

	// register the handlers and start the servers
	RegisterTestServer(rpcServer.GrpcServer, testHandler)
	rpcServer.Start()

	RegisterTestServer(tlsRPCServer.GrpcServer, tlsTestHandler)
	tlsRPCServer.Start()

	// create tls & non-tls RPC client
	rpcClient, err := NewRPCClient("testNonTLSClient", nontlsURL)
	AssertOk(t, err, "Error creating non-TLS RPC client")
	tlsRPCClient, err := NewRPCClient("testServer", tlsURL, WithTLSProvider(tlsProvider), WithRemoteServerName("testServer"))
	AssertOk(t, err, "Error creating TLS RPC client")
	testClient := NewTestClient(rpcClient.ClientConn)
	tlsTestClient := NewTestClient(tlsRPCClient.ClientConn)

	// close client connection and stop the server
	defer stopRPCServerClient(rpcServer, rpcClient)
	defer stopRPCServerClient(tlsRPCServer, tlsRPCClient)

	// make sure non-tls client can not connect to tls server
	// Note: grpc non-tls clients dont block till certification validation is done.
	//       So, we need to make a dummy rpc call and confirm we are not able to connect
	t.Logf("non-TLS client trying to connect to TLS server")
	nontlsRPCClient, err := NewRPCClient("testClient", tlsURL)
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
	_, err = NewRPCClient("testServer", nontlsURL, WithTLSProvider(tlsProvider))
	Assert(t, (err != nil), "tls client was able to connect to non-tls server")

	// verify client and servers with different certs will not connect
	tlsProvider2, err := tlsproviders.NewFileBasedProvider(
		"tlsproviders/testcerts/test2Server.crt",
		"tlsproviders/testcerts/test2Server.key",
		"tlsproviders/testcerts/test2CA.crt")
	AssertOk(t, err, "Failed to instantiate TLS provider")
	defer tlsProvider2.Close()

	t.Logf("TLS client with incorrect certificate trying to connect to TLS server")
	_, err = NewRPCClient("testServer", tlsURL, WithTLSProvider(tlsProvider2))
	Assert(t, (err != nil), "tls client was able to connect to non-tls server")

	// verify client and servers with different root CA will not connect
	tlsProvider3, err := tlsproviders.NewFileBasedProvider(
		"tlsproviders/testcerts/testClient.crt",
		"tlsproviders/testcerts/testClient.key",
		"tlsproviders/testcerts/test2CA.crt")
	AssertOk(t, err, "Failed to instantiate TLS provider")
	defer tlsProvider3.Close()

	t.Logf("TLS client with incorrect certificate trying to connect to TLS server")
	_, err = NewRPCClient("testServer", tlsURL, WithTLSProvider(tlsProvider3))
	Assert(t, (err != nil), "tls client was able to connect to non-tls server")

	// make an RPC call
	_, err = testClient.TestRPC(context.Background(), &TestReq{ReqMsg: "test request"})
	AssertOk(t, err, "RPC error")
	resp, err := tlsTestClient.TestRPCWithContext(context.Background(), &TestReq{ReqMsg: "test TLS request"})
	AssertOk(t, err, "RPC error")
	Assert(t, (tlsTestHandler.ReqMsg == "test TLS request"), "Unexpected req msg", tlsTestHandler)
	Assert(t, (resp.RespMsg == "test TLS response"), "Unexpected resp msg", resp)
	Assert(t, resp.CallerID == "testServer", fmt.Sprintf("Unexpected Caller ID, have: %v, want: %v", resp.CallerID, "testServer"))
	Assert(t, resp.CallerAddress != "", "Caller address should not be empty")

	// verify reconnect with TLS succeeds
	// TODO: Timing issue - fails
	/*
		tlsRPCClient.Close()
		err = tlsRPCClient.Reconnect()
		AssertOk(t, err, "RPC reconnect error")
		tlsTestClient = NewTestClient(tlsRPCClient.ClientConn)
		_, err = tlsTestClient.TestRPC(context.Background(), &TestReq{ReqMsg: "test TLS request"})
		AssertOk(t, err, "RPC error")
	*/
}

// helper function to start a mock resolver.
func startMockResolver(t *testing.T) (*mock.ResolverService, *RPCServer) {
	// Create a mock resolver, no server backends are registered with the resolver.
	m := mock.NewResolverService()
	resolverHandler := service.NewRPCHandler(m)
	resolverServer, err := NewRPCServer("resolver", "localhost:0", WithTracerEnabled(true), WithTLSProvider(nil))
	AssertOk(t, err, "Failed to create listener")
	types.RegisterServiceAPIServer(resolverServer.GrpcServer, resolverHandler)
	resolverServer.Start()
	return m, resolverServer
}

// helper function to start a test server.
func startTestServer(t *testing.T, svcName, reqMsg, respMsg string) *RPCServer {
	testHandler := NewTestRPCHandler(reqMsg, respMsg)
	rpcServer, err := NewRPCServer(svcName, "localhost:0", WithTracerEnabled(true), WithTLSProvider(nil))
	AssertOk(t, err, "Failed to create listener")

	// register handlers with rpc server and start it
	RegisterTestServer(rpcServer.GrpcServer, testHandler)
	rpcServer.Start()
	return rpcServer
}

// helper function to add a test server to a mock resolver.
func addServerToResolver(t *testing.T, m *mock.ResolverService, s *RPCServer, svcName, instanceName string) {
	_, portStr, err := net.SplitHostPort(s.GetListenURL())
	AssertOk(t, err, "Failed to parse port")
	port, err := strconv.Atoi(portStr)
	AssertOk(t, err, "Failed to convert port")

	// populate the mock resolver with the server instance.
	si := types.ServiceInstance{
		TypeMeta: api.TypeMeta{
			Kind: "ServiceInstance",
		},
		ObjectMeta: api.ObjectMeta{
			Name: instanceName,
		},
		Service: svcName,
		Node:    "localhost",
		URL:     fmt.Sprintf("localhost:%d", port),
	}
	m.AddServiceInstance(&si)
}

// test load balancing of RPCs
func TestRPCBalancing(t *testing.T) {
	m, resolverServer := startMockResolver(t)

	rpcServer1 := startTestServer(t, "testService", "", "t1")
	rpcServer2 := startTestServer(t, "testService", "", "t2")
	defer func() {
		rpcServer1.Stop()
		rpcServer2.Stop()
		time.Sleep(time.Millisecond)
	}()

	addServerToResolver(t, m, rpcServer1, "testService", "t1")
	addServerToResolver(t, m, rpcServer2, "testService", "t2")

	// Now create a rpc client with a balancer
	r := resolver.New(&resolver.Config{Name: "testService", Servers: []string{resolverServer.GetListenURL()}})
	b := balancer.New(r)
	client, err := NewRPCClient("RPCBalanceTest", "testService", WithBalancer(b), WithTLSProvider(nil))
	AssertOk(t, err, "Failed to create RPC Client")
	defer client.Close()
	testClient := NewTestClient(client.ClientConn)

	// Wait until grpc connects to both the servers
	AssertEventually(t, func() (bool, interface{}) {
		return b.NumUpConns() == 2, nil
	}, "Unexpected up servers")

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

// test RPC blocking with Balancer. This is the default behavior. RPC should
// block when there is no backend available and should succeed if the backend
// becomes available before the default timeout of 3 seconds.
func TestRPCBlockingWithBalancer(t *testing.T) {
	m, resolverServer := startMockResolver(t)

	var wg sync.WaitGroup
	wg.Add(1)
	t1 := time.Now()
	sleepTime := time.Millisecond * 100
	go func() {
		// Now create a rpc client with a balancer
		r := resolver.New(&resolver.Config{Name: "testService", Servers: []string{resolverServer.GetListenURL()}})
		b := balancer.New(r)
		client, err := NewRPCClient("RPCBlockingWithBalancerTest", "testService", WithBalancer(b), WithTLSProvider(nil))
		AssertOk(t, err, "Failed to create RPC Client")
		defer client.Close()
		testClient := NewTestClient(client.ClientConn)
		resp, err := testClient.TestRPC(context.Background(), &TestReq{ReqMsg: "test request"})
		t.Logf("Got resp %v, err %v", resp, err)
		if time.Since(t1) < sleepTime {
			t.Fatalf("Client RPC returned before server backend is up")
		}
		wg.Done()
	}()

	// Add a server after the client sends the RPC.
	time.Sleep(sleepTime)

	rpcServer := startTestServer(t, "testService", "", "t1")
	defer func() {
		rpcServer.Stop()
		time.Sleep(time.Millisecond)
	}()

	addServerToResolver(t, m, rpcServer, "testService", "t1")

	wg.Wait()

	if time.Since(t1) > time.Second*1 {
		t.Fatalf("Client RPC didn't finish in 1 second after server backend is up")
	}
}

// DummyCollector implements Collector but performs no work.
type DummyCollector struct {
	AnnotationsSeen map[string]interface{}
}

// Collect implements Collector.
func (c *DummyCollector) Collect(s *zipkincore.Span) error {
	for _, a := range s.Annotations {
		if a != nil {
			c.AnnotationsSeen[a.Value] = nil
		}
	}
	return nil
}

// Close implements Collector.
func (c *DummyCollector) Close() error { return nil }

// test tracing middleware
func TestRPCTraceMiddlewares(t *testing.T) {
	collector := &DummyCollector{
		AnnotationsSeen: make(map[string]interface{}),
	}
	recorder := zipkin.NewRecorder(collector, true, "", t.Name(), zipkin.WithJSONMaterializer())
	tracer, err := zipkin.NewTracer(recorder, zipkin.ClientServerSameSpan(false), zipkin.TraceID128Bit(true))
	AssertOk(t, err, "Error creating Tracer")
	trace.SetGlobalTracer(tracer)

	// create an rpc handler object
	testHandler := NewTestRPCHandler("dummy message", "test response")

	// create an RPC server
	rpcServer, err := NewRPCServer("testServer-trace", ":0", WithTracerEnabled(true))
	AssertOk(t, err, "Error creating RPC server")

	// create an RPC client
	rpcClient, err := NewRPCClient("testClient-trace", rpcServer.GetListenURL(), WithTracerEnabled(true))
	AssertOk(t, err, "Error creating RPC client")

	// close client connection and stop the server
	defer stopRPCServerClient(rpcServer, rpcClient)

	// register the handlers and start the server
	RegisterTestServer(rpcServer.GrpcServer, testHandler)
	rpcServer.Start()

	testClient := NewTestClient(rpcClient.ClientConn)

	// make an RPC call
	_, err = testClient.TestRPC(context.Background(), &TestReq{ReqMsg: "test request"})
	AssertOk(t, err, "RPC error")

	_, ok := collector.AnnotationsSeen["{\"gRPC request sent\":\"reqMsg:\\\"test request\\\" \"}"]
	Assert(t, ok, "RPC request sent not found in annotations")
	_, ok = collector.AnnotationsSeen["{\"gRPC request received\":\"reqMsg:\\\"test request\\\" \"}"]
	Assert(t, ok, "RPC request received not found in annotations")
	fmt.Printf("%+v", collector)
	_, ok = collector.AnnotationsSeen["{\"gRPC response sent\":\"respMsg:\\\"test response\\\" callerNodeUUID:\\\"testnode-uuid\\\" \"}"]
	Assert(t, ok, "RPC response sent not found in annotations")
	_, ok = collector.AnnotationsSeen["{\"gRPC response received\":\"respMsg:\\\"test response\\\" callerNodeUUID:\\\"testnode-uuid\\\" \"}"]
	Assert(t, ok, "RPC response received not found in annotations")
}

func createRPCClients(t *testing.T, wg *sync.WaitGroup, srcAddr, srvURL string, numConns int, rate int) (int, int, time.Duration) {
	var errors, success int
	sleepMs := time.Duration(1000.0 / rate)
	defer wg.Done()
	begin := time.Now()
	for i := 0; i < numConns; i++ {
		dialer := &net.Dialer{LocalAddr: &net.TCPAddr{IP: net.ParseIP(srcAddr)}}
		client, err := NewRPCClient(srcAddr, srvURL, WithTracerEnabled(false), WithDialer(dialer), WithDialTimeout(200*time.Millisecond))
		if err != nil {
			t.Logf("Failed to create client with error %v", err)
			errors++
		} else {
			testClient := NewTestClient(client.ClientConn)
			_, err = testClient.TestRPC(context.Background(), &TestReq{ReqMsg: "test request"})
			if err != nil {
				t.Logf("Failed to perform RPC with error %v", err)
				errors++
			} else {
				success++
			}
			client.Close()
		}
		time.Sleep(sleepMs * time.Millisecond)
	}
	end := time.Now()
	return success, errors, end.Sub(begin)
}

func TestRPCConnectionRateLimit(t *testing.T) {
	ClearStats()

	SetDefaultClientFactory(NewClientFactory("testnode-uuid"))

	// create an rpc handler object
	testHandler := NewTestRPCHandler("dummy message", "test response")

	rlc := &RateLimitConfig{
		B: 10,
		R: 1.0,
	}
	rpcServer, err := NewRPCServer("testServer", "127.0.0.1:0", WithTracerEnabled(false),
		WithListenerRateLimitConfig(rlc), WithTCPListenerKeepAlivePeriod(1*time.Second))
	AssertOk(t, err, "Failed to create listener")

	// register the handlers and start the server
	RegisterTestServer(rpcServer.GrpcServer, testHandler)

	rpcServer.Start()
	defer rpcServer.Stop()

	getCompletionTime := func(numConns int, connRate float64) time.Duration {
		return time.Duration(float64(numConns)/connRate) * 1000 * time.Millisecond
	}

	getSuccessfulRPCs := func(numConns, burstSize int, connRate, acceptRate float64) int {
		if connRate <= acceptRate {
			return numConns
		}
		rlConns := numConns - burstSize
		rlTime := getCompletionTime(rlConns, connRate)
		return burstSize + int(acceptRate*float64(rlTime)/float64(time.Second))
	}

	numConnsPerClient := 60
	rates := []float64{1, 5, 10, 20}
	expSuccessfulRPCs := make([]int, len(rates))
	actualSuccessfulRPCs := make([]int, len(rates))
	expCompletionTimes := make([]time.Duration, len(rates))
	actualCompletionTimes := make([]time.Duration, len(rates))

	for i := 0; i < len(rates); i++ {
		expCompletionTimes[i] = getCompletionTime(numConnsPerClient, rates[i])
		var acceptRate float64
		if rates[i] <= float64(rlc.R) {
			acceptRate = rates[i]
		} else {
			acceptRate = float64(rlc.R)
		}
		expSuccessfulRPCs[i] = getSuccessfulRPCs(numConnsPerClient, rlc.B, rates[i], acceptRate)
	}

	var wg sync.WaitGroup
	wg.Add(len(rates))
	for i := range rates {
		go func(i int) {
			var numErrs int
			actualSuccessfulRPCs[i], numErrs, actualCompletionTimes[i] =
				createRPCClients(t, &wg, fmt.Sprintf("127.0.0.%d", i+10), rpcServer.GetListenURL(), numConnsPerClient, int(rates[i]))
			Assert(t, actualSuccessfulRPCs[i]+numErrs == numConnsPerClient,
				"Results mismatch, successful RPCs: %d, errors: %d, total: %d", actualSuccessfulRPCs[i]+numErrs == numConnsPerClient)
		}(i)
	}
	wg.Wait()

	margin := 0.25
	for i, r := range rates {
		rpcLowerBound := float64(expSuccessfulRPCs[i]) * (1 - margin)
		rpcUpperBound := float64(expSuccessfulRPCs[i]) * (1 + margin)
		rpcActualValue := float64(actualSuccessfulRPCs[i])
		Assert(t, rpcActualValue > rpcLowerBound && rpcActualValue < rpcUpperBound,
			"Rate: %f Unexpected number of successful RPCs. Have: %f, want: %f-%f", r, rpcActualValue, rpcLowerBound, rpcUpperBound)

		compTimeLowerBound := float64(expCompletionTimes[i]) * (1 - margin)
		// In the upper bound, allow up to 50 ms of average (dial + RPC time) for each successful RPC
		compTimeUpperBound := float64(expCompletionTimes[i])*(1+margin) +
			float64(time.Duration(expSuccessfulRPCs[i]*50)*time.Millisecond)
		compTimeActualValue := float64(actualCompletionTimes[i])
		Assert(t, compTimeActualValue > compTimeLowerBound && compTimeActualValue < compTimeUpperBound,
			"Rate: %f Unexpected completion time. Have: %f, want: %f-%f", r, compTimeActualValue, compTimeLowerBound, compTimeUpperBound)
	}
}
