package requirement

import (
	"context"
	"fmt"
	"testing"

	"github.com/pensando/sw/api/api_test"
	"github.com/pensando/sw/api/cache/mocks"
	"github.com/pensando/sw/api/graph"
	"github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/api/utils"
	"github.com/pensando/sw/venice/utils/runtime"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestConsUpdateReq(t *testing.T) {
	ctx := context.Background()
	c, err := graph.NewCayleyStore()
	if err != nil {
		t.Fatalf("could not create store (%s)", err)
	}
	defer c.Close()
	kvs := &mocks.FakeOverlay{}
	reqs := NewRequirementSet(c, kvs, nil).(*apiRequirements)
	var CallID, callID1, callID2, callID3, func1, func2, func3 int
	obj := &apitest.TestObj{}
	obj.ResourceVersion = "10"
	creqs := []apiintf.ConstUpdateItem{
		{Key: "/abc1", Func: func(in runtime.Object) (runtime.Object, error) {
			callID1 = CallID
			CallID++
			func1++
			return in, nil
		}, Into: obj},
		{Key: "/abc2", Func: func(in runtime.Object) (runtime.Object, error) {
			callID2 = CallID
			CallID++
			func2++
			return in, nil
		}, Into: obj},
		{Key: "/abc3", ResourceVersion: "12", Func: func(in runtime.Object) (runtime.Object, error) {
			callID3 = CallID
			CallID++
			func3++
			return in, nil
		}, Into: obj},
	}

	if r := reqs.NewConsUpdateRequirement(creqs); r == nil {
		t.Fatalf("failed to create new requirement")
	} else {
		t.Logf("created new requirement [%s]", r.String())
	}

	retObj := apitest.TestObj{}
	retObj.ResourceVersion = "10"
	cache := &mocks.FakeCache{FakeKvStore: mocks.FakeKvStore{}}

	nctx := apiutils.SetRequirements(ctx, reqs)
	txn := &mocks.FakeTxn{}
	cache.FakeKvStore.Getfn = func(ctx context.Context, key string, into runtime.Object) error {
		return fmt.Errorf("not found")
	}
	// Apply without object in cache
	errs := reqs.Apply(ctx, txn, cache)
	Assert(t, errs != nil, "expecting errors on Apply")

	// positive case
	cache.FakeKvStore.Getfn = func(ctx context.Context, key string, into runtime.Object) error {
		fmt.Printf("calling Get[%+v]", into)
		robj := into.(*apitest.TestObj)
		*robj = retObj
		return nil
	}
	errs = reqs.Apply(nctx, txn, cache)
	Assert(t, errs == nil, "not expecting errors")
	Assert(t, callID1 == 0, "unexpected callid for func1")
	Assert(t, callID2 == 1, "unexpected callid for func2")
	Assert(t, callID3 == 2, "unexpected callid for func3")
	Assert(t, func1 == 1, "unexpected number of calls for func1")
	Assert(t, func2 == 1, "unexpected number of calls for func2")
	Assert(t, func3 == 1, "unexpected number of calls for func3")
	Assert(t, len(txn.Cmps) == 3, "unexpected number of ops in txn (%v)", len(txn.Ops))
	Assert(t, txn.Cmps[0].Version == 10, "Unexpected resource version comparator [%v]", txn.Cmps[0].Version)
	Assert(t, txn.Cmps[1].Version == 10, "Unexpected resource version comparator [%v]", txn.Cmps[1].Version)
	Assert(t, txn.Cmps[2].Version == 12, "Unexpected resource version comparator [%v]", txn.Cmps[2].Version)
}
