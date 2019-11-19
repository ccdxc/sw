package requirement

import (
	"context"
	"errors"
	"testing"

	"github.com/pensando/sw/api/cache/mocks"
	"github.com/pensando/sw/api/graph"
	"github.com/pensando/sw/api/interfaces"
)

func TestRequirements(t *testing.T) {
	ctx := context.Background()
	c, err := graph.NewCayleyStore()
	if err != nil {
		t.Fatalf("could not create store (%s)", err)
	}
	defer c.Close()
	kvs := &mocks.FakeOverlay{}
	req := NewRequirementSet(c, kvs, nil).(*apiRequirements)
	if r := req.NewRefRequirement(apiintf.CreateOper, "/test/obj1", nil); r == nil {
		t.Fatalf("could not create ref requirement")
	}

	if len(req.reqs) != 1 {
		t.Fatalf("unexpected number of requirements [%d]", len(req.reqs))
	}

	// Add another request and check there is still one
	if r := req.NewRefRequirement(apiintf.CreateOper, "/test/obj1", nil); r == nil {
		t.Fatalf("could not create ref requirement")
	}

	if len(req.reqs) != 1 {
		t.Fatalf("unexpected number of requirements [%d]", len(req.reqs))
	}

	// Add second object and make sure we have 2
	if r := req.NewRefRequirement(apiintf.CreateOper, "/test/obj2", nil); r == nil {
		t.Fatalf("could not create ref requirement")
	}

	if len(req.reqs) != 2 {
		t.Fatalf("unexpected number of requirements [%d]", len(req.reqs))
	}

	req.reqs = []apiintf.Requirement{&mocks.FakeRequirement{}, &mocks.FakeRequirement{}}
	req.reqIdx = map[string]int{"/test/obj1": 0, "/test/obj2": 1}
	t.Logf("created requiremtn [%v]", req.String())
	if errs := req.Check(ctx); errs != nil {
		t.Fatalf("Check failed [%v]", errs)
	}
	txn := &mocks.FakeTxn{}
	if errs := req.Apply(ctx, txn, nil); errs != nil {
		t.Fatalf("Apply failed [%v]", errs)
	}

	if errs := req.Finalize(ctx); errs != nil {
		t.Fatalf("Finalize failed [%v]", errs)
	}
	if len(req.reqs) != 2 {
		t.Fatalf("unexpected number of requirements [%d]", len(req.reqs))
	}

	for i := range req.reqs {
		r := req.reqs[i].(*mocks.FakeRequirement)
		if r.ApplyCalled != 1 || r.CheckCalled != 1 || r.FinalizeCalled != 1 {
			t.Fatalf("Unexpected number of calls [%d%d/%d]", r.CheckCalled, r.ApplyCalled, r.FinalizeCalled)
		}
		r.CheckCalled, r.ApplyCalled, r.FinalizeCalled = 0, 0, 0
	}

	req.reqs[0].(*mocks.FakeRequirement).RetErr = errors.New("fake error")
	if errs := req.Check(ctx); errs == nil {
		t.Fatalf("Check expected to fail ")
	} else {
		if len(errs) != 1 {
			t.Fatalf("unexpected number of errors [%+v]", errs)
		}
	}
	if errs := req.Apply(ctx, txn, nil); errs == nil {
		t.Fatalf("Apply expected to fail")
	} else {
		if len(errs) != 1 {
			t.Fatalf("unexpected number of errors [%+v]", errs)
		}
	}
	if errs := req.Finalize(ctx); errs == nil {
		t.Fatalf("Finalize expected to fail ")
	} else {
		if len(errs) != 1 {
			t.Fatalf("unexpected number of errors [%+v]", errs)
		}
	}
	if len(req.reqs) != 2 {
		t.Fatalf("unexpected number of requirements [%d]", len(req.reqs))
	}

	for i := range req.reqs {
		r := req.reqs[i].(*mocks.FakeRequirement)
		if r.ApplyCalled != 1 || r.CheckCalled != 1 || r.FinalizeCalled != 1 {
			t.Fatalf("Unexpected number of calls [%d%d/%d]", r.CheckCalled, r.ApplyCalled, r.FinalizeCalled)
		}
		r.CheckCalled, r.ApplyCalled, r.FinalizeCalled = 0, 0, 0
	}

	// Test CheckOne
	_, found := req.CheckOne(ctx, "/test/dummy")
	if found {
		t.Fatalf("not expecting to find requirement")
	}
	_, found = req.CheckOne(ctx, "/test/obj1")
	if !found {
		t.Fatalf("not expecting to find requirement")
	}
	r := req.reqs[0].(*mocks.FakeRequirement)
	if r.CheckCalled != 1 || r.ApplyCalled != 0 || r.FinalizeCalled != 0 {
		t.Fatalf("invalid counts on mock requirement [%d/%d/%d]", r.CheckCalled, r.ApplyCalled, r.FinalizeCalled)
	}
	r = req.reqs[1].(*mocks.FakeRequirement)
	if r.CheckCalled != 0 || r.ApplyCalled != 0 || r.FinalizeCalled != 0 {
		t.Fatalf("invalid counts on mock requirement [%d/%d/%d]", r.CheckCalled, r.ApplyCalled, r.FinalizeCalled)
	}
	req.Clear(ctx)
	if len(req.reqs) != 0 {
		t.Fatalf("did not clear requirements [%v]", len(req.reqs))
	}

}
