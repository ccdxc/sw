package apigwpkg

import (
	"context"
	"testing"
)

type testHooks struct {
	preAuthNCnt, preAuthZCnt int
	preCallCnt, postCallCnt  int
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
	return ctx, t.retObj, skip, t.retErr
}

func (t *testHooks) preAuthZHook(ctx context.Context, in interface{}) (context.Context, interface{}, error) {
	t.preAuthZCnt++
	return ctx, t.retObj, t.retErr
}

func (t *testHooks) preCallHook(ctx context.Context, in interface{}) (context.Context, interface{}, bool, error) {
	t.preCallCnt++
	skip := false
	if t.skipCallFn != nil {
		skip = t.skipCallFn()
	}
	return ctx, t.retObj, skip, t.retErr
}

func (t *testHooks) postCallHook(ctx context.Context, in interface{}) (context.Context, interface{}, error) {
	t.postCallCnt++
	return ctx, t.retObj, t.retErr
}

func TestSvcProfile(t *testing.T) {
	prof := NewServiceProfile(nil)
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

	mprof := NewServiceProfile(prof)
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

}
