package apisrvpkg

import (
	"context"
	"testing"

	opentracing "github.com/opentracing/opentracing-go"
	"google.golang.org/grpc/metadata"

	"github.com/pensando/sw/api"
	apisrv "github.com/pensando/sw/venice/apiserver"
	mocks "github.com/pensando/sw/venice/apiserver/pkg/mocks"
	"github.com/pensando/sw/venice/utils/kvstore"
	compliance "github.com/pensando/sw/venice/utils/kvstore/compliance"
)

// TestMethodWiths
// Test registration of various hooks to method.
// Check all precommit and post commit functions are called and skipping
// KV operation feedback from hook is honored
func TestMethodWiths(t *testing.T) {
	MustGetAPIServer()
	singletonAPISrv.runstate.running = true
	req := mocks.NewFakeMessage("TestType1", true).(*mocks.FakeMessage)
	resp := mocks.NewFakeMessage("TestType2", true).(*mocks.FakeMessage)
	f := mocks.NewFakeMethod(true).(*mocks.FakeMethod)
	// Add a few Pres and Posts and skip KV for testing
	m := NewMethod(req, resp, "testm", "TestMethodWiths").WithVersion("v1").WithPreCommitHook(f.PrecommitFunc).WithPreCommitHook(f.PrecommitFunc).WithPreCommitHook(f.PrecommitFunc)
	m = m.WithPostCommitHook(f.PostcommitfFunc).WithPostCommitHook(f.PostcommitfFunc).WithResponseWriter(f.RespWriterFunc).WithMakeURI(f.MakeURIFunc)
	m = m.WithOper("POST").WithVersion("Vtest")
	reqmsg := TestType1{}
	md := metadata.Pairs(apisrv.RequestParamVersion, singletonAPISrv.version,
		apisrv.RequestParamMethod, "GET")
	mhdlr := m.(*MethodHdlr)
	if mhdlr.version != "Vtest" || mhdlr.oper != "POST" {
		t.Errorf("Flags not set correction ver[%v] oper[%v]", mhdlr.version, mhdlr.oper)
	}
	ctx := metadata.NewIncomingContext(context.Background(), md)
	respmsg, _ := m.HandleInvocation(ctx, reqmsg)

	if r, ok := respmsg.(string); ok {
		if r != "TestResponse" {
			t.Errorf("Expecting a result of [TestResponse] got [%v]", r)
		}
	} else {
		t.Errorf("Result not overridden")
	}

	if f.Pres != 3 {
		t.Errorf("Expecting 3 precommit invocations found %v", f.Pres)
	}
	if f.Posts != 2 {
		t.Errorf("Expecting 2 postcommit invocations found %v", f.Posts)
	}
	// There should be no KV operations involved since the precommit skipped KV
	if req.Kvreads != 0 {
		t.Errorf("Expecting no KV reads but found %v", req.Kvreads)
	}

	if f.MakeURIs != 1 {
		t.Errorf("expecting 1 MakeURI request got %d", f.MakeURIs)
	}
	m.Disable()
	if mhdlr.enabled == true {
		t.Errorf("method should have been disabled")
	}
	m.Enable()
	if mhdlr.enabled == false {
		t.Errorf("method should have been enabled")
	}

	msg1 := m.GetRequestType()
	if msg1 != req {
		t.Errorf("retrieved wrong request type")
	}
	msg1 = m.GetResponseType()
	if msg1 != resp {
		t.Errorf("retrieved wrong response type")
	}
}

// TestMethodKvWrite
// Validate KV operation on Method invocation
func TestMethodKvWrite(t *testing.T) {
	req := mocks.NewFakeMessage("/requestmsg/A", true).(*mocks.FakeMessage)
	resp := mocks.NewFakeMessage("/responsmsg/A", true).(*mocks.FakeMessage)
	MustGetAPIServer()
	singletonAPISrv.runstate.running = true

	m := NewMethod(req, resp, "testm", "TestMethodKvWrite")
	reqmsg := TestType1{}

	// Set the same version as the apiServer
	md := metadata.Pairs(apisrv.RequestParamVersion, singletonAPISrv.version,
		apisrv.RequestParamMethod, "GET")
	ctx := metadata.NewIncomingContext(context.Background(), md)
	// this call should fail because the data is not added to the kvstore yet
	// fakemessage implements a dummy kv read/write
	if respmsg, _ := m.HandleInvocation(ctx, reqmsg); respmsg != nil {
		t.Errorf("Expecting err but succeeded")
	}
	if req.Kvreads != 1 {
		t.Errorf("Expecting [1] read but found [%v]", req.Kvreads)
	}
	// Now add the object and check
	md1 := metadata.Pairs(apisrv.RequestParamVersion, singletonAPISrv.version,
		apisrv.RequestParamMethod, "POST")
	ctx1 := metadata.NewIncomingContext(context.Background(), md1)
	m.HandleInvocation(ctx1, reqmsg)
	if req.Kvwrites != 1 {
		t.Errorf("Expecting [1] kvwrite but found [%v]", req.Kvwrites)
	}
	// Now modify the object and check
	md2 := metadata.Pairs(apisrv.RequestParamVersion, singletonAPISrv.version,
		apisrv.RequestParamMethod, "PUT")
	ctx2 := metadata.NewIncomingContext(context.Background(), md2)
	m.HandleInvocation(ctx2, reqmsg)
	if req.Kvwrites != 2 {
		t.Errorf("Expecting [2] kvwrite but found [%v]", req.Kvwrites)
	}
	// Now delete the object and check
	md3 := metadata.Pairs(apisrv.RequestParamVersion, singletonAPISrv.version,
		apisrv.RequestParamMethod, "DELETE")
	ctx3 := metadata.NewIncomingContext(context.Background(), md3)
	span := opentracing.StartSpan("delete")
	ctx3 = opentracing.ContextWithSpan(ctx3, span)
	if _, err := m.HandleInvocation(ctx3, reqmsg); err != nil {
		t.Errorf("Expecting success but failed %v", err)
	}
	if req.Kvdels != 1 {
		t.Errorf("Expecting [1] Kvdels but found [%v]", req.Kvdels)
	}
}

