package apisrvpkg

import (
	"context"
	"testing"

	opentracing "github.com/opentracing/opentracing-go"
	"github.com/pensando/sw/api"
	apisrv "github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/utils/kvstore"
	"google.golang.org/grpc/metadata"
)

// fakeMethod is used as mock Method for testing.
type fakeMethod struct {
	pres    int
	posts   int
	skipkv  bool
	enabled bool
}

func (m *fakeMethod) Enable()                        { m.enabled = true }
func (m *fakeMethod) Disable()                       { m.enabled = false }
func (m *fakeMethod) WithRateLimiter() apisrv.Method { return m }

func (m *fakeMethod) WithPreCommitHook(fn apisrv.PreCommitFunc) apisrv.Method       { return m }
func (m *fakeMethod) WithPostCommitHook(fn apisrv.PostCommitFunc) apisrv.Method     { return m }
func (m *fakeMethod) WithResponseWriter(fn apisrv.ResponseWriterFunc) apisrv.Method { return m }
func (m *fakeMethod) WithOper(oper apisrv.APIOperType) apisrv.Method                { return m }
func (m *fakeMethod) WithVersion(ver string) apisrv.Method                          { return m }
func (m *fakeMethod) GetRequestType() apisrv.Message                                { return nil }
func (m *fakeMethod) GetResponseType() apisrv.Message                               { return nil }
func (m *fakeMethod) HandleInvocation(ctx context.Context, i interface{}) (interface{}, error) {
	return nil, nil
}

func newFakeMethod(skipkv bool) apisrv.Method {
	return &fakeMethod{skipkv: skipkv}
}

func (m *fakeMethod) precommitFunc(ctx context.Context, kvs kvstore.Interface, txn kvstore.Txn, key string, oper apisrv.APIOperType, i interface{}) (interface{}, bool, error) {
	m.pres++
	if m.skipkv {
		return i, false, nil
	}
	return i, true, nil
}

func (m *fakeMethod) postcommitfFunc(ctx context.Context, oper apisrv.APIOperType, i interface{}) {
	m.posts++
}

func (m *fakeMethod) respWriterFunc(ctx context.Context, kvs kvstore.Interface, prefix string, i interface{}, o interface{}, oper apisrv.APIOperType) (interface{}, error) {
	return "TestResponse", nil
}

