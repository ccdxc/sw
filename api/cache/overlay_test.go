package cache

import (
	"context"
	"errors"
	"fmt"
	"reflect"
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/runtime"
	. "github.com/pensando/sw/venice/utils/testutils"
)

type fakeCache struct {
	fakeKvStore
	listFilteredFn func(ctx context.Context, prefix string, into runtime.Object, opts api.ListWatchOptions) error
}

func (f *fakeCache) ListFiltered(ctx context.Context, prefix string, into runtime.Object, opts api.ListWatchOptions) error {
	if f.listFilteredFn != nil {
		return f.listFilteredFn(ctx, prefix, into, opts)
	}
	return nil
}

func (f *fakeCache) WatchFiltered(ctx context.Context, key string, opts api.ListWatchOptions) (kvstore.Watcher, error) {
	return nil, nil
}

func (f *fakeCache) Start() error {
	return nil
}

func (f *fakeCache) Clear() {
}

func TestOverlayCRD(t *testing.T) {
	// Get empty
	_, err := GetOverlay("dummy")
	Assert(t, err != nil, "found overlay in empty overlaySingleton")

	// Create overlay
	c := &fakeCache{fakeKvStore: fakeKvStore{}}
	r1, err := NewOverlay("new1", c)
	AssertOk(t, err, "could not create new overlay")

	r2, err := NewOverlay("new2", c)
	AssertOk(t, err, "could not create new overlay")
	Assert(t, len(overlaysSingleton.ovMap) == 2, "number of overlays do not match, expecting 2 got %d", len(overlaysSingleton.ovMap))

	// Get
	x1, err := GetOverlay("new1")
	AssertOk(t, err, "could not retrieve existing overlay")

	x2, err := GetOverlay("new2")
	AssertOk(t, err, "could not retrieve existing overlay")

	Assert(t, x1 == r1 && x2 == r2, "did not get correct overlay")

	_, err = GetOverlay("dummy")
	Assert(t, err != nil, "found non-existent overlay")

	// Add Duplicate
	_, err = NewOverlay("new1", c)
	if err == nil {
		t.Fatalf("could add duplicate overlay")
	}
	// Delete
	err = DelOverlay("dummy")
	Assert(t, err != nil, "could delete non-existent overlay")

	err = DelOverlay("new1")
	AssertOk(t, err, "could not delete overlay")

	err = DelOverlay("new2")
	AssertOk(t, err, "could not delete overlay")

	err = DelOverlay("new1")
	Assert(t, err != nil, "could delete already deleted overlay")

	Assert(t, len(overlaysSingleton.ovMap) == 0,
		"number of overlays do not match, expecting 0 got %d", len(overlaysSingleton.ovMap))
}

func TestOverlayCreate(t *testing.T) {
	c := &fakeCache{fakeKvStore: fakeKvStore{}}
	o, _ := NewOverlay("testCreate", c)
	defer DelOverlay("testCreate")
	ov := o.(*overlay)
	ctx := context.TODO()
	obj := &testObj{}
	obj.Name = "testObj"
	obj.Spec = "Spec 1"
	key := obj.MakeKey("overlay")
	// Create empty
	retObj := &testObj{}
	c.fakeKvStore.getfn = func(ctx context.Context, key string, into runtime.Object) error {
		return errors.New("NotFound")
	}

	err := ov.Get(ctx, key, retObj)
	Assert(t, err != nil, "Getting failure on get of empty overlay and cache got [%+v]", retObj)

	err = ov.Create(ctx, key, obj)
	AssertOk(t, err, "failed to create in overlay")
	Assert(t, c.fakeKvStore.creates == 0, "not expecting write to KV store got %d writes", c.fakeKvStore.creates)

	err = ov.Get(ctx, key, retObj)
	AssertOk(t, err, "could not find object in overlay")
	Assert(t, reflect.DeepEqual(obj, retObj), "Returned object does not match expected [%v]/[]%v]", retObj, obj)

	// Create with existing in overlay
	obj.Spec = "Spec 2"
	err = ov.Create(ctx, key, obj)
	AssertOk(t, err, "failed to create in overlay")
	Assert(t, c.fakeKvStore.creates == 0, "not expecting write to KV store got %d writes", c.fakeKvStore.creates)

	err = ov.Get(ctx, key, retObj)
	AssertOk(t, err, "could not find object in overlay")
	Assert(t, reflect.DeepEqual(obj, retObj), "Returned object does not match expected [%v]/[]%v]", retObj, obj)

	// Create with existing KV
	c.fakeKvStore.getfn = func(ctx context.Context, key string, into runtime.Object) error {
		into = retObj
		return nil
	}
	err = ov.Create(ctx, key, obj)
	Assert(t, err != nil, "Expecting failure for create of existing key")

	// The KV store should not have been touched anytime for anykind of write
	Assert(t, c.fakeKvStore.creates == 0 && c.fakeKvStore.updates == 0 && c.fakeKvStore.deletes == 0, "backend has been written to!")
}