func TestMethodKvList(t *testing.T) {
	req := mocks.NewFakeMessage("/requestmsg/A", true).(*mocks.FakeMessage)
	resp := mocks.NewFakeMessage("/responsmsg/A", true).(*mocks.FakeMessage)

	MustGetAPIServer()
	singletonAPISrv.runstate.running = true
	m := NewMethod(req, resp, "testm", "TestMethodKvWrite")
	reqmsg := api.ListWatchOptions{}

	// Set the same version as the apiServer
	md := metadata.Pairs(apisrv.RequestParamVersion, singletonAPISrv.version,
		apisrv.RequestParamMethod, "LIST")
	ctx := metadata.NewIncomingContext(context.Background(), md)
	if respmsg, _ := m.HandleInvocation(ctx, reqmsg); respmsg != nil {
		t.Errorf("Expecting err but succeeded")
	}
	if resp.Kvlists != 1 {
		t.Errorf("Expecting [1] kvlist but found [%v]", req.Kvlists)
	}
}

func TestMapOper(t *testing.T) {
	req := mocks.NewFakeMessage("/requestmsg/A", true).(*mocks.FakeMessage)
	resp := mocks.NewFakeMessage("/responsmsg/A", true).(*mocks.FakeMessage)

	MustGetAPIServer()
	singletonAPISrv.runstate.running = true
	m := NewMethod(req, resp, "testm", "TestMethodKvWrite")
	md := metadata.Pairs(apisrv.RequestParamVersion, singletonAPISrv.version,
		apisrv.RequestParamMethod, "GET")
	// Test that the oper method is correct
	mhdlr := m.(*MethodHdlr)
	if mhdlr.mapOper(md) != apisrv.GetOper {
		t.Errorf("Found wrong oper type")
	}
	md = metadata.Pairs(apisrv.RequestParamVersion, "v1")
	m.WithOper("create")
	if mhdlr.mapOper(md) != apisrv.CreateOper {
		t.Errorf("Found wrong oper type")
	}
	m.WithOper("update")
	if mhdlr.mapOper(md) != apisrv.UpdateOper {
		t.Errorf("Found wrong oper type")
	}
	m.WithOper("get")
	if mhdlr.mapOper(md) != apisrv.GetOper {
		t.Errorf("Found wrong oper type")
	}
	m.WithOper("delete")
	if mhdlr.mapOper(md) != apisrv.DeleteOper {
		t.Errorf("Found wrong oper type")
	}
	m.WithOper("watch")
	if mhdlr.mapOper(md) != apisrv.WatchOper {
		t.Errorf("Found wrong oper type")
	}
}

func testTxnPreCommithook(ctx context.Context,
	kv kvstore.Interface,
	txn kvstore.Txn, key string, oper apisrv.APIOperType,
	i interface{}) (interface{}, bool, error) {

	txn.AddComparator(kvstore.Compare(kvstore.WithVersion("/requestmsg/A/NotThere"), "=", 0))
	return i, true, nil
}

func TestTxn(t *testing.T) {
	req := mocks.NewFakeMessage("/requestmsg/A", true).(*mocks.FakeMessage)
	resp := mocks.NewFakeMessage("/responsmsg/A", true).(*mocks.FakeMessage)
	MustGetAPIServer()
	singletonAPISrv.runstate.running = true

	// Add a few Pres and Posts and skip KV for testing
	m := NewMethod(req, resp, "testm", "TestMethodKvWrite").WithPreCommitHook(testTxnPreCommithook)
	reqmsg := &compliance.TestObj{TypeMeta: api.TypeMeta{Kind: "TestObj"}, ObjectMeta: api.ObjectMeta{Name: "testObj1"}}
	md := metadata.Pairs(apisrv.RequestParamVersion, singletonAPISrv.version,
		apisrv.RequestParamMethod, "POST")
	ctx := metadata.NewIncomingContext(context.Background(), md)
	m.HandleInvocation(ctx, reqmsg)
	if req.Txnwrites != 1 {
		t.Fatalf("Txn Write: expecting [1] saw [%d]", req.Txnwrites)
	}
	// Modify the same object
	md1 := metadata.Pairs(apisrv.RequestParamVersion, singletonAPISrv.version,
		apisrv.RequestParamMethod, "PUT")
	ctx1 := metadata.NewIncomingContext(context.Background(), md1)
	req.Kvpath = mocks.TxnTestKey
	m.HandleInvocation(ctx1, reqmsg)
	if req.Txnwrites != 2 {
		t.Fatalf("Txn Write: expecting [2] saw [%d]", req.Txnwrites)
	}
	// Delete the Object
	md2 := metadata.Pairs(apisrv.RequestParamVersion, singletonAPISrv.version,
		apisrv.RequestParamMethod, "DELETE")
	ctx2 := metadata.NewIncomingContext(context.Background(), md2)
	_, err := m.HandleInvocation(ctx2, reqmsg)
	if err != nil {
		t.Fatalf("Invocation failed (%s)", err)
	}
	if req.Txndels != 1 {
		t.Fatalf("Txn Del: expecting [1] saw [%d]", req.Txndels)
	}
}
