package requirement

import (
	"context"
	"reflect"
	"testing"

	"github.com/pensando/sw/api/cache/mocks"
	"github.com/pensando/sw/api/graph"
	"github.com/pensando/sw/api/interfaces"
)

func makeRefs(in map[string][]string, tpe apiintf.ReferenceType) map[string]apiintf.ReferenceObj {
	ret := make(map[string]apiintf.ReferenceObj)
	for k, v := range in {
		ret[k] = apiintf.ReferenceObj{
			RefType: tpe,
			Refs:    v,
		}
	}
	return ret
}

func TestReqReferencesCreateOper(t *testing.T) {
	ctx := context.Background()
	c, err := graph.NewCayleyStore()
	if err != nil {
		t.Fatalf("could not create store (%s)", err)
	}
	defer c.Close()
	kvs := &mocks.FakeCache{}
	node := graph.Node{
		This: "/test/obj1",
		Dir:  graph.RefOut,
		Refs: map[string][]string{
			"spec.field1": {
				"/test/obj2",
				"/test/obj3",
			},
			"spec.field2": {
				"/test/obj3",
			},
		},
		WeakRefs:     make(map[string][]string),
		SelectorRefs: make(map[string][]string),
	}

	req := NewReferenceReq(apiintf.CreateOper, "/test/obj1", makeRefs(node.Refs, apiintf.NamedReference), c, kvs)
	if err := req.Check(ctx); err == nil {
		t.Fatalf("should have failed but got no errors")
	}
	statMap := map[string]apiintf.ObjectStat{
		"/test/obj1": {Key: "/test/obj1", Valid: true, Revision: 9},
		"/test/obj2": {Key: "/test/obj2", Valid: true, Revision: 10},
		"/test/obj3": {Key: "/test/obj3", Valid: true, Revision: 11},
	}

	kvs.StatFn = func(keys []string) []apiintf.ObjectStat {
		var ret []apiintf.ObjectStat
		for i := range keys {
			ret = append(ret, statMap[keys[i]])
		}
		return ret
	}
	if err := req.Check(ctx); err != nil {
		t.Fatalf("should have succeeded but got errors (%s)", err)
	}

	statMap["/test/obj2"] = apiintf.ObjectStat{Key: "/test/obj2", Valid: false, Revision: 10}
	if err := req.Check(ctx); err == nil {
		t.Fatalf("should have failed but got no errors")
	}

	txn := &mocks.FakeTxn{}
	if err := req.Apply(ctx, txn, nil); err != nil {
		t.Fatalf("failed to create transaction (%s)", err)
	}
	if len(txn.Cmps) != 2 {
		t.Fatalf("txn does not have right number of the comparators got (%d)", len(txn.Cmps))
	}
	exp := map[string]bool{
		"/test/obj2": false,
		"/test/obj3": false,
	}
	for _, c := range txn.Cmps {
		if v, ok := exp[c.Key]; !ok || v {
			t.Fatalf("unexpected or repeated comparator [%v] [%v]", v, ok)
		}
		exp[c.Key] = true
	}
	for k, v := range exp {
		if !v {
			t.Fatalf("key [%v] was not found in txn", k)
		}
	}
	n := c.References("/test/obj1")
	if n != nil {
		t.Fatalf("Found node while there should be none [%+v]", n)
	}
	if err := req.Finalize(ctx); err != nil {
		t.Fatalf("failed to finalize requirement (%s)", err)
	}
	n = c.References("/test/obj1")
	if n == nil {
		t.Fatalf("Failed to find node")
	}
	if !reflect.DeepEqual(n, &node) {
		t.Fatalf("curent node does not match expectation: got\n[%+v]\nwant\n[%+v]", n, node)
	}
}