func TestOverlayUpdate(t *testing.T) {
	c := &fakeCache{fakeKvStore: fakeKvStore{}}
	o, _ := NewOverlay("testUpdate", c)
	defer DelOverlay("testUpdate")
	ov := o.(*overlay)
	ctx := context.TODO()
	obj := &testObj{}
	obj.Name = "testObj"
	obj.Spec = "Spec 1"
	key := obj.MakeKey("overlay")
	retObj := &testObj{}
	getObj := false
	c.fakeKvStore.getfn = func(ctx context.Context, key string, into runtime.Object) error {
		if getObj {
			retObj.Clone(into)
			return nil
		}
		return errors.New("NotFound")
	}

	// Update on empty
	err := ov.Update(ctx, key, obj)
	Assert(t, err != nil, "expecting update on empty to fail")

	// update with cache entry
	getObj = true
	err = ov.Update(ctx, key, obj)
	AssertOk(t, err, "expecting update to succeed, got (%s)", err)

	err = ov.Get(ctx, key, retObj)
	AssertOk(t, err, "could not find object in overlay")

	if !reflect.DeepEqual(obj, retObj) {
		t.Errorf("Returned object does not match expected [%v]/[]%v]", retObj, obj)
	}
	cobj := ov.overlay[key]
	Assert(t, cobj.oper == operUpdate, "expecting operation to be update")

	// UPdate with existing update entry
	obj.Spec = "Spec 2"
	err = ov.Update(ctx, key, obj)
	AssertOk(t, err, "expecting update to succeed, got (%s)", err)

	err = ov.Get(ctx, key, retObj)
	AssertOk(t, err, "could not find object in overlay")
	Assert(t, reflect.DeepEqual(obj, retObj), "Returned object does not match expected [%v]/[]%v]", retObj, obj)

	cobj = ov.overlay[key]
	Assert(t, cobj.oper == operUpdate, "expecting operation to be update")

	// Update with existing Create entry
	cobj = ov.overlay[key]
	getObj = false
	cobj.oper = operCreate
	obj.Spec = "Spec 3"
	err = ov.Update(ctx, key, obj)
	AssertOk(t, err, "expecting update to succeed, got (%s)", err)

	err = ov.Get(ctx, key, retObj)
	AssertOk(t, err, "could not find object in overlay")

	Assert(t, reflect.DeepEqual(obj, retObj), fmt.Sprintf("Returned object does not match expected [%v]/[]%v]", retObj, obj))

	cobj = ov.overlay[key]
	Assert(t, cobj.oper == operCreate, fmt.Sprintf("expecting operation to be create got %v", cobj.oper))

	// with existing delete entry, and in API cache.
	getObj = true
	err = ov.Update(ctx, key, obj)
	AssertOk(t, err, "expecting update to succeed")

	err = ov.Get(ctx, key, retObj)
	AssertOk(t, err, "could not find object in overlay")

	if !reflect.DeepEqual(obj, retObj) {
		t.Errorf("Returned object does not match expected [%v]/[]%v]", retObj, obj)
	}
	cobj = ov.overlay[key]
	Assert(t, cobj.oper == operUpdate, "expecting operation to be Update")

	// Consistent update test
	updatefunc := func(oldObj runtime.Object) (newObj runtime.Object, err error) {
		return oldObj, nil
	}
	updatefunc1 := func(oldObj runtime.Object) (newObj runtime.Object, err error) {
		return oldObj, nil
	}
	err = ov.ConsistentUpdate(ctx, key, obj, updatefunc)
	cobj = ov.overlay[key]
	Assert(t, cobj.oper == operUpdate, "expecting operation to be Update")
	Assert(t, reflect.ValueOf(cobj.updateFn).Pointer() == reflect.ValueOf(updatefunc).Pointer(), "did not find updatefunc populated")

	// Overwrite update func
	err = ov.ConsistentUpdate(ctx, key, obj, updatefunc1)
	cobj = ov.overlay[key]
	Assert(t, cobj.oper == operUpdate, "expecting operation to be Update")
	Assert(t, reflect.ValueOf(cobj.updateFn).Pointer() == reflect.ValueOf(updatefunc1).Pointer(), "did not find updatefunc overwritten")

	// The KV store should not have been touched anytime for anykind of write
	Assert(t, c.fakeKvStore.creates == 0 && c.fakeKvStore.updates == 0 && c.fakeKvStore.deletes == 0, "backend has been written to!")
}
func TestOverlayDelete(t *testing.T) {
	c := &fakeCache{fakeKvStore: fakeKvStore{}}
	o, _ := NewOverlay("testDelete", c)
	defer DelOverlay("testDelete")
	ov := o.(*overlay)
	ctx := context.TODO()
	obj := &testObj{}
	obj.Name = "testObj"
	obj.Spec = "Spec 1"
	key := obj.MakeKey("overlay")
	retObj := &testObj{}
	getObj := false
	c.fakeKvStore.getfn = func(ctx context.Context, key string, into runtime.Object) error {
		if getObj {
			retObj.Clone(into)
			return nil
		}
		return errors.New("NotFound")
	}

	// Delete non-existing key
	err := ov.Delete(ctx, key, nil)
	Assert(t, err != nil, "expecting delete to fail")

	// Delete in API cache
	getObj = true
	delObj := &testObj{}
	err = ov.Delete(ctx, key, delObj)
	AssertOk(t, err, fmt.Sprintf("expecting delete to succeed, got (%s)", err))

	cobj := ov.overlay[key]
	Assert(t, cobj.oper == operDelete, "expecting operation to be delete")

	// Delete on an already staged object
	err = ov.Delete(ctx, key, delObj)
	AssertOk(t, err, "expecting delete to succeed, got (%s)", err)

	cobj = ov.overlay[key]
	Assert(t, cobj.oper == operDelete, "expecting operation to be delete")

	// Delete with Create/UPdate in overlay and object in cache
	cobj.oper = operUpdate
	err = ov.Delete(ctx, key, delObj)
	AssertOk(t, err, "expecting delete to succeed, got (%s)", err)
	Assert(t, cobj.oper == operDelete, "expecting operation to be delete")

	// Delete with create in overlay and no object in cache
	cobj.oper = operCreate
	getObj = false
	err = ov.Delete(ctx, key, delObj)
	AssertOk(t, err, "expecting delete to succeed, got (%s)", err)

	cobj, ok := ov.overlay[key]
	Assert(t, !ok, "expecting key to be deleted from overlay")

	// The KV store should not have been touched anytime for anykind of write
	Assert(t, c.fakeKvStore.creates == 0 && c.fakeKvStore.updates == 0 && c.fakeKvStore.deletes == 0, "backend has been written to!")
}

