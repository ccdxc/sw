package cache

import (
	"reflect"
	"testing"
	"time"

	"github.com/tchap/go-patricia/patricia"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/utils/runtime"
)

func TestStoreOper(t *testing.T) {
	s := NewStore()

	t.Logf("  ->Insert an object")
	b1 := testObj{}
	b1.ResourceVersion = "10"
	b1.Name = "Example"
	s.Set("/venice/books/book/Example", 10, &b1, nil)
	r, err := s.Get("/venice/books/book/Example")
	if err != nil {
		t.Errorf("expecting object to be found")
	}
	if !reflect.DeepEqual(&b1, r) {
		t.Errorf("objects do not match")
	}
	t.Logf("  ->Insert the same object with a previous rev")
	b2 := testObj{}
	b2.ResourceVersion = "9"
	b2.Name = "changed"
	cbCalled := 0
	cbfunc := func(str string, obj, prev runtime.Object) {
		cbCalled++
	}
	err = s.Set("/venice/books/book/Example", 9, &b2, cbfunc)
	if err == nil {
		t.Errorf("expecting error for packpedalling")
	}
	if cbCalled != 0 {
		t.Errorf("callback called in failure case")
	}
	r, err = s.Get("/venice/books/book/Example")
	if err != nil {
		t.Errorf("expecting object to be found")
	}
	b := r.(*testObj)
	if b.Name != "Example" {
		t.Errorf("Object was changed while backpedalling %s", b.Name)
	}

	t.Logf("  ->Insert the same object with a forward rev")
	b3 := testObj{}
	b3.ResourceVersion = "11"
	b3.Name = "changed again"
	err = s.Set("/venice/books/book/Example", 11, &b3, cbfunc)
	if err != nil {
		t.Errorf("got error when updating %s", err)
	}
	if cbCalled != 1 {
		t.Errorf("callback not called in success case")
	}
	r, err = s.Get("/venice/books/book/Example")
	if err != nil {
		t.Errorf("expecting object to be found")
	}
	b = r.(*testObj)
	if b.Name != "changed again" {
		t.Errorf("Object was not updated [%s]", b.Name)
	}

	t.Logf("  ->Insert Multiple objects")
	b4 := testObj{}
	b4.ResourceVersion = "9"
	b4.Name = "B4"
	b5 := testObj{}
	b5.ResourceVersion = "10"
	b5.Name = "B5"
	err = s.Set("/venice/books/book/Example1", 9, &b4, nil)
	if err != nil {
		t.Errorf("got error when updating %s", err)
	}
	err = s.Set("/venice/books/book/Example2", 10, &b5, nil)
	if err != nil {
		t.Errorf("got error when updating %s", err)
	}
	r, err = s.Get("/venice/books/book/Example1")
	if err != nil {
		t.Errorf("expecting object to be found")
	}
	if !reflect.DeepEqual(&b4, r) {
		t.Errorf("objects do not match")
	}
	r, err = s.Get("/venice/books/book/Example2")
	if err != nil {
		t.Errorf("expecting object to be found")
	}
	if !reflect.DeepEqual(&b5, r) {
		t.Errorf("objects do not match")
	}

	t.Logf("  ->List Multiple objects")
	opts := api.ListWatchOptions{}
	rs, err := s.List("/venice/books/book", opts)
	if err != nil {
		t.Errorf("List failed (%s)", err)
	}
	if len(rs) != 3 {
		t.Errorf("expecting 3 objects")
	}
	checkobjs := []runtime.Object{
		&b3, &b4, &b5,
	}
	for _, v := range rs {
		for i, cmp := range checkobjs {
			if reflect.DeepEqual(v, cmp) {
				checkobjs[i] = nil
			}
		}
	}
	for _, cmp := range checkobjs {
		if cmp != nil {
			t.Errorf("Found an object that was not mached in list %+v", cmp)
		}
	}
	t.Logf("  ->List filtered on version")
	opts.ResourceVersion = "10"
	rs, err = s.List("/venice/books/book", opts)
	if err != nil {
		t.Errorf("List failed (%s)", err)
	}
	if len(rs) != 2 {
		t.Errorf("expecting 2 objects")
	}
	checkobjs = []runtime.Object{
		&b3, &b5,
	}
	for _, v := range rs {
		for i, cmp := range checkobjs {
			if reflect.DeepEqual(v, cmp) {
				checkobjs[i] = nil
			}
		}
	}
	for _, cmp := range checkobjs {
		if cmp != nil {
			t.Errorf("Found an object that was not mached in list %+v", cmp)
		}
	}

	cbCalled = 0
	t.Logf("  ->Mark and sweep objects")
	s.Mark("/venice/books/book/")
	// Add back 2 items
	err = s.Set("/venice/books/book/Example1", 9, &b4, nil)
	if err != nil {
		t.Errorf("got error when updating %s", err)
	}
	err = s.Set("/venice/books/book/Example2", 10, &b5, nil)
	if err != nil {
		t.Errorf("got error when updating %s", err)
	}
	s.Sweep("/venice/books/book/", cbfunc)
	opts.ResourceVersion = ""
	rs, err = s.List("/venice/books/book", opts)
	if err != nil {
		t.Errorf("List failed (%s)", err)
	}
	if len(rs) != 2 {
		t.Errorf("expecting 2 objects")
	}
	checkobjs = []runtime.Object{
		&b4, &b5,
	}
	for _, v := range rs {
		for i, cmp := range checkobjs {
			if reflect.DeepEqual(v, cmp) {
				checkobjs[i] = nil
			}
		}
	}
	for _, cmp := range checkobjs {
		if cmp != nil {
			t.Errorf("Found an object that was not mached in list %+v", cmp)
		}
	}

	cbCalled = 0
	t.Logf("  ->Delete an valid object (unconditional)")
	_, err = s.Delete("/venice/books/book/Example1", 0, cbfunc)
	if err != nil {
		t.Errorf("not expecting error on delete")
	}
	if cbCalled != 1 {
		t.Errorf("callback not called in success case")
	}
	_, err = s.Get("/venice/books/book/Example1")
	if err == nil {
		t.Errorf("expecting object to be not found")
	}
	r, err = s.Get("/venice/books/book/Example2")
	if err != nil {
		t.Errorf("expecting object to be found")
	}
	if !reflect.DeepEqual(&b5, r) {
		t.Errorf("objects do not match")
	}

	t.Logf("  ->Delete an valid object (conditional)")
	cbCalled = 0
	_, err = s.Delete("/venice/books/book/Example2", 9, cbfunc)
	if err == nil {
		t.Errorf("expecting error on delete")
	}
	if cbCalled != 0 {
		t.Errorf("callback called in failer case")
	}
	o, err := s.Delete("/venice/books/book/Example2", 12, cbfunc)
	if err != nil {
		t.Errorf("not expecting error on delete (%s)", err)
	}
	if !reflect.DeepEqual(o, &b5) {
		t.Errorf("returned object in delete does not match")
	}
	if cbCalled != 1 {
		t.Errorf("callback not called in success case")
	}
	r, err = s.Get("/venice/books/book/Example2")
	if err == nil {
		t.Errorf("expecting object to be not found")
	}

	t.Logf("  ->Flush the cache")
	s.Clear()
	r, err = s.Get("/venice/books/book/Example")
	if err == nil {
		t.Errorf("not expecting object to be found after flush")
	}
}