// TestMethodWiths
// Test registration of various hooks to method.
// Check all precommit and post commit functions are called and skipping
// KV operation feedback from hook is honored
func TestMethodWiths(t *testing.T) {
	req := newFakeMessage("TestType1", true).(*fakeMessage)
	resp := newFakeMessage("TestType2", true).(*fakeMessage)
	f := newFakeMethod(true).(*fakeMethod)
	// Add a few Pres and Posts and skip KV for testing
	m := NewMethod(req, resp, "testm", "TestMethodWiths").WithVersion("v1").WithPreCommitHook(f.precommitFunc).WithPreCommitHook(f.precommitFunc).WithPreCommitHook(f.precommitFunc)
	m = m.WithPostCommitHook(f.postcommitfFunc).WithPostCommitHook(f.postcommitfFunc).WithResponseWriter(f.respWriterFunc)
	m = m.WithOper("POST").WithVersion("Vtest")
	reqmsg := TestType1{}
	md := metadata.Pairs(apisrv.RequestParamVersion, singletonAPISrv.version,
		apisrv.RequestParamMethod, "GET")
	mhdlr := m.(*MethodHdlr)
	if mhdlr.version != "Vtest" || mhdlr.oper != "POST" {
		t.Errorf("Flags not set correction ver[%v] oper[%v]", mhdlr.version, mhdlr.oper)
	}
	ctx := metadata.NewContext(context.Background(), md)
	respmsg, _ := m.HandleInvocation(ctx, reqmsg)

	if r, ok := respmsg.(string); ok {
		if r != "TestResponse" {
			t.Errorf("Expecting a result of [TestResponse] got [%v]", r)
		}
	} else {
		t.Errorf("Result not overriden")
	}

	if f.pres != 3 {
		t.Errorf("Expecting 3 precommit invocations found %v", f.pres)
	}
	if f.posts != 2 {
		t.Errorf("Expecting 3 precommit invocations found %v", f.posts)
	}
	// There should be no KV operations involved since the precommit skipped KV
	if req.kvreads != 0 {
		t.Errorf("Expecting no KV reads but found %v", req.kvreads)
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
	req := newFakeMessage("/requestmsg/A", true).(*fakeMessage)
	resp := newFakeMessage("/responsmsg/A", true).(*fakeMessage)

	// Add a few Pres and Posts and skip KV for testing
	m := NewMethod(req, resp, "testm", "TestMethodKvWrite")
	reqmsg := TestType1{}

	// Set the same version as the apiServer
	md := metadata.Pairs(apisrv.RequestParamVersion, singletonAPISrv.version,
		apisrv.RequestParamMethod, "GET")
	ctx := metadata.NewContext(context.Background(), md)
	if respmsg, _ := m.HandleInvocation(ctx, reqmsg); respmsg != nil {
		t.Errorf("Expecting err but succeded")
	}
	if req.kvreads != 1 {
		t.Errorf("Expecting [1] read but found [%v]", req.kvreads)
	}
	// Now add the object and check
	md1 := metadata.Pairs(apisrv.RequestParamVersion, singletonAPISrv.version,
		apisrv.RequestParamMethod, "POST")
	ctx1 := metadata.NewContext(context.Background(), md1)
	m.HandleInvocation(ctx1, reqmsg)
	if req.kvwrites != 1 {
		t.Errorf("Expecting [1] kvwrite but found [%v]", req.kvwrites)
	}
	// Now modify the object and check
	md2 := metadata.Pairs(apisrv.RequestParamVersion, singletonAPISrv.version,
		apisrv.RequestParamMethod, "PUT")
	ctx2 := metadata.NewContext(context.Background(), md2)
	m.HandleInvocation(ctx2, reqmsg)
	if req.kvwrites != 2 {
		t.Errorf("Expecting [1] kvwrite but found [%v]", req.kvwrites)
	}
	// Now delete the object and check
	md3 := metadata.Pairs(apisrv.RequestParamVersion, singletonAPISrv.version,
		apisrv.RequestParamMethod, "DELETE")
	ctx3 := metadata.NewContext(context.Background(), md3)
	span := opentracing.StartSpan("delete")
	ctx3 = opentracing.ContextWithSpan(ctx3, span)
	if _, err := m.HandleInvocation(ctx3, reqmsg); err != nil {
		t.Errorf("Expecting success but failed %v", err)
	}
	if req.kvdels != 1 {
		t.Errorf("Expecting [1] kvdels but found [%v]", req.kvdels)
	}
}

func TestMethodKvList(t *testing.T) {
	req := newFakeMessage("/requestmsg/A", true).(*fakeMessage)
	resp := newFakeMessage("/responsmsg/A", true).(*fakeMessage)

	m := NewMethod(req, resp, "testm", "TestMethodKvWrite")
	reqmsg := api.ListWatchOptions{}

	// Set the same version as the apiServer
	md := metadata.Pairs(apisrv.RequestParamVersion, singletonAPISrv.version,
		apisrv.RequestParamMethod, "LIST")
	ctx := metadata.NewContext(context.Background(), md)
	if respmsg, _ := m.HandleInvocation(ctx, reqmsg); respmsg != nil {
		t.Errorf("Expecting err but succeded")
	}
	if resp.kvlists != 1 {
		t.Errorf("Expecting [1] kvlist but found [%v]", req.kvlists)
	}
}

func TestMapOper(t *testing.T) {
	req := newFakeMessage("/requestmsg/A", true).(*fakeMessage)
	resp := newFakeMessage("/responsmsg/A", true).(*fakeMessage)

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
	req := newFakeMessage("/requestmsg/A", true).(*fakeMessage)
	resp := newFakeMessage("/responsmsg/A", true).(*fakeMessage)

	// Add a few Pres and Posts and skip KV for testing
	m := NewMethod(req, resp, "testm", "TestMethodKvWrite").WithPreCommitHook(testTxnPreCommithook)
	reqmsg := &kvstore.TestObj{TypeMeta: api.TypeMeta{Kind: "TestObj"}, ObjectMeta: api.ObjectMeta{Name: "testObj1"}}
	md := metadata.Pairs(apisrv.RequestParamVersion, singletonAPISrv.version,
		apisrv.RequestParamMethod, "POST")
	ctx := metadata.NewContext(context.Background(), md)
	m.HandleInvocation(ctx, reqmsg)
	if req.txnwrites != 1 {
		t.Fatalf("Txn Write: expecting [1] saw [%d]", req.txnwrites)
	}
	// Modify the same object
	md1 := metadata.Pairs(apisrv.RequestParamVersion, singletonAPISrv.version,
		apisrv.RequestParamMethod, "PUT")
	ctx1 := metadata.NewContext(context.Background(), md1)
	req.kvpath = txnTestKey
	m.HandleInvocation(ctx1, reqmsg)
	if req.txnwrites != 2 {
		t.Fatalf("Txn Write: expecting [2] saw [%d]", req.txnwrites)
	}
	// Delete the Object
	md2 := metadata.Pairs(apisrv.RequestParamVersion, singletonAPISrv.version,
		apisrv.RequestParamMethod, "DELETE")
	ctx2 := metadata.NewContext(context.Background(), md2)
	_, err := m.HandleInvocation(ctx2, reqmsg)
	if err != nil {
		t.Fatalf("Invocation failed (%s)", err)
	}
	if req.txndels != 1 {
		t.Fatalf("Txn Write: expecting [2] saw [%d]", req.txnwrites)
	}
}
