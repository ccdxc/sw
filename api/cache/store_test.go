package cache

import (
	"fmt"
	"reflect"
	"testing"
	"time"

	"github.com/tchap/go-patricia/patricia"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/utils/runtime"
	. "github.com/pensando/sw/venice/utils/testutils"
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
	rs, err := s.List("/venice/books/book", "", opts)
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
	rs, err = s.List("/venice/books/book", "", opts)
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
	rs, err = s.List("/venice/books/book", "", opts)
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

	t.Logf("  -> Stat a set of objects")
	skeys := []string{"/venice/books/book/Example1"}
	stat := s.Stat(skeys)
	if len(stat) != 1 {
		t.Errorf("unexpected numner of objects in Stat")
	}
	if stat[0].Key != "/venice/books/book/Example1" || stat[0].Revision != 9 || !stat[0].Valid || !reflect.DeepEqual(stat[0].ObjectMeta, b4.ObjectMeta) {
		t.Errorf("Stat does not match")
	}
	skeys = []string{"/venice/books/book/Example1", "/venice/books/book/Example2", "/venice/books/book/invalid"}
	stat = s.Stat(skeys)
	if len(stat) != 3 {
		t.Errorf("unexpected numner of objects in Stat")
	}
	if stat[0].Key != "/venice/books/book/Example1" || stat[0].Revision != 9 || !stat[0].Valid || !reflect.DeepEqual(stat[0].ObjectMeta, b4.ObjectMeta) {
		t.Errorf("Stat does not match")
	}
	if stat[1].Key != "/venice/books/book/Example2" || stat[1].Revision != 10 || !stat[1].Valid || !reflect.DeepEqual(stat[1].ObjectMeta, b5.ObjectMeta) {
		t.Errorf("Stat does not match")
	}
	if stat[2].Valid {
		t.Errorf("expecting invalid entry")
	}

	t.Logf("  -> StatAll objects")
	prefix := "/venice/books/book"
	stat = s.StatAll(prefix)
	if len(stat) != 2 {
		t.Errorf("unexpected numner of objects in Stat")
	}
	if stat[0].Key != "/venice/books/book/Example1" || stat[0].Revision != 9 || !stat[0].Valid || !reflect.DeepEqual(stat[0].ObjectMeta, b4.ObjectMeta) {
		t.Errorf("Stat does not match")
	}
	if stat[1].Key != "/venice/books/book/Example2" || stat[1].Revision != 10 || !stat[1].Valid || !reflect.DeepEqual(stat[1].ObjectMeta, b5.ObjectMeta) {
		t.Errorf("Stat does not match")
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
	t.Logf("   -> reinsert deleted object")
	cbfunc2 := func(str string, obj, prev runtime.Object) {
		if prev == nil && obj != nil {
			cbCalled++
		}
	}
	cbCalled = 0
	b6 := testObj{}
	b6.ResourceVersion = "12"
	b6.Name = "B6"
	err = s.Set("/venice/books/book/Example1", 12, &b6, cbfunc2)
	if err != nil {
		t.Errorf("got error when updating %s", err)
	}
	if cbCalled != 1 {
		t.Errorf("create after delete CB not right")
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
	key3 := "/venice/books/book/Example3"
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
	b3 := testObj{}
	b3.ResourceVersion = "10"
	b3.Name = "Example3"
	s.Set(key3, 12, &b3, nil)

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
	// tweak the time on the elements
	for i := range s.delPending.l {
		s.delPending.l[i].lastUpd = s.delPending.l[i].lastUpd.Add(-(time.Second * 2))
	}
	t.Logf("  ->Delete third object and verify it is in delQ behind (1)")
	s.Delete(key3, 0, nil)
	if s.delPending.Len() != 3 {
		t.Fatalf("expecting 3 element in del pending queue got %d [%+v]", s.delPending.Len(), s.delPending.l)
	}

	// Validate Ids
	for i := range s.delPending.l {
		if i != s.delPending.l[i].delQId {
			t.Fatalf("Ids dont match for [%d]:%d", i, s.delPending.l[i].delQId)
		}
	}

	t.Logf("  ->Run purge with duration encompassing items in the delpending items")
	s.PurgeDeleted(time.Second * 1)
	if s.delPending.Len() != 1 {
		t.Fatalf("expecting 1 element in del pending queue got %d [%+v]", s.delPending.Len(), s.delPending.l)
	}

	if s.delPending.l[0].key != key3 {
		t.Fatalf("wrong object [%v]", s.delPending.l[0])
	}
}

func TestMVCCOps(t *testing.T) {
	newRuntimeObj := func(rev uint64) runtime.Object {
		return &testObj{
			ObjectMeta: api.ObjectMeta{
				Name:            "object1",
				ResourceVersion: fmt.Sprintf("%d", rev),
			},
		}
	}

	s := NewStore().(*store)

	t.Logf("  ->Insert an object")
	key1 := "/venice/books/book/Example1"
	key2 := "/venice/books/book/Example2"
	prefix := "/venice/books/book"
	b1 := newRuntimeObj(5)
	s.Set(key1, 5, b1, nil)

	_, err := s.GetFromSnapshot(1, key1)
	Assert(t, err != nil, "expecting error")

	robj, err := s.GetFromSnapshot(10, key1)
	Assert(t, err != nil, "expecting error")

	robj, err = s.Get(key1)
	AssertOk(t, err, "expecting to suceed")
	rrobj := robj.(*testObj)
	Assert(t, rrobj.ResourceVersion == "5", "unexpected object [%s]", rrobj.ResourceVersion)

	t.Logf("  ->Create a snapshot at 10")
	s.lastVer = 10
	s.StartSnapshot()

	{
		_, err = s.GetFromSnapshot(4, key1)
		Assert(t, err != nil, "expecting error")

		_, err = s.GetFromSnapshot(5, key1)
		Assert(t, err != nil, "expecting error")

		_, err = s.GetFromSnapshot(6, key1)
		Assert(t, err != nil, "expecting error")

		robj, err = s.GetFromSnapshot(10, key1)
		AssertOk(t, err, "expecting to suceed")
		rrobj = robj.(*testObj)
		Assert(t, rrobj.ResourceVersion == "5", "unexpected object [%s]", rrobj.ResourceVersion)

		robj, err = s.GetFromSnapshot(11, key1)
		Assert(t, err != nil, "expecting error")

		robj, err = s.Get(key1)
		AssertOk(t, err, "expecting to suceed")
		rrobj = robj.(*testObj)
		Assert(t, rrobj.ResourceVersion == "5", "unexpected object [%s]", rrobj.ResourceVersion)
	}

	t.Logf("  ->update object on top of snapshot at 10")
	nobj := newRuntimeObj(11)
	s.Set(key1, 11, nobj, nil)
	{
		_, err = s.GetFromSnapshot(4, key1)
		Assert(t, err != nil, "expecting error")

		_, err = s.GetFromSnapshot(5, key1)
		Assert(t, err != nil, "expecting error")

		_, err = s.GetFromSnapshot(6, key1)
		Assert(t, err != nil, "expecting error")

		robj, err = s.GetFromSnapshot(10, key1)
		AssertOk(t, err, "expecting to suceed")
		rrobj = robj.(*testObj)
		Assert(t, rrobj.ResourceVersion == "5", "unexpected object [%s]", rrobj.ResourceVersion)

		robj, err = s.GetFromSnapshot(11, key1)
		Assert(t, err != nil, "expecting error")
		robj, err = s.Get(key1)
		AssertOk(t, err, "expecting to suceed")
		rrobj = robj.(*testObj)
		Assert(t, rrobj.ResourceVersion == "11", "unexpected object [%s]", rrobj.ResourceVersion)

	}

	t.Logf("  ->Update(2) on top of snapshot at 10")
	nobj = newRuntimeObj(15)
	s.Set(key1, 15, nobj, nil)
	b2 := newRuntimeObj(15)
	s.Set(key2, 15, b2, nil)
	{
		_, err = s.GetFromSnapshot(6, key1)
		Assert(t, err != nil, "expecting error")

		robj, err = s.GetFromSnapshot(10, key1)
		AssertOk(t, err, "expecting to suceed")
		rrobj = robj.(*testObj)
		Assert(t, rrobj.ResourceVersion == "5", "unexpected object [%s]", rrobj.ResourceVersion)

		robj, err = s.GetFromSnapshot(11, key1)
		Assert(t, err != nil, "expecting error")

		robj, err = s.GetFromSnapshot(15, key1)
		Assert(t, err != nil, "expecting error")

		robj, err = s.Get(key1)
		AssertOk(t, err, "expecting to suceed")
		rrobj = robj.(*testObj)
		Assert(t, rrobj.ResourceVersion == "15", "unexpected object [%s]", rrobj.ResourceVersion)
	}

	t.Logf("  ->Create a snapshot at 15")
	s.lastVer = 15
	s.StartSnapshot()
	{
		_, err = s.GetFromSnapshot(6, key1)
		Assert(t, err != nil, "expecting error")

		robj, err = s.GetFromSnapshot(10, key1)
		AssertOk(t, err, "expecting to suceed")
		rrobj = robj.(*testObj)
		Assert(t, rrobj.ResourceVersion == "5", "unexpected object [%s]", rrobj.ResourceVersion)

		robj, err = s.GetFromSnapshot(11, key1)
		Assert(t, err != nil, "expecting error")

		robj, err = s.GetFromSnapshot(15, key1)
		AssertOk(t, err, "expecting to suceed")
		rrobj = robj.(*testObj)
		Assert(t, rrobj.ResourceVersion == "15", "unexpected object [%s]", rrobj.ResourceVersion)

		robj, err = s.Get(key1)
		AssertOk(t, err, "expecting to suceed")
		rrobj = robj.(*testObj)
		Assert(t, rrobj.ResourceVersion == "15", "unexpected object [%s]", rrobj.ResourceVersion)
	}

	t.Logf("  ->List from a snapshot")
	s.Set(key2, 15, nobj, nil)
	{
		l, err := s.List(prefix, "book", api.ListWatchOptions{})
		AssertOk(t, err, "expecting list to succeed")
		Assert(t, len(l) == 2, "expecting 2 entries got %d", len(l))

		l, err = s.ListFromSnapshot(10, prefix, "book", api.ListWatchOptions{})
		AssertOk(t, err, "expecting list to succeed")
		Assert(t, len(l) == 1, "expecting 1 entries got %d", len(l))

		l, err = s.ListFromSnapshot(15, prefix, "book", api.ListWatchOptions{})
		AssertOk(t, err, "expecting list to succeed")
		Assert(t, len(l) == 2, "expecting 2 entries got %d", len(l))

		cbCalled := 0
		keys := make(map[string]bool)
		handleFunc := func(key string, cur, revObj runtime.Object, deleted bool) error {
			if revObj != nil {
				keys[key] = true
			}
			cbCalled++
			return nil
		}

		err = s.ListSnapshotWithCB(prefix, 10, handleFunc)
		AssertOk(t, err, "expecting list with cb to succeed")

		Assert(t, cbCalled == 2, "expecing 1 call of cb got %d", cbCalled)
		_, ok := keys[key1]
		Assert(t, ok, "expecting to find Key1")
		_, ok = keys[key2]
		Assert(t, !ok, "not expecting to find Key2")

		cbCalled = 0
		keys = make(map[string]bool)
		err = s.ListSnapshotWithCB(prefix, 15, handleFunc)
		AssertOk(t, err, "expecting list with cb to succeed")

		Assert(t, cbCalled == 2, "expecing 2 call of cb got %d", cbCalled)
		_, ok = keys[key1]
		Assert(t, ok, "expecting to find Key1")
		_, ok = keys[key2]
		Assert(t, ok, "expecting to find Key2")
	}

	t.Logf("  ->Delete on top of snapshot at 15")
	_, err = s.Delete(key1, 16, nil)
	AssertOk(t, err, "delete failed (%s)", err)
	{
		_, err = s.GetFromSnapshot(6, key1)
		Assert(t, err != nil, "expecting error")

		robj, err = s.GetFromSnapshot(10, key1)
		AssertOk(t, err, "expecting to suceed")
		rrobj = robj.(*testObj)
		Assert(t, rrobj.ResourceVersion == "5", "unexpected object [%s]", rrobj.ResourceVersion)

		robj, err = s.GetFromSnapshot(11, key1)
		Assert(t, err != nil, "expecting error")

		robj, err = s.GetFromSnapshot(15, key1)
		AssertOk(t, err, "expecting to suceed")
		rrobj = robj.(*testObj)
		Assert(t, rrobj.ResourceVersion == "15", "unexpected object [%s]", rrobj.ResourceVersion)

		robj, err = s.Get(key1)
		Assert(t, err != nil, "expecting error")
	}
	t.Logf("  ->Create a snapshot at 17")
	s.lastVer = 15
	s.StartSnapshot()
	{
		_, err = s.GetFromSnapshot(6, key1)
		Assert(t, err != nil, "expecting error")

		robj, err = s.GetFromSnapshot(10, key1)
		AssertOk(t, err, "expecting to suceed")
		rrobj = robj.(*testObj)
		Assert(t, rrobj.ResourceVersion == "5", "unexpected object [%s]", rrobj.ResourceVersion)

		robj, err = s.GetFromSnapshot(11, key1)
		Assert(t, err != nil, "expecting error")

		robj, err = s.GetFromSnapshot(15, key1)
		AssertOk(t, err, "expecting to suceed")
		rrobj = robj.(*testObj)
		Assert(t, rrobj.ResourceVersion == "15", "unexpected object [%s]", rrobj.ResourceVersion)

		robj, err = s.GetFromSnapshot(17, key1)
		Assert(t, err != nil, "expecting error")

		robj, err = s.Get(key1)
		Assert(t, err != nil, "expecting error")
	}

	t.Logf("  ->recreatre deleted object")
	nobj = newRuntimeObj(18)
	s.Set(key1, 18, nobj, nil)
	{
		_, err = s.GetFromSnapshot(6, key1)
		Assert(t, err != nil, "expecting error")

		robj, err = s.GetFromSnapshot(10, key1)
		AssertOk(t, err, "expecting to suceed")
		rrobj = robj.(*testObj)
		Assert(t, rrobj.ResourceVersion == "5", "unexpected object [%s]", rrobj.ResourceVersion)

		robj, err = s.GetFromSnapshot(11, key1)
		Assert(t, err != nil, "expecting error")

		robj, err = s.GetFromSnapshot(15, key1)
		AssertOk(t, err, "expecting to suceed")
		rrobj = robj.(*testObj)
		Assert(t, rrobj.ResourceVersion == "15", "unexpected object [%s]", rrobj.ResourceVersion)

		robj, err = s.Get(key1)
		AssertOk(t, err, "expecting to suceed")
		rrobj = robj.(*testObj)
		Assert(t, rrobj.ResourceVersion == "18", "unexpected object [%s]", rrobj.ResourceVersion)
	}

	t.Logf("  ->Create an empty snapshot at 20")
	s.lastVer = 20
	s.StartSnapshot()

	t.Logf("  ->Create an empty snapshot at 22")
	s.lastVer = 22
	s.StartSnapshot()
	{
		_, err = s.GetFromSnapshot(6, key1)
		Assert(t, err != nil, "expecting error")

		robj, err = s.GetFromSnapshot(10, key1)
		AssertOk(t, err, "expecting to suceed")
		rrobj = robj.(*testObj)
		Assert(t, rrobj.ResourceVersion == "5", "unexpected object [%s]", rrobj.ResourceVersion)

		robj, err = s.GetFromSnapshot(11, key1)
		Assert(t, err != nil, "expecting error")

		robj, err = s.GetFromSnapshot(15, key1)
		AssertOk(t, err, "expecting to suceed")
		rrobj = robj.(*testObj)
		Assert(t, rrobj.ResourceVersion == "15", "unexpected object [%s]", rrobj.ResourceVersion)

		robj, err = s.GetFromSnapshot(20, key1)
		AssertOk(t, err, "expecting to suceed")
		rrobj = robj.(*testObj)
		Assert(t, rrobj.ResourceVersion == "18", "unexpected object [%s]", rrobj.ResourceVersion)

		robj, err = s.GetFromSnapshot(22, key1)
		AssertOk(t, err, "expecting to suceed")
		rrobj = robj.(*testObj)
		Assert(t, rrobj.ResourceVersion == "18", "unexpected object [%s]", rrobj.ResourceVersion)

		robj, err = s.Get(key1)
		AssertOk(t, err, "expecting to suceed")
		rrobj = robj.(*testObj)
		Assert(t, rrobj.ResourceVersion == "18", "unexpected object [%s]", rrobj.ResourceVersion)
	}

	t.Logf("  ->update object on top of snapshot 22")
	nobj = newRuntimeObj(25)
	s.Set(key1, 25, nobj, nil)
	{
		_, err = s.GetFromSnapshot(6, key1)
		Assert(t, err != nil, "expecting error")

		robj, err = s.GetFromSnapshot(10, key1)
		AssertOk(t, err, "expecting to suceed")
		rrobj = robj.(*testObj)
		Assert(t, rrobj.ResourceVersion == "5", "unexpected object [%s]", rrobj.ResourceVersion)

		robj, err = s.GetFromSnapshot(11, key1)
		Assert(t, err != nil, "expecting error")

		robj, err = s.GetFromSnapshot(15, key1)
		AssertOk(t, err, "expecting to suceed")
		rrobj = robj.(*testObj)
		Assert(t, rrobj.ResourceVersion == "15", "unexpected object [%s]", rrobj.ResourceVersion)

		robj, err = s.GetFromSnapshot(20, key1)
		AssertOk(t, err, "expecting to suceed")
		rrobj = robj.(*testObj)
		Assert(t, rrobj.ResourceVersion == "18", "unexpected object [%s]", rrobj.ResourceVersion)

		robj, err = s.GetFromSnapshot(22, key1)
		AssertOk(t, err, "expecting to suceed")
		rrobj = robj.(*testObj)
		Assert(t, rrobj.ResourceVersion == "18", "unexpected object [%s]", rrobj.ResourceVersion)

		robj, err = s.Get(key1)
		AssertOk(t, err, "expecting to suceed")
		rrobj = robj.(*testObj)
		Assert(t, rrobj.ResourceVersion == "25", "unexpected object [%s]", rrobj.ResourceVersion)
	}

	t.Logf("  ->delete snapshot 10")
	s.DeleteSnapshot(10)
	{
		_, err = s.GetFromSnapshot(6, key1)
		Assert(t, err != nil, "expecting error")

		robj, err = s.GetFromSnapshot(10, key1)
		Assert(t, err != nil, "expecting error")

		robj, err = s.GetFromSnapshot(11, key1)
		Assert(t, err != nil, "expecting error")

		robj, err = s.GetFromSnapshot(15, key1)
		AssertOk(t, err, "expecting to suceed")
		rrobj = robj.(*testObj)
		Assert(t, rrobj.ResourceVersion == "15", "unexpected object [%s]", rrobj.ResourceVersion)

		robj, err = s.GetFromSnapshot(20, key1)
		AssertOk(t, err, "expecting to suceed")
		rrobj = robj.(*testObj)
		Assert(t, rrobj.ResourceVersion == "18", "unexpected object [%s]", rrobj.ResourceVersion)

		robj, err = s.GetFromSnapshot(22, key1)
		AssertOk(t, err, "expecting to suceed")
		rrobj = robj.(*testObj)
		Assert(t, rrobj.ResourceVersion == "18", "unexpected object [%s]", rrobj.ResourceVersion)

		robj, err = s.Get(key1)
		AssertOk(t, err, "expecting to suceed")
		rrobj = robj.(*testObj)
		Assert(t, rrobj.ResourceVersion == "25", "unexpected object [%s]", rrobj.ResourceVersion)
	}

	t.Logf("  ->delete snapshot 20")
	s.DeleteSnapshot(20)
	{
		_, err = s.GetFromSnapshot(6, key1)
		Assert(t, err != nil, "expecting error")

		robj, err = s.GetFromSnapshot(10, key1)
		Assert(t, err != nil, "expecting error")

		robj, err = s.GetFromSnapshot(11, key1)
		Assert(t, err != nil, "expecting error")

		robj, err = s.GetFromSnapshot(15, key1)
		AssertOk(t, err, "expecting to suceed")
		rrobj = robj.(*testObj)
		Assert(t, rrobj.ResourceVersion == "15", "unexpected object [%s]", rrobj.ResourceVersion)

		robj, err = s.GetFromSnapshot(20, key1)
		Assert(t, err != nil, "expecting error")

		robj, err = s.GetFromSnapshot(22, key1)
		AssertOk(t, err, "expecting to suceed")
		rrobj = robj.(*testObj)
		Assert(t, rrobj.ResourceVersion == "18", "unexpected object [%s]", rrobj.ResourceVersion)

		robj, err = s.Get(key1)
		AssertOk(t, err, "expecting to suceed")
		rrobj = robj.(*testObj)
		Assert(t, rrobj.ResourceVersion == "25", "unexpected object [%s]", rrobj.ResourceVersion)
	}
	s.PurgeDeleted(1 * time.Nanosecond)
	{
		_, err = s.GetFromSnapshot(6, key1)
		Assert(t, err != nil, "expecting error")

		robj, err = s.GetFromSnapshot(10, key1)
		Assert(t, err != nil, "expecting error")

		robj, err = s.GetFromSnapshot(11, key1)
		Assert(t, err != nil, "expecting error")

		robj, err = s.GetFromSnapshot(15, key1)
		AssertOk(t, err, "expecting to suceed")
		rrobj = robj.(*testObj)
		Assert(t, rrobj.ResourceVersion == "15", "unexpected object [%s]", rrobj.ResourceVersion)

		robj, err = s.GetFromSnapshot(20, key1)
		Assert(t, err != nil, "expecting error")

		robj, err = s.GetFromSnapshot(22, key1)
		AssertOk(t, err, "expecting to suceed")
		rrobj = robj.(*testObj)
		Assert(t, rrobj.ResourceVersion == "18", "unexpected object [%s]", rrobj.ResourceVersion)

		robj, err = s.Get(key1)
		AssertOk(t, err, "expecting to suceed")
		rrobj = robj.(*testObj)
		Assert(t, rrobj.ResourceVersion == "25", "unexpected object [%s]", rrobj.ResourceVersion)
	}
}