func TestOverlayGet(t *testing.T) {
	c := &fakeCache{fakeKvStore: fakeKvStore{}}
	o, _ := NewOverlay("testGet", c)
	defer DelOverlay("testGet")
	ov := o.(*overlay)
	ctx := context.TODO()
	retobj := &testObj{}
	retobj.Name = "testObj"
	retobj.Spec = "Spec 1"
	key := retobj.MakeKey("overlay")
	obj := &testObj{}
	getObj := false
	c.fakeKvStore.getfn = func(ctx context.Context, key string, into runtime.Object) error {
		if getObj {
			retobj.Clone(into)
			return nil
		}
		return errors.New("NotFound")
	}
	// get with no cache or overlay
	err := ov.Get(ctx, key, obj)
	if err == nil {
		t.Errorf("expecting get to fail")
	}
	// Get with Cache no overlay
	getObj = true
	obj = &testObj{}
	err = ov.Get(ctx, key, obj)
	AssertOk(t, err, "expecting get to succeed, got (%s)", err)
	Assert(t, reflect.DeepEqual(retobj, obj), "retrieved object does not match [%+v]\n[%+v]", retobj, obj)

	// Get with Cache and Overlay (Update)
	obj.Name = "New Item"
	err = ov.Update(ctx, key, obj)
	AssertOk(t, err, "expecting update to succeed, got (%s)", err)

	obj1 := &testObj{}
	err = ov.Get(ctx, key, obj1)
	AssertOk(t, err, "expecting get to succeed, got (%s)", err)
	Assert(t, reflect.DeepEqual(obj, obj1), "retrieved object does not match [%+v]\n[%+v]", retobj, obj)

	// get with cache and overlay (delete)
	cobj := ov.overlay[key]
	cobj.oper = operDelete
	err = ov.Get(ctx, key, obj1)
	Assert(t, err != nil, "expecting get to fail")

	// get with no cache and overlay(create)
	getObj = false
	cobj = ov.overlay[key]
	cobj.oper = operCreate
	err = ov.Get(ctx, key, obj1)
	AssertOk(t, err, "expecting get to succeed, got (%s)", err)
	Assert(t, reflect.DeepEqual(obj, obj1), "retrieved object does not match [%+v]\n[%+v]", retobj, obj)

	// The KV store should not have been touched anytime for anykind of write
	Assert(t, c.fakeKvStore.creates == 0 && c.fakeKvStore.updates == 0 && c.fakeKvStore.deletes == 0, "backend has been written to!")
}