func TestDelayedDelete(t *testing.T) {
	s := NewStore().(*store)

	t.Logf("  ->Insert an object")
	key1 := "/venice/books/book/Example1"
	key2 := "/venice/books/book/Example2"
	prefix1 := patricia.Prefix(key1)
	prefix2 := patricia.Prefix(key2)
	b1 := testObj{}
	b1.ResourceVersion = "10"
	b1.Name = "Example1"
	s.Set(key1, 10, &b1, nil)
	b2 := testObj{}
	b2.ResourceVersion = "10"
	b2.Name = "Example2"
	s.Set(key2, 11, &b2, nil)

	t.Logf("  ->Delete one object and verify it is in delQ")
	s.Delete(key1, 0, nil)
	_, err := s.Get(key1)
	if err == nil {
		t.Fatalf("able to retreive deleted object")
	}
	if s.delPending.Len() != 1 {
		t.Fatalf("expecting 1 element in del pending queue got %d", s.delPending.Len())
	}
	if !reflect.DeepEqual(s.delPending.Peek(), s.objs.Get(prefix1)) {
		t.Fatalf("objects in delqueue and trie do not match")
	}

	t.Logf("  ->Delete second object and verify it is in delQ behind (1)")
	s.Delete(key2, 0, nil)
	_, err = s.Get(key2)
	if err == nil {
		t.Fatalf("able to retreive deleted object")
	}
	if s.delPending.Len() != 2 {
		t.Fatalf("expecting 2 element in del pending queue got %d", s.delPending.Len())
	}
	if !reflect.DeepEqual(s.delPending.Peek(), s.objs.Get(prefix1)) {
		t.Fatalf("objects in delqueue and trie do not match [%v]/[%v]\n objects", s.delPending.Peek(), s.objs.Get(prefix1))
	}

	t.Logf("  ->Re-add first object and verify it is no longer in the delQ")
	b1.ResourceVersion = "12"
	s.Set(key1, 12, &b1, nil)
	if s.delPending.Len() != 1 {
		t.Fatalf("expecting 1 element in del pending queue got %d", s.delPending.Len())
	}
	cobj := s.objs.Get(prefix2).(*cacheObj)
	if !reflect.DeepEqual(s.delPending.Peek(), cobj) {
		t.Fatalf("objects in delqueue and trie do not match[%v]/[%v]", s.delPending.Peek(), s.objs.Get(prefix2))
	}

	if !cobj.deleted || !cobj.inDelQ || cobj.delQId != 0 {
		t.Fatalf("flags not set for deleted object [%v/%v/%d]", cobj.deleted, cobj.inDelQ, cobj.delQId)
	}

	_, err = s.Get(key1)
	if err != nil {
		t.Fatalf("retreive of re-added object returned error (%s)", err)
	}

	t.Logf("  ->delete first object and verify it is still not at head of delq")
	s.Delete(key1, 0, nil)
	if s.delPending.Len() != 2 {
		t.Fatalf("expecting 2 element in del pending queue got %d [%+v]", s.delPending.Len(), s.delPending.l)
	}
	cobj = s.objs.Get(prefix2).(*cacheObj)
	if !reflect.DeepEqual(s.delPending.Peek(), cobj) {
		t.Fatalf("objects in delqueue and trie do not match[%v]/[%v]", s.delPending.Peek(), s.objs.Get(prefix2))
	}

	// Validate Ids
	for i := range s.delPending.l {
		if i != s.delPending.l[i].delQId {
			t.Fatalf("Ids dont match for [%d]:%d", i, s.delPending.l[i].delQId)
		}
	}

	t.Logf("  ->Run purge with duration outside deleted items")
	s.PurgeDeleted(time.Hour)
	if s.delPending.Len() != 2 {
		t.Fatalf("expecting 2 element in del pending queue got %d [%+v]", s.delPending.Len(), s.delPending.l)
	}
	cobj = s.objs.Get(prefix2).(*cacheObj)
	if !reflect.DeepEqual(s.delPending.Peek(), cobj) {
		t.Fatalf("objects in delqueue and trie do not match[%v]/[%v]", s.delPending.Peek(), s.objs.Get(prefix2))
	}

	// Validate Ids
	for i := range s.delPending.l {
		if i != s.delPending.l[i].delQId {
			t.Fatalf("Ids dont match for [%d]:%d", i, s.delPending.l[i].delQId)
		}
	}

	t.Logf("  ->Run purge with duration encompassing items in the delpending items")
	s.PurgeDeleted(0)
	if s.delPending.Len() != 0 {
		t.Fatalf("expecting 2 element in del pending queue got %d [%+v]", s.delPending.Len(), s.delPending.l)
	}

}
