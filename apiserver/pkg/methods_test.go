package apisrvpkg

import (
	"context"
	"testing"

	apisrv "github.com/pensando/sw/apiserver"
	"github.com/pensando/sw/utils/kvstore"
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
func (m *fakeMethod) WithOper(oper string) apisrv.Method                            { return m }
func (m *fakeMethod) WithVersion(ver string) apisrv.Method                          { return m }
func (m *fakeMethod) GetRequestType() apisrv.Message                                { return nil }
func (m *fakeMethod) GetResponseType() apisrv.Message                               { return nil }
func (m *fakeMethod) HandleInvocation(ctx context.Context, i interface{}) (interface{}, error) {
	return nil, nil
}

func newFakeMethod(skipkv bool) apisrv.Method {
	return &fakeMethod{skipkv: skipkv}
}

func (m *fakeMethod) precommitFunc(ctx context.Context, oper string, i interface{}) (interface{}, bool) {
	m.pres++
	if m.skipkv {
		return i, false
	}
	return i, true
}

func (m *fakeMethod) postcommitfFunc(ctx context.Context, oper string, i interface{}) {
	m.posts++
}

func (m *fakeMethod) respWriterFunc(ctx context.Context, kvs kvstore.Interface, prefix string, i interface{}, o interface{}, oper string) (interface{}, error) {
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
	md := metadata.Pairs("req-version", singletonApiSrv.version,
		"req-method", "GET")
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

	//f := newFakeMethod(true).(*fakeMethod)
	// Add a few Pres and Posts and skip KV for testing
	m := NewMethod(req, resp, "testm", "TestMethodKvWrite")
	reqmsg := TestType1{}

	// Set the same version as the apiServer
	md := metadata.Pairs("req-version", singletonApiSrv.version,
		"req-method", "GET")
	ctx := metadata.NewContext(context.Background(), md)
	if respmsg, _ := m.HandleInvocation(ctx, reqmsg); respmsg != nil {
		t.Errorf("Expecting err but succeded")
	}
	if req.kvreads != 1 {
		t.Errorf("Expecting [1] read but found [%v]", req.kvreads)
	}
	// Now add the object and check
	md1 := metadata.Pairs("req-version", singletonApiSrv.version,
		"req-method", "POST")
	ctx1 := metadata.NewContext(context.Background(), md1)
	m.HandleInvocation(ctx1, reqmsg)
	if req.kvwrites != 1 {
		t.Errorf("Expecting [1] kvwrite but found [%v]", req.kvwrites)
	}

}

func TestMapOper(t *testing.T) {
	req := newFakeMessage("/requestmsg/A", true).(*fakeMessage)
	resp := newFakeMessage("/responsmsg/A", true).(*fakeMessage)

	//f := newFakeMethod(true).(*fakeMethod)
	// Add a few Pres and Posts and skip KV for testing
	m := NewMethod(req, resp, "testm", "TestMethodKvWrite")
	md := metadata.Pairs("req-version", singletonApiSrv.version,
		"req-method", "GET")
	// Test that the oper method is correct
	mhdlr := m.(*MethodHdlr)
	if mhdlr.mapOper(md) != "GET" {
		t.Errorf("Found wrong oper type")
	}
	md = metadata.Pairs("req-version", "v1")
	m.WithOper("create")
	if mhdlr.mapOper(md) != "POST" {
		t.Errorf("Found wrong oper type")
	}
	m.WithOper("update")
	if mhdlr.mapOper(md) != "PUT" {
		t.Errorf("Found wrong oper type")
	}
	m.WithOper("get")
	if mhdlr.mapOper(md) != "GET" {
		t.Errorf("Found wrong oper type")
	}
	m.WithOper("delete")
	if mhdlr.mapOper(md) != "DELETE" {
		t.Errorf("Found wrong oper type")
	}
}