func TestOverlayList(t *testing.T) {
	c := &fakeCache{fakeKvStore: fakeKvStore{}}
	o, _ := NewOverlay("testList", c)
	defer DelOverlay("testList")
	ov := o.(*overlay)
	ctx := context.TODO()
	obj := &testObj{}
	key := obj.MakeKey("overlay")

	retList := &testObjList{}

	getObj := false
	c.listFilteredFn = func(ctx context.Context, prefix string, into runtime.Object, opts api.ListWatchOptions) error {
		if getObj {
			out := into.(*testObjList)
			out.Items = retList.Items
			return nil
		}
		return errors.New("NotFound")
	}

	validateList := func(got, exp *testObjList) (string, bool) {
		if len(got.Items) != len(exp.Items) {
			g := ""
			for _, x := range got.Items {
				g = g + fmt.Sprintf(" [%+v]", x.GetObjectMeta())
			}
			e := ""
			for _, x := range exp.Items {
				e = e + fmt.Sprintf(" [%v]", x.GetObjectMeta())
			}
			t.Logf("got [%+v] \n expected [%+v]", g, e)
			return fmt.Sprintf("expecting %d objects, got %d", len(exp.Items), len(got.Items)), false
		}
		for _, v := range got.Items {
			for i, cmp := range exp.Items {
				if reflect.DeepEqual(v, cmp) {
					exp.Items[i] = nil
				}
			}
		}
		for _, cmp := range exp.Items {
			if cmp != nil {
				return fmt.Sprintf("Found an object that was not matched in list %+v", cmp), false
			}
		}
		return "pass", true
	}

	tobjs := []*testObj{
		&testObj{
			ObjectMeta: api.ObjectMeta{
				Tenant: "tenant1",
				Name:   "test1",
			},
			Spec: "Cache1",
		},
		&testObj{
			ObjectMeta: api.ObjectMeta{
				Tenant: "tenant2",
				Name:   "test2",
			},
			Spec: "Cache2",
		},
		&testObj{
			ObjectMeta: api.ObjectMeta{
				Tenant: "tenant1",
				Name:   "test3",
			},
			Spec: "Cache3",
		},
	}

	modobjs := []*testObj{
		&testObj{
			ObjectMeta: api.ObjectMeta{
				Tenant: "tenant2",
				Name:   "testObj",
			},
			Spec: "overlay-Create",
		},
		&testObj{
			ObjectMeta: api.ObjectMeta{
				Tenant: "tenant1",
				Name:   "test1",
			},
			Spec: "overlay-update",
		},
	}

	// List empty
	getList := &testObjList{}
	expList := &testObjList{}
	err := ov.List(ctx, key, getList)
	AssertOk(t, err, "List failed (%s)", err)

	if e, ok := validateList(getList, expList); !ok {
		t.Errorf(" List did not match [%s]", e)
	}
	// List with empty overlay and Cache Objects
	retList.Items = []*testObj{tobjs[0], tobjs[1], tobjs[2]}
	expList.Items = []*testObj{tobjs[0], tobjs[1], tobjs[2]}

	getObj = true
	err = ov.List(ctx, key, getList)
	AssertOk(t, err, "List failed")

	if e, ok := validateList(getList, expList); !ok {
		t.Errorf(" List did not match [%s]", e)
	}
	// List with mixture of overlay and Cache objects
	nkey := modobjs[0].MakeKey("overlay")
	retObj := &testObj{}
	c.fakeKvStore.getfn = func(ctx context.Context, key string, into runtime.Object) error {
		return errors.New("NotFound")
	}

	err = ov.Get(ctx, nkey, retObj)
	Assert(t, err != nil, "expecting failure on get of empty overlay and cache got [%+v]", retObj)

	err = ov.Create(ctx, nkey, modobjs[0])
	AssertOk(t, err, "failed to create in overlay")

	// Create one like the one in cache
	nkey = modobjs[1].MakeKey("overlay")
	c.fakeKvStore.getfn = func(ctx context.Context, key string, into runtime.Object) error {
		modobjs[1].Clone(into)
		return nil
	}
	err = ov.Update(ctx, nkey, modobjs[1])
	AssertOk(t, err, "failed to update in overlay (%s)", err)

	retList.Items = []*testObj{tobjs[0], tobjs[1], tobjs[2]}
	expList.Items = []*testObj{modobjs[1], tobjs[1], tobjs[2], modobjs[0]}
	getObj = true
	err = ov.List(ctx, key, getList)
	AssertOk(t, err, "List failed")

	if e, ok := validateList(getList, expList); !ok {
		t.Errorf(" List did not match [%s]", e)
	}
	// List with mixture of overlay and Cache objects filtered - tenant specified
	opts := api.ListWatchOptions{}
	opts.Tenant = "tenant2"
	retList.Items = []*testObj{tobjs[1]}
	expList.Items = []*testObj{modobjs[0], tobjs[1]}

	err = ov.ListFiltered(ctx, key, getList, opts)
	AssertOk(t, err, "List failed")
	if e, ok := validateList(getList, expList); !ok {
		t.Errorf(" List did not match [%s]", e)
	}

	opts.Tenant = "tenant1"
	retList.Items = []*testObj{tobjs[0], tobjs[2]}
	expList.Items = []*testObj{modobjs[1], tobjs[2]}
	err = ov.ListFiltered(ctx, key, getList, opts)
	AssertOk(t, err, "List failed")
	if e, ok := validateList(getList, expList); !ok {
		t.Errorf(" List did not match [%s]", e)
	}

	opts.Tenant = "tenant3"
	retList.Items = []*testObj{}
	expList.Items = []*testObj{}
	err = ov.ListFiltered(ctx, key, getList, opts)
	AssertOk(t, err, "List failed")
	if e, ok := validateList(getList, expList); !ok {
		t.Errorf(" List did not match [%s]", e)
	}

	// with only overlay objects
	retList.Items = []*testObj{}
	expList.Items = []*testObj{modobjs[0], modobjs[1]}
	getObj = true
	err = ov.List(ctx, key, getList)
	AssertOk(t, err, "List failed")
	if e, ok := validateList(getList, expList); !ok {
		t.Errorf(" List did not match [%s]", e)
	}
}