func TestReqReferencesUpdateOper(t *testing.T) {
	ctx := context.Background()
	c, err := graph.NewCayleyStore()
	if err != nil {
		t.Fatalf("could not create store (%s)", err)
	}
	defer c.Close()
	kvs := &mocks.FakeCache{}
	node := graph.Node{
		This: "/test/obj1",
		Dir:  graph.RefOut,
		Refs: map[string][]string{
			"spec.field1": {
				"/test/obj2",
				"/test/obj3",
			},
			"spec.field2": {
				"/test/obj3",
			},
		},
		WeakRefs:     make(map[string][]string),
		SelectorRefs: make(map[string][]string),
	}

	req := NewReferenceReq(apiintf.UpdateOper, "/test/obj1", makeRefs(node.Refs, apiintf.NamedReference), c, kvs)
	if err := req.Check(ctx); err == nil {
		t.Fatalf("should have failed but got no errors")
	}
	statMap := map[string]apiintf.ObjectStat{
		"/test/obj1": {Key: "/test/obj1", Valid: true, Revision: 9},
		"/test/obj2": {Key: "/test/obj2", Valid: true, Revision: 10},
		"/test/obj3": {Key: "/test/obj3", Valid: true, Revision: 11},
	}

	kvs.StatFn = func(keys []string) []apiintf.ObjectStat {
		var ret []apiintf.ObjectStat
		for i := range keys {
			ret = append(ret, statMap[keys[i]])
		}
		return ret
	}
	if err := req.Check(ctx); err != nil {
		t.Fatalf("should have succeeded but got errors (%s)", err)
	}

	statMap["/test/obj2"] = apiintf.ObjectStat{Key: "/test/obj2", Valid: false, Revision: 10}
	if err := req.Check(ctx); err == nil {
		t.Fatalf("should have failed but got no errors")
	}

	txn := &mocks.FakeTxn{}
	if err := req.Apply(ctx, txn, nil); err != nil {
		t.Fatalf("failed to create transaction (%s)", err)
	}
	if len(txn.Cmps) != 2 {
		t.Fatalf("txn does not have right number of the comparators got (%d)", len(txn.Cmps))
	}
	exp := map[string]bool{
		"/test/obj2": false,
		"/test/obj3": false,
	}
	for _, c := range txn.Cmps {
		if v, ok := exp[c.Key]; !ok || v {
			t.Fatalf("unexpected or repeated comparator [%v] [%v]", v, ok)
		}
		exp[c.Key] = true
	}
	for k, v := range exp {
		if !v {
			t.Fatalf("key [%v] was not found in txn", k)
		}
	}
	n := c.References("/test/obj1")
	if n != nil {
		t.Fatalf("Found node while there should be none [%+v]", n)
	}
	if err := req.Finalize(ctx); err != nil {
		t.Fatalf("failed to finalize requirement (%s)", err)
	}
	n = c.References("/test/obj1")
	if n == nil {
		t.Fatalf("Failed to find node")
	}
	if !reflect.DeepEqual(n, &node) {
		t.Fatalf("curent node does not match expectation: got\n[%+v]\nwant\n[%+v]", n, node)
	}
}

func TestReqReferencesDeleteOper(t *testing.T) {
	ctx := context.Background()
	c, err := graph.NewCayleyStore()
	if err != nil {
		t.Fatalf("could not create store (%s)", err)
	}
	defer c.Close()
	kvs := &mocks.FakeCache{}
	node := graph.Node{
		This: "/test/obj1",
		Refs: map[string][]string{
			"spec.field1": {
				"/test/obj2",
				"/test/obj3",
			},
			"spec.field2": {
				"/test/obj3",
			},
		},
		WeakRefs:     make(map[string][]string),
		SelectorRefs: make(map[string][]string),
	}
	if err := c.UpdateNode(&node); err != nil {
		t.Fatalf("Update node failed (%s)", err)
	}
	req := NewReferenceReq(apiintf.DeleteOper, "/test/obj1", makeRefs(node.Refs, apiintf.NamedReference), c, kvs)
	if err := req.Check(ctx); err != nil {
		t.Fatalf("should have succeeded errors (%s)", err)
	}
	statMap := map[string]apiintf.ObjectStat{
		"/test/obj1": {Key: "/test/obj1", Valid: true, Revision: 9},
		"/test/obj2": {Key: "/test/obj2", Valid: true, Revision: 10},
		"/test/obj3": {Key: "/test/obj3", Valid: true, Revision: 11},
	}

	kvs.StatFn = func(keys []string) []apiintf.ObjectStat {
		var ret []apiintf.ObjectStat
		for i := range keys {
			ret = append(ret, statMap[keys[i]])
		}
		return ret
	}
	if err := req.Check(ctx); err != nil {
		t.Fatalf("should have succeeded but got errors (%s)", err)
	}

	req1 := NewReferenceReq(apiintf.DeleteOper, "/test/obj2", makeRefs(node.Refs, apiintf.NamedReference), c, kvs)
	if err := req1.Check(ctx); err == nil {
		t.Fatalf("should have failed but got no errors")
	}

	node2 := graph.Node{
		This: "/test/obj1",
		Refs: map[string][]string{
			"spec.field1": {
				"/test/obj1",
				"/test/obj3",
			},
		},
	}
	if err := c.UpdateNode(&node2); err != nil {
		t.Fatalf("Update node failed (%s)", err)
	}

	if err := req1.Check(ctx); err != nil {
		t.Fatalf("should have passed but got errors(%s)", err)
	}

	node = graph.Node{
		This: "/test/obj1",
	}

	if err := c.UpdateNode(&node); err != nil {
		t.Fatalf("Update node failed (%s)", err)
	}

	if err := req.Check(ctx); err != nil {
		t.Fatalf("should have succeeded but got errors (%s)", err)
	}
	if err := req1.Check(ctx); err != nil {
		t.Fatalf("should have succeeded but got errors (%s)", err)
	}
	txn := &mocks.FakeTxn{}
	if err := req.Apply(ctx, txn, kvs); err != nil {
		t.Fatalf("failed to create transaction (%s)", err)
	}
	if len(txn.Cmps) != 1 {
		t.Fatalf("numnber of comparators does not match got [%v]", len(txn.Cmps))
	}
	if txn.Cmps[0].Key != "/test/obj1" || txn.Cmps[0].Operator != "=" || txn.Cmps[0].Version != 9 {
		t.Fatalf("comparator does not match got [%+v]", txn.Cmps[0])
	}
}
