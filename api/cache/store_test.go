package cache

import (
	"reflect"
	"testing"

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
	cbfunc := func(str string, obj runtime.Object) {
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
	rs := s.List("/venice/books/book", opts)
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
	rs = s.List("/venice/books/book", opts)
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
	rs = s.List("/venice/books/book", opts)
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
	_, err = s.Delete("/venice/books/book/Example2", 11, cbfunc)
	if err == nil {
		t.Errorf("expecting error on delete")
	}
	if cbCalled != 0 {
		t.Errorf("callback called in failer case")
	}
	o, err := s.Delete("/venice/books/book/Example2", 10, cbfunc)
	if err != nil {
		t.Errorf("not expecting error on delete")
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