func TestMultiple(t *testing.T) {
	c := &fakeCache{fakeKvStore: fakeKvStore{}}
	o, _ := NewOverlay("testUpdate", c)
	defer DelOverlay("testUpdate")
	ov := o.(*overlay)
	ctx := context.TODO()
	obj := &testObj{}
	obj.Name = "testObj"
	obj.Spec = "Spec 1"
	key := obj.MakeKey("overlay")
	retObj := &testObj{}
	getObj := false
	c.fakeKvStore.getfn = func(ctx context.Context, key string, into runtime.Object) error {
		if getObj {
			retObj.Clone(into)
			return nil
		}
		return errors.New("NotFound")
	}

	// Test with empty API cache.
	err := ov.Delete(ctx, key, nil)
	Assert(t, err != nil, "expecting Delete on empty to fail")

	// Create on empty
	err = ov.Create(ctx, key, obj)
	AssertOk(t, err, "expecting Create on empty to succeed got (%s)", err)
	cobj := ov.overlay[key]
	Assert(t, cobj.oper == operCreate, "expecting operation to be create got [%s]", cobj.oper)
	err = ov.Get(ctx, key, retObj)
	AssertOk(t, err, "could not find object in overlay")
	Assert(t, reflect.DeepEqual(obj, retObj), "Returned object does not match expected [%v]/[]%v]", retObj, obj)

	// Update on top of create
	obj.Spec = "Spec 2"
	err = ov.Update(ctx, key, obj)
	AssertOk(t, err, "expecting update to succeed got (%s)", err)
	cobj = ov.overlay[key]
	Assert(t, cobj.oper == operCreate, "expecting operation to be create got [%s]", cobj.oper)
	err = ov.Get(ctx, key, retObj)
	AssertOk(t, err, "could not find object in overlay")
	Assert(t, reflect.DeepEqual(obj, retObj), "Returned object does not match expected [%v]/[]%v]", retObj, obj)

	// Delete
	err = ov.Delete(ctx, key, retObj)
	AssertOk(t, err, "expecting delete to succeed got (%s)", err)
	cobj = ov.overlay[key]
	Assert(t, cobj == nil, "expecting not to find object in overlay got [%+v]", cobj)
	Assert(t, reflect.DeepEqual(obj, retObj), "Returned object does not match expected [%v]/[]%v]", retObj, obj)
	err = ov.Get(ctx, key, retObj)
	Assert(t, err != nil, "not expecting to find object")

	// Test with object in Cache
	getObj = true
	// Test with empty API cache.
	err = ov.Delete(ctx, key, nil)
	AssertOk(t, err, "expecting Delete on empty to succeed got (%s)", err)
	cobj = ov.overlay[key]
	Assert(t, cobj.oper == operDelete, "expecting operation to be delete got [%s]", cobj.oper)
	err = ov.Get(ctx, key, retObj)
	Assert(t, err != nil, "expecting get to fail for deleted object")

	// Create on empty
	err = ov.Create(ctx, key, obj)
	Assert(t, err != nil, "expecting Create to fail")
	cobj = ov.overlay[key]
	Assert(t, cobj.oper == operDelete, "expecting operation to be delete got [%s]", cobj.oper)
	err = ov.Get(ctx, key, retObj)
	Assert(t, err != nil, "expecting get to fail for deleted object")

	// Update on top of create
	obj.Spec = "Spec 2"
	err = ov.Update(ctx, key, obj)
	AssertOk(t, err, "expecting update to succeed got (%s)", err)
	cobj = ov.overlay[key]
	Assert(t, cobj.oper == operUpdate, "expecting operation to be create got [%s]", cobj.oper)
	err = ov.Get(ctx, key, retObj)
	AssertOk(t, err, "could not find object in overlay")
	Assert(t, reflect.DeepEqual(obj, retObj), "Returned object does not match expected [%v]/[]%v]", retObj, obj)

	// Delete
	err = ov.Delete(ctx, key, retObj)
	AssertOk(t, err, "expecting delete to succeed got (%s)", err)
	Assert(t, reflect.DeepEqual(obj, retObj), "Returned object does not match expected [%v]/[]%v]", retObj, obj)
	cobj = ov.overlay[key]
	Assert(t, cobj.oper == operDelete, "expecting operation to be delete got [%s]", cobj.oper)
	err = ov.Get(ctx, key, retObj)
	Assert(t, err != nil, "expecting get to fail for deleted object")
}
