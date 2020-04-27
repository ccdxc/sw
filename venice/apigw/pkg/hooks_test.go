package apigwpkg

import (
	"context"
	"testing"

	"github.com/pensando/sw/api/generated/audit"

	apiintf "github.com/pensando/sw/api/interfaces"
)

type testHooks struct {
	preAuthNCnt, preAuthZCnt int
	preCallCnt, postCallCnt  int
	useRetAuthnCtx           bool
	retAuthnCtx              context.Context
	retAuthzCtx              context.Context
	retObj                   interface{}
	retErr                   error
	skipAuthFn               func() bool
	skipCallFn               func() bool
}

func (t *testHooks) preAuthNHook(ctx context.Context, in interface{}) (context.Context, interface{}, bool, error) {
	t.preAuthNCnt++
	skip := false
	if t.skipAuthFn != nil {
		skip = t.skipAuthFn()
	}
	if t.useRetAuthnCtx {
		return t.retAuthnCtx, t.retObj, skip, t.retErr
	}
	return ctx, t.retObj, skip, t.retErr
}

func (t *testHooks) preAuthZHook(ctx context.Context, in interface{}) (context.Context, interface{}, error) {
	t.preAuthZCnt++
	return t.retAuthzCtx, t.retObj, t.retErr
}

func (t *testHooks) preCallHook(ctx context.Context, in, out interface{}) (context.Context, interface{}, interface{}, bool, error) {
	t.preCallCnt++
	skip := false
	if t.skipCallFn != nil {
		skip = t.skipCallFn()
	}
	return ctx, t.retObj, out, skip, t.retErr
}

func (t *testHooks) postCallHook(ctx context.Context, in interface{}) (context.Context, interface{}, error) {
	t.postCallCnt++
	return ctx, t.retObj, t.retErr
}

func TestSvcProfile(t *testing.T) {
	prof := NewServiceProfile(nil, "", "", apiintf.UnknownOper)
	mock := &testHooks{}
	prof.AddPreAuthZHook(mock.preAuthZHook)
	prof.AddPreAuthZHook(mock.preAuthZHook)
	prof.AddPreAuthZHook(mock.preAuthZHook)
	prof.AddPreCallHook(mock.preCallHook)
	prof.AddPreCallHook(mock.preCallHook)
	prof.AddPostCallHook(mock.postCallHook)

	pa := prof.PreAuthZHooks()
	if len(pa) != 3 {
		t.Errorf("expecting 3 pre auth hooks got %d", len(pa))
	}
	pc := prof.PreCallHooks()
	if len(pc) != 2 {
		t.Errorf("expecting 2 pre call hooks got %d", len(pc))
	}
	tc := prof.PostCallHooks()
	if len(tc) != 1 {
		t.Errorf("expecting 1 pre call hooks got %d", len(tc))
	}

	mprof := NewServiceProfile(prof, "test", "testGroup", apiintf.CreateOper)
	pa = mprof.PreAuthZHooks()
	if len(pa) != 3 {
		t.Errorf("expecting 3 pre auth hooks got %d", len(pa))
	}
	pc = mprof.PreCallHooks()
	if len(pc) != 2 {
		t.Errorf("expecting 2 pre call hooks got %d", len(pc))
	}
	tc = mprof.PostCallHooks()
	if len(tc) != 1 {
		t.Errorf("expecting 1 pre call hooks got %d", len(tc))
	}

	if mprof.GetKind() != "test" {
		t.Errorf("got wrong kind [%s]", mprof.GetKind())
	}
	if mprof.GetAPIGoup() != "testGroup" {
		t.Errorf("got wrong group [%s]", mprof.GetAPIGoup())
	}
	if mprof.GetOper() != apiintf.CreateOper {
		t.Errorf("got wrong oper [%s]", mprof.GetOper())
	}

	// Set some New hooks
	mprof.AddPreAuthZHook(mock.preAuthZHook)
	mprof.AddPreCallHook(mock.preCallHook)
	mprof.AddPreCallHook(mock.preCallHook)
	mprof.AddPreCallHook(mock.preCallHook)
	mprof.AddPostCallHook(mock.postCallHook)
	mprof.AddPostCallHook(mock.postCallHook)
	mprof.AddPostCallHook(mock.postCallHook)
	mprof.AddPostCallHook(mock.postCallHook)

	pa = mprof.PreAuthZHooks()
	if len(pa) != 1 {
		t.Errorf("expecting 3 pre auth hooks got %d", len(pa))
	}
	pc = mprof.PreCallHooks()
	if len(pc) != 3 {
		t.Errorf("expecting 2 pre call hooks got %d", len(pc))
	}
	tc = mprof.PostCallHooks()
	if len(tc) != 4 {
		t.Errorf("expecting 1 pre call hooks got %d", len(tc))
	}

	// When no audit level is set
	_, ok := mprof.GetAuditLevel()
	if ok {
		t.Errorf("expecting ok to be false")
	}

	err := mprof.SetAuditLevel("JunkValue")
	if err == nil {
		t.Errorf("Set Audit level is expected to fail")
	}

	err = mprof.SetAuditLevel(audit.Level_RequestResponse.String())
	if err != nil {
		t.Errorf("Set Audit level is expected to pass (%s)", err)
	}

	lvl, ok := mprof.GetAuditLevel()
	if !ok {
		t.Errorf("expecting ok to be true")
	}
	if lvl != audit.Level_RequestResponse.String() {
		t.Errorf("unexpected level [%v]", lvl)
	}
}
