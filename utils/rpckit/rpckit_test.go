package rpckit

import (
	"errors"
	"fmt"
	"testing"
	"time"

	log "github.com/Sirupsen/logrus"
	"golang.org/x/net/context"
	"google.golang.org/grpc"
	"google.golang.org/grpc/credentials"

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
func newRPCServerClient() (*RPCServer, *RPCClient, error) {
	// create an RPC server
	rpcServer, err := NewRPCServer(":9900", "", "", "")
	if err != nil {
		return nil, nil, err
	}

	// create an RPC client
	rpcClient, err := NewRPCClient("localhost:9900", "", "", "")
	if err != nil {
		return nil, nil, err
	}

	return rpcServer, rpcClient, nil
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
	rpcServer, rpcClient, err := newRPCServerClient()
	AssertOk(t, err, "Error creating RPC server/client")

	// close client connection and stop the server
	defer stopRPCServerClient(rpcServer, rpcClient)

	// register the handlers
	RegisterTestServer(rpcServer.GrpcServer, testHandler)
	testClient := NewTestClient(rpcClient.ClientConn)

	// make sure server with no URL or client with no URL fails
	_, err = NewRPCServer("", "", "", "")
	Assert(t, (err != nil), "server with no URL succeded while expecting it to fail")
	_, err = NewRPCClient("", "", "", "")
	Assert(t, (err != nil), "client with no URL succeded while expecting it to fail")
	_, err = NewRPCServer("google.com", "", "", "")
	Assert(t, (err != nil), "server with no URL succeded while expecting it to fail")

	// make an RPC call
	resp, err := testClient.TestRPC(context.Background(), &TestReq{"test request"})
	AssertOk(t, err, "RPC error")
	Assert(t, (testHandler.reqMsg == "test request"), "Unexpected req msg", testHandler)
	Assert(t, (resp.RespMsg == "test response"), "Unexpected resp msg", resp)

	// make test err rpc and confirm we get an error
	_, err = testClient.TestRPCErr(context.Background(), &TestReq{"test request"})
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
	rpcServer, rpcClient, err := newRPCServerClient()
	AssertOk(t, err, "Error creating RPC server/client")

	// close client connection and stop the server
	defer stopRPCServerClient(rpcServer, rpcClient)

	// register the handlers
	RegisterTestServer(rpcServer.GrpcServer, testHandler)
	testClient := NewTestClient(rpcClient.ClientConn)

	// make an RPC call
	_, err = testClient.TestRPC(context.Background(), &TestReq{"test request"})
	AssertOk(t, err, "RPC error")

	// close the client connection
	t.Logf("Disconnecting rpc client connection")
	rpcClient.Close()

	// make an RPC call on closed connection
	_, err = testClient.TestRPC(context.Background(), &TestReq{"test request"})
	Assert(t, (err != nil), "RPC succeded while expecting it to fail")

	// reconnect the client
	t.Logf("Reconnecting to RPC server")
	err = rpcClient.Reconnect()
	AssertOk(t, err, "RPC reconnect error")

	// make sure an RPC call succeeds after reconnecting
	testClient = NewTestClient(rpcClient.ClientConn)
	_, err = testClient.TestRPC(context.Background(), &TestReq{"test request"}, grpc.FailFast(false))
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
	rpcServer, rpcClient, err := newRPCServerClient()
	AssertOk(t, err, "Error creating RPC server/client")

	// close client connection and stop the server
	defer stopRPCServerClient(rpcServer, rpcClient)

	// register the handlers
	RegisterTestServer(rpcServer.GrpcServer, testHandler)
	testClient := NewTestClient(rpcClient.ClientConn)

	// make an RPC call
	_, err = testClient.TestRPC(context.Background(), &TestReq{"test request"})
	AssertOk(t, err, "RPC error")

	// stop the old server and wait for a while to make sure connectin closes
	t.Logf("Stopping RPC server")
	rpcServer.Stop()
	time.Sleep(time.Millisecond * 100)

	// make an RPC call on closed connection make sure it fails
	_, err = testClient.TestRPC(context.Background(), &TestReq{"test request"})
	Assert(t, (err != nil), "RPC succeded while expecting it to fail")

	// start a new server
	t.Logf("Restarting RPC server")
	rpcServer, err = NewRPCServer(":9900", "", "", "")
	AssertOk(t, err, "RPC server restart error")
	RegisterTestServer(rpcServer.GrpcServer, testHandler)
	defer func() { rpcServer.Stop() }()

	// make an RPC call succeds with new server
	_, err = testClient.TestRPC(context.Background(), &TestReq{"test request"}, grpc.FailFast(false))
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
	rpcServer, rpcClient, err := newRPCServerClient()
	AssertOk(t, err, "Error creating RPC server/client")

	// close client connection and stop the server
	defer stopRPCServerClient(rpcServer, rpcClient)

	// register the handlers
	RegisterTestServer(rpcServer.GrpcServer, testHandler)
	testClient := NewTestClient(rpcClient.ClientConn)
	RegisterTest2Server(rpcServer.GrpcServer, test2Handler)
	test2Client := NewTest2Client(rpcClient.ClientConn)

	// make an RPC call
	resp, err := testClient.TestRPC(context.Background(), &TestReq{"test request"})
	AssertOk(t, err, "RPC error")
	Assert(t, (testHandler.reqMsg == "test request"), "Unexpected req msg", testHandler)
	Assert(t, (resp.RespMsg == "test response"), "Unexpected resp msg", resp)

	// make RPC call to second handler
	resp, err = test2Client.TestRPC(context.Background(), &TestReq{"test2 request"})
	AssertOk(t, err, "RPC error")
	Assert(t, (test2Handler.reqMsg == "test2 request"), "Unexpected req msg", testHandler)
	Assert(t, (resp.RespMsg == "test2 response"), "Unexpected resp msg", resp)
}

// test middleware for testing middleware callbacks
type testMiddleware struct {
	rpcStats map[string]int64
}

// ReqInterceptor implements request interception
func (s *testMiddleware) ReqInterceptor(ctx context.Context, role string, method string, req interface{}) {
	s.rpcStats[fmt.Sprintf("%s-%s:Req", role, method)]++
}

// RespInterceptor handles responses
func (s *testMiddleware) RespInterceptor(ctx context.Context, role string, method string, req, reply interface{}, err error) {
	s.rpcStats[fmt.Sprintf("%s-%s:Resp", role, method)]++
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
	rpcServer, err := NewRPCServer(":9900", "", "", "", tstmdl)
	AssertOk(t, err, "Error creating RPC server")

	// create an RPC client
	rpcClient, err := NewRPCClient("localhost:9900", "", "", "", tstmdl)
	AssertOk(t, err, "Error creating RPC client")

	// close client connection and stop the server
	defer stopRPCServerClient(rpcServer, rpcClient)

	// register the handlers
	RegisterTestServer(rpcServer.GrpcServer, testHandler)
	testClient := NewTestClient(rpcClient.ClientConn)

	// make an RPC call
	_, err = testClient.TestRPC(context.Background(), &TestReq{"test request"})
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

	// create tls & non-tls RPC server
	rpcServer, err := NewRPCServer(nontlsURL, "", "", "")
	AssertOk(t, err, "Error creating non-TLS RPC server")
	tlsRPCServer, err := NewRPCServer(tlsURL, "testcerts/testServer.crt", "testcerts/testServer.key", "testcerts/testCA.crt")
	AssertOk(t, err, "Error creating TLS RPC server")

	// create tls & non-tls RPC client
	rpcClient, err := NewRPCClient(nontlsURL, "", "", "")
	AssertOk(t, err, "Error creating non-TLS RPC client")
	tlsRPCClient, err := NewRPCClient(tlsURL, "testcerts/testClient.crt", "testcerts/testClient.key", "testcerts/testCA.crt")
	AssertOk(t, err, "Error creating TLS RPC client")

	// close client connection and stop the server
	defer stopRPCServerClient(rpcServer, rpcClient)
	defer stopRPCServerClient(tlsRPCServer, tlsRPCClient)

	// make sure server and clients with bad cert file names fail
	_, err = NewRPCServer(":9800", "test.crt", "", "")
	Assert(t, (err != nil), "server with bad cert succeded while expecting it to fail")
	_, err = NewRPCServer(":9801", "test.crt", "test.key", "testcerts/testCA.crt")
	Assert(t, (err != nil), "server with bad cert succeded while expecting it to fail")
	_, err = NewRPCServer(":9802", "testcerts/testServer.crt", "testcerts/testServer.key", "test.ca")
	Assert(t, (err != nil), "server with bad cert succeded while expecting it to fail")

	_, err = NewRPCClient("localhost:9000", "test.crt", "", "")
	Assert(t, (err != nil), "client with bad cert succeded while expecting it to fail")
	_, err = NewRPCClient("localhost:9000", "test.crt", "test.key", "testcerts/testCA.crt")
	Assert(t, (err != nil), "client with bad cert succeded while expecting it to fail")
	_, err = NewRPCClient("localhost:9000", "testcerts/testClient.crt", "testcerts/testClient.key", "testCA.crt")
	Assert(t, (err != nil), "client with bad cert succeded while expecting it to fail")

	// make sure non-tls client can not connect to tls server
	// Note: grpc non-tls clients dont block till certification validation is done.
	//       So, we need to make a dummy rpc call and confirm we are not able to connect
	t.Logf("non-TLS client trying to connect to TLS server")
	nontlsRPCClient, err := NewRPCClient(tlsURL, "", "", "")
	AssertOk(t, err, "Error creating non-TLS RPC client")
	nontlsTestClient := NewTestClient(nontlsRPCClient.ClientConn)
	_, err = nontlsTestClient.TestRPC(context.Background(), &TestReq{"test request"})
	Assert(t, (err != nil), "non-tls client was able to connect to tls-server")

	// make sure a client with empty credentials can not connect to tls server
	t.Logf("Empty TLS client trying to connect to TLS server")
	_, err = grpc.Dial(tlsURL, grpc.WithBlock(), grpc.WithTimeout(time.Second*3),
		grpc.WithTransportCredentials(credentials.NewClientTLSFromCert(nil, "")))
	Assert(t, (err != nil), "client with empty tls cert was able to connect to tls-server")

	// make sure tls client will not connect to non-tls server
	t.Logf("TLS client trying to connect to non-TLS server")
	_, err = NewRPCClient(nontlsURL, "testcerts/testClient.crt", "testcerts/testClient.key", "testcerts/testCA.crt")
	Assert(t, (err != nil), "tls client was able to connect to non-tls server")

	// verify client and servers with different certs will not connect
	t.Logf("TLS client with incorrect certificate trying to connect to TLS server")
	_, err = NewRPCClient(tlsURL, "testcerts/test2Client.crt", "testcerts/test2Client.key", "testcerts/test2CA.crt")
	Assert(t, (err != nil), "tls client was able to connect to non-tls server")

	// verify client and servers with different root CA will not connect
	t.Logf("TLS client with incorrect certificate trying to connect to TLS server")
	_, err = NewRPCClient(tlsURL, "testcerts/testClient.crt", "testcerts/testClient.key", "testcerts/test2CA.crt")
	Assert(t, (err != nil), "tls client was able to connect to non-tls server")

	// register the handlers
	RegisterTestServer(rpcServer.GrpcServer, testHandler)
	testClient := NewTestClient(rpcClient.ClientConn)
	RegisterTestServer(tlsRPCServer.GrpcServer, tlsTestHandler)
	tlsTestClient := NewTestClient(tlsRPCClient.ClientConn)

	// make an RPC call
	_, err = testClient.TestRPC(context.Background(), &TestReq{"test request"})
	AssertOk(t, err, "RPC error")
	resp, err := tlsTestClient.TestRPC(context.Background(), &TestReq{"test TLS request"})
	AssertOk(t, err, "RPC error")
	Assert(t, (tlsTestHandler.reqMsg == "test TLS request"), "Unexpected req msg", tlsTestHandler)
	Assert(t, (resp.RespMsg == "test TLS response"), "Unexpected resp msg", resp)

	// verify reconnect with TLS succeeds
	tlsRPCClient.Close()
	err = tlsRPCClient.Reconnect()
	AssertOk(t, err, "RPC reconnect error")
	tlsTestClient = NewTestClient(tlsRPCClient.ClientConn)
	_, err = tlsTestClient.TestRPC(context.Background(), &TestReq{"test TLS request"})
	AssertOk(t, err, "RPC error")
}
