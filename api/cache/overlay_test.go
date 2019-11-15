package cache

import (
	"context"
	"errors"
	"fmt"
	"reflect"
	"strings"
	"testing"

	"github.com/pensando/sw/api/utils"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/api_test"
	cachemocks "github.com/pensando/sw/api/cache/mocks"
	"github.com/pensando/sw/api/cache/ovpb"
	"github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/venice/apiserver/pkg/mocks"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
	. "github.com/pensando/sw/venice/utils/testutils"
	mocks2 "github.com/pensando/sw/venice/utils/watchstream/mocks"
)

func TestOverlayCRD(t *testing.T) {
	// Get empty
	_, err := GetOverlay("tenant1", "dummy")
	Assert(t, err != nil, "found overlay in empty overlaySingleton")

	// Create overlay
	c := &cachemocks.FakeCache{FakeKvStore: cachemocks.FakeKvStore{}, Kvconn: &cachemocks.FakeKvStore{}}
	r1, err := NewOverlay("tenant1", "new1", "/base/", c, mocks.NewFakeServer(), false)
	AssertOk(t, err, "could not create new overlay")

	r2, err := NewOverlay("tenant1", "new2", "/base/", c, mocks.NewFakeServer(), false)
	AssertOk(t, err, "could not create new overlay")
	Assert(t, len(overlaysSingleton.ovMap) == 2, "number of overlays do not match, expecting 2 got %d", len(overlaysSingleton.ovMap))

	// Get
	x1, err := GetOverlay("tenant1", "new1")
	AssertOk(t, err, "could not retrieve existing overlay")

	x2, err := GetOverlay("tenant1", "new2")
	AssertOk(t, err, "could not retrieve existing overlay")

	Assert(t, x1 == r1 && x2 == r2, "did not get correct overlay")

	x2, err = GetOverlay("tenant2", "new2")
	Assert(t, err != nil, "found overlay with wrong tenant")

	_, err = GetOverlay("tenant1", "dummy")
	Assert(t, err != nil, "found non-existent overlay")

	// Add Duplicate
	_, err = NewOverlay("tenant1", "new1", "/base/", c, mocks.NewFakeServer(), false)
	if err == nil {
		t.Fatalf("could add duplicate overlay")
	}
	ovs := GetOverlays()
	Assert(t, len(ovs) == 2, "number of overlays did not match, expecting 2 got (%d)", len(ovs))
	// Delete
	err = DelOverlay("tenant1", "dummy")
	Assert(t, err != nil, "could delete non-existent overlay")

	err = DelOverlay("tenant1", "new1")
	AssertOk(t, err, "could not delete overlay")

	err = DelOverlay("tenant1", "new2")
	AssertOk(t, err, "could not delete overlay")

	err = DelOverlay("tenant1", "new1")
	Assert(t, err != nil, "could delete already deleted overlay")

	Assert(t, len(overlaysSingleton.ovMap) == 0,
		"number of overlays do not match, expecting 0 got %d", len(overlaysSingleton.ovMap))

	r3, err := NewLocalOverlay("tenant1", "new3", "/base/", c, mocks.NewFakeServer())
	x3 := r3.(*overlay)
	Assert(t, x3.ephemeral == true, "Epehemerl flag was not set")
	SetOverlay("tenant1", "new3", r3)
	x3 = r3.(*overlay)
	Assert(t, x3.ephemeral == true, "Epehemerl flag was not set")
	err = DelOverlay("tenant1", "new3")
	AssertOk(t, err, "could not delete overlay")
}

func TestOverlayCreate(t *testing.T) {
	c := &cachemocks.FakeCache{FakeKvStore: cachemocks.FakeKvStore{}, Kvconn: &cachemocks.FakeKvStore{}}
	o, _ := NewOverlay("tenant1", "testCreate", "/base/", c, mocks.NewFakeServer(), false)
	defer DelOverlay("tenant1", "testCreate")
	ov := o.(*overlay)
	ctx := context.TODO()
	obj := &apitest.TestObj{TypeMeta: api.TypeMeta{Kind: "TestObj"}}
	obj.Name = "testObj"
	obj.Spec = "Spec 1"
	orig := &apitest.TestObj{TypeMeta: api.TypeMeta{Kind: "TestObj"}}
	orig.Name = "testObj"
	orig.Spec = "Original Spec"

	key := obj.MakeKey("overlay")
	// Create empty
	retObj := &apitest.TestObj{}
	c.FakeKvStore.Getfn = func(ctx context.Context, key string, into runtime.Object) error {
		return errors.New("NotFound")
	}

	err := ov.Get(ctx, key, retObj)
	Assert(t, err != nil, "Expecting failure on get of empty overlay and cache got [%+v]", retObj)

	// Secondary with (no cache, no overlay)
	err = ov.Create(ctx, key, obj)
	AssertOk(t, err, "failed to create in overlay")
	Assert(t, c.FakeKvStore.Creates == 0, "not expecting write to KV store got %d writes", c.FakeKvStore.Creates)

	err = ov.Get(ctx, key, retObj)
	AssertOk(t, err, "could not find object in overlay")
	Assert(t, reflect.DeepEqual(obj, retObj), "Returned object does not match expected [%v]/[]%v]", retObj, obj)

	// Secondary with (no cache, in overlay)
	obj.Spec = "Spec 2"
	err = ov.Create(ctx, key, obj)
	AssertOk(t, err, "failed to create in overlay")
	Assert(t, c.FakeKvStore.Creates == 0, "not expecting write to KV store got %d writes", c.FakeKvStore.Creates)

	err = ov.Get(ctx, key, retObj)
	AssertOk(t, err, "could not find object in overlay")
	Assert(t, reflect.DeepEqual(obj, retObj), "Returned object does not match expected [%v]/[]%v]", retObj, obj)

	// Secondary with (in cache, no overlay)
	c.FakeKvStore.Getfn = func(ctx context.Context, key string, into runtime.Object) error {
		into = retObj
		return nil
	}
	delete(ov.overlay, key)

	err = ov.Create(ctx, key, obj)
	Assert(t, err != nil, "Expecting failure for create of existing key")

	// primary with (no cache, no overlay)
	c.FakeKvStore.Getfn = func(ctx context.Context, key string, into runtime.Object) error {
		return errors.New("NotFound")
	}

	err = ov.CreatePrimary(ctx, "svc", "method1", "/uri/aa", key, orig, obj)
	AssertOk(t, err, "failed to create in overlay")
	Assert(t, c.FakeKvStore.Creates == 0, "not expecting write to KV store got %d writes", c.FakeKvStore.Creates)

	err = ov.Get(ctx, key, retObj)
	AssertOk(t, err, "could not find object in overlay")
	Assert(t, reflect.DeepEqual(obj, retObj), "Returned object does not match expected [%v]/[]%v]", retObj, obj)

	ovObj := ov.overlay[key]
	Assert(t, ovObj.primary, "not marked primary")
	Assert(t, ovObj.serviceName == "svc", "service does not match")
	Assert(t, ovObj.methodName == "method1", "method does not match")
	Assert(t, reflect.DeepEqual(orig, ovObj.orig), "stored original does not match")

	// primary with (no cache, in overlay)
	obj.Spec = "Spec 3"
	err = ov.CreatePrimary(ctx, "svc", "method1", "/uri/aa", key, orig, obj)
	Assert(t, err != nil, "Expecting failure for create of existing key")
	Assert(t, c.FakeKvStore.Creates == 0, "not expecting write to KV store got %d writes", c.FakeKvStore.Creates)

	err = ov.Get(ctx, key, retObj)
	AssertOk(t, err, "could not find object in overlay")
	Assert(t, reflect.DeepEqual(obj, retObj), "Returned object does not match expected [%v]/[]%v]", retObj, obj)

	Assert(t, ovObj.primary, "not marked primary")
	Assert(t, ovObj.serviceName == "svc", "service does not match")
	Assert(t, ovObj.methodName == "method1", "method does not match")
	Assert(t, reflect.DeepEqual(orig, ovObj.orig), "stored original does not match")

	// Primary (in cache, no overlay)
	c.FakeKvStore.Getfn = func(ctx context.Context, key string, into runtime.Object) error {
		into = retObj
		return nil
	}
	delete(ov.overlay, key)

	err = ov.CreatePrimary(ctx, "svc", "method1", "/uri/aa", key, orig, obj)
	Assert(t, err != nil, "Expecting failure for create of existing key")

	// The KV store should not have been touched anytime for anykind of write
	Assert(t, c.FakeKvStore.Creates == 0 && c.FakeKvStore.Updates == 0 && c.FakeKvStore.Deletes == 0, "backend has been written to!")
}

func TestOverlayUpdate(t *testing.T) {
	c := &cachemocks.FakeCache{FakeKvStore: cachemocks.FakeKvStore{}, Kvconn: &cachemocks.FakeKvStore{}}
	o, _ := NewOverlay("tenant1", "testUpdate", "/base/", c, mocks.NewFakeServer(), false)
	defer DelOverlay("tenant1", "testUpdate")
	ov := o.(*overlay)
	ctx := context.TODO()
	obj := &apitest.TestObj{}
	obj.Name = "testObj"
	obj.Spec = "Spec 1"
	obj.GenerationID = "1"
	orig := &apitest.TestObj{TypeMeta: api.TypeMeta{Kind: "TestObj"}}
	orig.Name = "testObj"
	orig.Spec = "Original Spec"
	serviceName := "svcname"
	methodName := "methname"
	URI := "/uri/aaa"
	key := obj.MakeKey("overlay")
	retObj := &apitest.TestObj{}
	getObj := false
	c.FakeKvStore.Getfn = func(ctx context.Context, key string, into runtime.Object) error {
		if getObj {
			retObj.Clone(into)
			return nil
		}
		return errors.New("Not Found")
	}

	// secondary Update on (no cache, no overlay)
	err := ov.Update(ctx, key, obj)
	Assert(t, err != nil, "expecting update on empty to fail")

	// primary Update on (no cache, no overlay)
	err = ov.UpdatePrimary(ctx, serviceName, methodName, URI, key, "", orig, obj, nil)
	Assert(t, err != nil, "expecting update on empty to fail")

	// update secondary (in cache, no overlay)
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
	Assert(t, cobj.primary == false, "primary on secondary update should be false")

	// update primary (in cache, no overlay)
	delete(ov.overlay, key)
	err = ov.UpdatePrimary(ctx, serviceName, methodName, URI, key, "", orig, obj, nil)
	AssertOk(t, err, "expecting update to succeed, got (%s)", err)

	err = ov.Get(ctx, key, retObj)
	AssertOk(t, err, "could not find object in overlay")

	if !reflect.DeepEqual(obj, retObj) {
		t.Errorf("Returned object does not match expected [%v]/[]%v]", retObj, obj)
	}
	cobj = ov.overlay[key]
	Assert(t, cobj.oper == operUpdate, "expecting operation to be update")
	Assert(t, cobj.primary, "not marked primary")
	Assert(t, cobj.serviceName == serviceName, "service does not match")
	Assert(t, cobj.methodName == methodName, "method does not match")
	Assert(t, reflect.DeepEqual(orig, cobj.orig), "stored original does not match")

	// update primary (in cache, no overlay) with updatefunction
	delete(ov.overlay, key)
	updatefn := func(oldObj runtime.Object) (newObj runtime.Object, err error) {
		return oldObj, nil
	}
	err = ov.UpdatePrimary(ctx, serviceName, methodName, URI, key, "", orig, obj, updatefn)
	AssertOk(t, err, "expecting update to succeed, got (%s)", err)

	err = ov.Get(ctx, key, retObj)
	AssertOk(t, err, "could not find object in overlay")

	if !reflect.DeepEqual(obj, retObj) {
		t.Errorf("Returned object does not match expected [%v]/[]%v]", retObj, obj)
	}
	cobj = ov.overlay[key]
	Assert(t, cobj.oper == operUpdate, "expecting operation to be update")
	Assert(t, cobj.primary, "not marked primary")
	Assert(t, cobj.serviceName == serviceName, "service does not match")
	Assert(t, cobj.methodName == methodName, "method does not match")
	Assert(t, reflect.DeepEqual(orig, cobj.orig), "stored original does not match")
	Assert(t, cobj.updateFn != nil, "updatefn does not match")

	// Updates with existing overlay entries, rules
	//  - Primary always takes precedence. If there is already a primary object, then a secondary update is not allowed.
	//  - Primary on top of secondary always turns the object to primary
	//  - update on create -> create
	//  - update on update  -> update
	//  - update on delete  -> fail if no cache entry exists, update if cache entry exists

	//secondary Update with existing primary entry
	obj.Spec = "Spec 2"
	err = ov.Update(ctx, key, obj)
	Assert(t, err != nil, "expecting update on primary to fail")

	// secondary update with existing secondary entry
	cobj = ov.overlay[key]
	cobj.primary = false
	cobj.serviceName = ""
	cobj.methodName = ""
	cobj.updateFn = nil
	cobj.orig = nil

	err = ov.Update(ctx, key, obj)
	AssertOk(t, err, "expecting update to succeed")

	// secondary update with existing secondary entry in a dry run
	dryRunCtx := setDryRun(ctx, 100)
	err = ov.Update(dryRunCtx, key, obj)
	AssertOk(t, err, "expecting update with existing update in dry run to succeed")

	err = ov.Get(ctx, key, retObj)
	AssertOk(t, err, "could not find object in overlay")
	Assert(t, reflect.DeepEqual(obj, retObj), "Returned object does not match expected [%v]/[]%v]", retObj, obj)

	cobj = ov.overlay[key]
	Assert(t, cobj.oper == operUpdate, "expecting operation to be update")
	Assert(t, cobj.primary == false, "expecting primary to be false")

	// Update with existing primary Create entry
	cobj = ov.overlay[key]
	getObj = false
	cobj.oper = operCreate
	cobj.primary = true
	cobj.methodName = methodName
	cobj.serviceName = serviceName
	cobj.orig = orig

	obj.Spec = "Spec 3"
	err = ov.Update(ctx, key, obj)
	Assert(t, err != nil, "expecting update with existing update to fail")

	err = ov.UpdatePrimary(ctx, serviceName, methodName, URI, key, "", orig, obj, nil)
	AssertOk(t, err, "expecting to pass")

	err = ov.Get(ctx, key, retObj)
	AssertOk(t, err, "could not find object in overlay")

	Assert(t, reflect.DeepEqual(obj, retObj), fmt.Sprintf("Returned object does not match expected [%v]/[]%v]", retObj, obj))

	cobj = ov.overlay[key]
	Assert(t, cobj.oper == operCreate, fmt.Sprintf("expecting operation to be create got %v", cobj.oper))
	Assert(t, cobj.primary, "not marked primary")
	Assert(t, cobj.serviceName == serviceName, "service does not match")
	Assert(t, cobj.methodName == methodName, "method does not match")
	Assert(t, reflect.DeepEqual(orig, cobj.orig), "stored original does not match")
	Assert(t, cobj.updateFn == nil, "updatefn does not match")

	// primary update with existing update entry
	getObj = true
	cobj = ov.overlay[key]
	cobj.oper = operUpdate
	err = ov.UpdatePrimary(ctx, serviceName, methodName, URI, key, "", orig, obj, nil)
	AssertOk(t, err, "expecting to pass")

	err = ov.Get(ctx, key, retObj)
	AssertOk(t, err, "could not find object in overlay")

	Assert(t, reflect.DeepEqual(obj, retObj), fmt.Sprintf("Returned object does not match expected [%v]/[]%v]", retObj, obj))

	cobj = ov.overlay[key]
	Assert(t, cobj.oper == operUpdate, "expecting operation to be update got %s", cobj.oper)
	Assert(t, cobj.primary, "not marked primary")
	Assert(t, cobj.serviceName == serviceName, "service does not match")
	Assert(t, cobj.methodName == methodName, "method does not match")
	Assert(t, reflect.DeepEqual(orig, cobj.orig), "stored original does not match")
	Assert(t, cobj.updateFn == nil, "updatefn does not match")

	// with existing delete entry, and in API cache.
	getObj = true
	cobj = ov.overlay[key]
	cobj.oper = operDelete

	err = ov.UpdatePrimary(ctx, serviceName, methodName, URI, key, "", orig, obj, nil)
	AssertOk(t, err, "expecting to pass")

	err = ov.Get(ctx, key, retObj)
	AssertOk(t, err, "could not find object in overlay")

	Assert(t, reflect.DeepEqual(obj, retObj), fmt.Sprintf("Returned object does not match expected [%v]/[]%v]", retObj, obj))

	cobj = ov.overlay[key]
	Assert(t, cobj.oper == operUpdate, "expecting operation to be update")
	Assert(t, cobj.primary, "not marked primary")
	Assert(t, cobj.serviceName == serviceName, "service does not match")
	Assert(t, cobj.methodName == methodName, "method does not match")
	Assert(t, reflect.DeepEqual(orig, cobj.orig), "stored original does not match")
	Assert(t, cobj.updateFn == nil, "updatefn does not match")

	// existing delete without a cache entry
	getObj = false
	cobj = ov.overlay[key]
	cobj.oper = operDelete
	err = ov.UpdatePrimary(ctx, serviceName, methodName, URI, key, "", orig, obj, nil)
	Assert(t, err != nil, "expecting update to fail")

	// Consistent update test without cache or overlay objects
	delete(ov.overlay, key)
	getObj = false

	updatefunc := func(oldObj runtime.Object) (newObj runtime.Object, err error) {
		return oldObj, nil
	}
	updatefunc1 := func(oldObj runtime.Object) (newObj runtime.Object, err error) {
		return oldObj, nil
	}
	err = ov.ConsistentUpdate(ctx, key, obj, updatefunc)
	Assert(t, err != nil, "expecting update to fail")

	// consistent update with cache object, no overlay
	getObj = true
	err = ov.ConsistentUpdate(ctx, key, obj, updatefunc)
	AssertOk(t, err, "expecting consistent update to pass")

	cobj = ov.overlay[key]
	Assert(t, cobj.oper == operUpdate, "expecting operation to be Update")
	Assert(t, reflect.ValueOf(cobj.updateFn).Pointer() == reflect.ValueOf(updatefunc).Pointer(), "did not find updatefunc populated")

	// Overwrite update func  without dry run
	err = ov.ConsistentUpdate(ctx, key, obj, updatefunc1)
	AssertOk(t, err, "expecting update to pass")

	// Overwrite update func  with dry run
	err = ov.ConsistentUpdate(dryRunCtx, key, obj, updatefunc1)
	AssertOk(t, err, "expecting update to pass")
	cobj = ov.overlay[key]
	Assert(t, cobj.oper == operUpdate, "expecting operation to be Update")
	Assert(t, reflect.ValueOf(cobj.updateFn).Pointer() == reflect.ValueOf(updatefunc1).Pointer(), "did not find updatefunc overwritten")

	// The KV store should not have been touched anytime for anykind of write
	Assert(t, c.FakeKvStore.Creates == 0 && c.FakeKvStore.Updates == 0 && c.FakeKvStore.Deletes == 0, "backend has been written to!")
}

func TestOverlayDelete(t *testing.T) {
	c := &cachemocks.FakeCache{FakeKvStore: cachemocks.FakeKvStore{}, Kvconn: &cachemocks.FakeKvStore{}}
	o, _ := NewOverlay("tenant1", "testDelete", "/base/", c, mocks.NewFakeServer(), false)
	defer DelOverlay("tenant1", "testDelete")
	ov := o.(*overlay)
	ctx := context.TODO()
	obj := &apitest.TestObj{}
	obj.Name = "testObj"
	obj.Spec = "Spec 1"
	key := obj.MakeKey("overlay")

	orig := &apitest.TestObj{TypeMeta: api.TypeMeta{Kind: "TestObj"}}
	orig.Name = "testObj"
	orig.Spec = "Original Spec"
	serviceName := "svcname"
	methodName := "methname"
	URI := "/uri/aaa"

	retObj := &apitest.TestObj{}
	getObj := false
	c.FakeKvStore.Getfn = func(ctx context.Context, key string, into runtime.Object) error {
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
	delObj := &apitest.TestObj{}
	err = ov.Delete(ctx, key, delObj)
	AssertOk(t, err, fmt.Sprintf("expecting delete to succeed, got (%s)", err))

	cobj := ov.overlay[key]
	Assert(t, cobj.oper == operDelete, "expecting operation to be delete")

	// secondary Delete on an already staged object
	err = ov.Delete(ctx, key, delObj)
	AssertOk(t, err, "expecting delete to pass")

	// delete already staged object in dry run
	dryRunCtx := setDryRun(ctx, 100)
	err = ov.Delete(dryRunCtx, key, delObj)
	AssertOk(t, err, "expecting delete to succeed, got (%s)", err)

	cobj = ov.overlay[key]
	Assert(t, cobj.oper == operDelete, "expecting operation to be delete")

	// delete with a primary object in  overlay
	delmeta := &apitest.TestObj{
		TypeMeta:   api.TypeMeta{Kind: "TestObj"},
		ObjectMeta: api.ObjectMeta{Name: "testObj"},
	}
	cobj.primary = true
	err = ov.Delete(dryRunCtx, key, delObj)
	Assert(t, err != nil, "expecting delete to fail")
	cobj.primary = false

	// Primary delete with object only in cache
	delete(ov.overlay, key)
	getObj = true
	err = ov.Get(ctx, key, retObj)
	AssertOk(t, err, "could not find object in overlay")

	err = ov.DeletePrimary(ctx, serviceName, methodName, URI, key, delmeta, delObj)
	AssertOk(t, err, "expecting Delete to succeed")
	cobj = ov.overlay[key]
	Assert(t, cobj.methodName == methodName, "method name does not match")
	Assert(t, cobj.serviceName == serviceName, "service name does not match")
	Assert(t, cobj.primary, "not set to primary")
	Assert(t, cobj.oper == operDelete, "expecting operation to be delete got %v", cobj.oper)
	Assert(t, reflect.DeepEqual(delObj, retObj), "returned object does not match cache object")

	// Primary delete with object in cache and in overlay
	cobj = ov.overlay[key]
	nobj := cobj.val.(*apitest.TestObj)
	nobj.Spec = "modified spec"
	err = ov.DeletePrimary(ctx, serviceName, methodName, URI, key, delmeta, delObj)
	AssertOk(t, err, "expecting Delete to succeed")
	Assert(t, cobj.methodName == methodName, "method name does not match")
	Assert(t, cobj.serviceName == serviceName, "service name does not match")
	Assert(t, cobj.primary, "not set to primary")
	Assert(t, cobj.oper == operDelete, "expecting operation to be delete got %v", cobj.oper)
	Assert(t, reflect.DeepEqual(delObj, nobj), "returned object does not match overlay object")

	err = ov.Get(ctx, key, retObj)
	Assert(t, err != nil, "should not find object in overlay")

	// Primary delete with object only in overlay
	getObj = false
	cobj.oper = operUpdate
	err = ov.DeletePrimary(ctx, serviceName, methodName, URI, key, delmeta, delObj)
	AssertOk(t, err, "expecting Delete to succeed")
	cobj = ov.overlay[key]
	Assert(t, cobj == nil, "expecting object to be deleted from overlay")

	// The KV store should not have been touched anytime for anykind of write
	Assert(t, c.FakeKvStore.Creates == 0 && c.FakeKvStore.Updates == 0 && c.FakeKvStore.Deletes == 0, "backend has been written to!")
}

func TestOverlayGet(t *testing.T) {
	c := &cachemocks.FakeCache{FakeKvStore: cachemocks.FakeKvStore{}, Kvconn: &cachemocks.FakeKvStore{}}
	o, _ := NewOverlay("tenant1", "testGet", "/base/", c, mocks.NewFakeServer(), false)
	defer DelOverlay("tenant1", "testGet")
	ov := o.(*overlay)
	ctx := context.TODO()
	retobj := &apitest.TestObj{}
	retobj.Name = "testObj"
	retobj.Spec = "Spec 1"
	key := retobj.MakeKey("overlay")
	obj := &apitest.TestObj{}
	getObj := false
	c.FakeKvStore.Getfn = func(ctx context.Context, key string, into runtime.Object) error {
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
	obj = &apitest.TestObj{}
	err = ov.Get(ctx, key, obj)
	AssertOk(t, err, "expecting get to succeed, got (%s)", err)
	Assert(t, reflect.DeepEqual(retobj, obj), "retrieved object does not match [%+v]\n[%+v]", retobj, obj)

	// Get with Cache and Overlay (Update)
	obj.Name = "New Item"
	err = ov.Update(ctx, key, obj)
	AssertOk(t, err, "expecting update to succeed, got (%s)", err)

	obj1 := &apitest.TestObj{}
	err = ov.Get(ctx, key, obj1)
	AssertOk(t, err, "expecting get to succeed, got (%s)", err)
	Assert(t, reflect.DeepEqual(obj, obj1), "retrieved object does not match [%+v]\n[%+v]", retobj, obj)

	// get with cache and overlay (delete)
	cobj := ov.overlay[key]
	cobj.oper = operDelete
	err = ov.Get(ctx, key, obj1)
	Assert(t, err != nil, "expecting get to fail")

	// Test with persisted get set
	pctx := apiutils.SetVar(ctx, apiutils.CtxKeyGetPersistedKV, true)
	err = ov.Get(pctx, key, obj1)
	AssertOk(t, err, "should be successful")
	Assert(t, reflect.DeepEqual(retobj, obj1), "retrieved object does not match [%+v]\n[%+v]", obj1, retobj)

	pctx = apiutils.SetVar(ctx, apiutils.CtxKeyGetPersistedKV, false)
	err = ov.Get(pctx, key, obj1)
	Assert(t, err != nil, "expecting get to fail")

	// get with no cache and overlay(create)
	getObj = false
	cobj = ov.overlay[key]
	cobj.oper = operCreate
	err = ov.Get(ctx, key, obj1)
	AssertOk(t, err, "expecting get to succeed, got (%s)", err)
	Assert(t, reflect.DeepEqual(obj, obj1), "retrieved object does not match [%+v]\n[%+v]", retobj, obj)

	// Test with persisted get set
	pctx = apiutils.SetVar(ctx, apiutils.CtxKeyGetPersistedKV, true)
	err = ov.Get(pctx, key, obj1)
	Assert(t, err != nil, "expecting get to fail")

	pctx = apiutils.SetVar(ctx, apiutils.CtxKeyGetPersistedKV, false)
	err = ov.Get(pctx, key, obj1)
	AssertOk(t, err, "expecting get to succeed, got (%s)", err)
	Assert(t, reflect.DeepEqual(obj, obj1), "retrieved object does not match [%+v]\n[%+v]", retobj, obj)

	// check get in a dryrun
	ov.Lock()
	dryRunCtx := setDryRun(ctx, 100)
	err = ov.Get(dryRunCtx, key, obj1)
	AssertOk(t, err, "expecting get to succeed, got (%s)", err)
	Assert(t, reflect.DeepEqual(obj, obj1), "retrieved object does not match [%+v]\n[%+v]", retobj, obj)
	ov.Unlock()

	// The KV store should not have been touched anytime for anykind of write
	Assert(t, c.FakeKvStore.Creates == 0 && c.FakeKvStore.Updates == 0 && c.FakeKvStore.Deletes == 0, "backend has been written to!")
}

func TestOverlayList(t *testing.T) {
	c := &cachemocks.FakeCache{FakeKvStore: cachemocks.FakeKvStore{}, Kvconn: &cachemocks.FakeKvStore{}}
	o, _ := NewOverlay("tenant1", "testList", "/base/", c, mocks.NewFakeServer(), false)
	defer DelOverlay("tenant1", "testList")
	ov := o.(*overlay)
	ctx := context.TODO()
	obj := &apitest.TestObj{}
	key := obj.MakeKey("overlay")

	retList := &apitest.TestObjList{}

	getObj := false
	c.ListFilteredFn = func(ctx context.Context, prefix string, into runtime.Object, opts api.ListWatchOptions) error {
		if getObj {
			out := into.(*apitest.TestObjList)
			out.Items = retList.Items
			return nil
		}
		return errors.New("NotFound")
	}

	validateList := func(got, exp *apitest.TestObjList) (string, bool) {
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

	tobjs := []*apitest.TestObj{
		&apitest.TestObj{
			ObjectMeta: api.ObjectMeta{
				Tenant: "tenant1",
				Name:   "test1",
			},
			Spec: "Cache1",
		},
		&apitest.TestObj{
			ObjectMeta: api.ObjectMeta{
				Tenant: "tenant2",
				Name:   "test2",
			},
			Spec: "Cache2",
		},
		&apitest.TestObj{
			ObjectMeta: api.ObjectMeta{
				Tenant: "tenant1",
				Name:   "test3",
			},
			Spec: "Cache3",
		},
	}

	modobjs := []*apitest.TestObj{
		&apitest.TestObj{
			ObjectMeta: api.ObjectMeta{
				Tenant: "tenant2",
				Name:   "testObj",
			},
			Spec: "overlay-Create",
		},
		&apitest.TestObj{
			ObjectMeta: api.ObjectMeta{
				Tenant: "tenant1",
				Name:   "test1",
			},
			Spec: "overlay-update",
		},
	}

	// List empty
	getList := &apitest.TestObjList{}
	expList := &apitest.TestObjList{}
	err := ov.List(ctx, key, getList)
	AssertOk(t, err, "List failed (%s)", err)

	if e, ok := validateList(getList, expList); !ok {
		t.Errorf(" List did not match [%s]", e)
	}
	// List with empty overlay and Cache Objects
	retList.Items = []*apitest.TestObj{tobjs[0], tobjs[1], tobjs[2]}
	expList.Items = []*apitest.TestObj{tobjs[0], tobjs[1], tobjs[2]}

	getObj = true
	err = ov.List(ctx, key, getList)
	AssertOk(t, err, "List failed")

	if e, ok := validateList(getList, expList); !ok {
		t.Errorf(" List did not match [%s]", e)
	}
	// List with mixture of overlay and Cache objects
	nkey := modobjs[0].MakeKey("overlay")
	retObj := &apitest.TestObj{}
	c.FakeKvStore.Getfn = func(ctx context.Context, key string, into runtime.Object) error {
		return errors.New("NotFound")
	}

	err = ov.Get(ctx, nkey, retObj)
	Assert(t, err != nil, "expecting failure on get of empty overlay and cache got [%+v]", retObj)

	err = ov.Create(ctx, nkey, modobjs[0])
	AssertOk(t, err, "failed to create in overlay")

	// Create one like the one in cache
	nkey = modobjs[1].MakeKey("overlay")
	c.FakeKvStore.Getfn = func(ctx context.Context, key string, into runtime.Object) error {
		modobjs[1].Clone(into)
		return nil
	}
	err = ov.Update(ctx, nkey, modobjs[1])
	AssertOk(t, err, "failed to update in overlay (%s)", err)

	retList.Items = []*apitest.TestObj{tobjs[0], tobjs[1], tobjs[2]}
	expList.Items = []*apitest.TestObj{modobjs[1], tobjs[1], tobjs[2], modobjs[0]}
	getObj = true
	err = ov.List(ctx, key, getList)
	AssertOk(t, err, "List failed")

	if e, ok := validateList(getList, expList); !ok {
		t.Errorf(" List did not match [%s]", e)
	}
	// List with mixture of overlay and Cache objects filtered - tenant specified
	opts := api.ListWatchOptions{}
	opts.Tenant = "tenant2"
	retList.Items = []*apitest.TestObj{tobjs[1]}
	expList.Items = []*apitest.TestObj{modobjs[0], tobjs[1]}

	err = ov.ListFiltered(ctx, key, getList, opts)
	AssertOk(t, err, "List failed")
	if e, ok := validateList(getList, expList); !ok {
		t.Errorf(" List did not match [%s]", e)
	}

	opts.Tenant = "tenant1"
	retList.Items = []*apitest.TestObj{tobjs[0], tobjs[2]}
	expList.Items = []*apitest.TestObj{modobjs[1], tobjs[2]}
	err = ov.ListFiltered(ctx, key, getList, opts)
	AssertOk(t, err, "List failed")
	if e, ok := validateList(getList, expList); !ok {
		t.Errorf(" List did not match [%s]", e)
	}

	opts.Tenant = "tenant3"
	retList.Items = []*apitest.TestObj{}
	expList.Items = []*apitest.TestObj{}
	err = ov.ListFiltered(ctx, key, getList, opts)
	AssertOk(t, err, "List failed")
	if e, ok := validateList(getList, expList); !ok {
		t.Errorf(" List did not match [%s]", e)
	}

	// with only overlay objects
	retList.Items = []*apitest.TestObj{}
	expList.Items = []*apitest.TestObj{modobjs[0], modobjs[1]}
	getObj = true
	err = ov.List(ctx, key, getList)
	AssertOk(t, err, "List failed")
	if e, ok := validateList(getList, expList); !ok {
		t.Errorf(" List did not match [%s]", e)
	}

	// test as part of dryRun
	ov.Lock()
	dryRunCtx := setDryRun(ctx, 100)
	err = ov.List(dryRunCtx, key, getList)
	AssertOk(t, err, "List failed")
	if e, ok := validateList(getList, expList); !ok {
		t.Errorf(" List did not match [%s]", e)
	}
	ov.Unlock()
}

func TestParseParsePath(t *testing.T) {
	c := &cachemocks.FakeCache{FakeKvStore: cachemocks.FakeKvStore{}, Kvconn: &cachemocks.FakeKvStore{}}
	o, _ := NewOverlay("tenant1", "testUpdate", "/base", c, mocks.NewFakeServer(), false)
	defer DelOverlay("tenant1", "testUpdate")
	ov := o.(*overlay)

	cases := []struct {
		input, tenant, id string
	}{
		{"/base/tenant/xyz/buffer/buf1/aaa", "xyz", "buf1"},
		{"/base/tenant/xyz-11/buffer/bufa-adad/aaa/aadadad/", "xyz-11", "bufa-adad"},
		{"/base/tenant/xyz_11/buffer/bufa_adad/aaa/aadadad/aa", "xyz_11", "bufa_adad"},
		{"/base/tenant/xyz.11/buffer/bufa.adad/aaa/aadadad/aa", "xyz.11", "bufa.adad"},
	}
	for _, v := range cases {
		tn, i, err := ov.parseKey(v.input)
		if err != nil {
			t.Errorf("failed to parse key")
		}
		if tn != v.tenant || i != v.id {
			t.Errorf("want [%v/%v] got [%v/%v]", v.tenant, v.id, tn, i)
		}
	}
}

type handlerResp struct {
	err    error
	obj    interface{}
	callFn func(ctx context.Context) (interface{}, error)
}

func TestVerify(t *testing.T) {
	c := &cachemocks.FakeCache{FakeKvStore: cachemocks.FakeKvStore{}, Kvconn: &cachemocks.FakeKvStore{}}
	o, _ := NewOverlay("tenant1", "testGet", "/base/", c, mocks.NewFakeServer(), false)
	defer DelOverlay("tenant1", "testGet")
	ov := o.(*overlay)
	ctx := context.TODO()
	server := mocks.NewFakeServer()
	service := mocks.NewFakeService().(*mocks.FakeService)
	method := mocks.NewFakeMethod(false).(*mocks.FakeMethod)
	service.AddMethod("TestMethod", method)
	server.SvcMap["TestService"] = service
	ov.server = server
	var handleError error
	obj1 := apitest.TestObj{}
	obj1.Name = "testObj1"
	obj1.Spec = "Spec 1"

	retObj := &apitest.TestObj{}
	retObj.Name = "testObj"
	retObj.Spec = "return Object"
	errorObjs := make(map[string]*handlerResp)
	handleCount := 0
	handleFunc := func(hctx context.Context, i interface{}) (interface{}, error) {
		handleCount++
		r := i.(apitest.TestObj)
		t.Logf("handler got obj %s", r.Name)
		if v := errorObjs[r.Name]; v != nil {
			t.Logf("handler returning error for %s", r.Name)
			if v.callFn != nil {
				return v.callFn(hctx)
			}
			return v.obj, v.err
		}
		return retObj, handleError
	}
	method.HandleMethod = handleFunc

	getObj := false
	c.FakeKvStore.Getfn = func(ctx context.Context, key string, into runtime.Object) error {
		if getObj {
			retObj.Clone(into)
			return nil
		}
		return errors.New("Not Found")
	}

	// insert multiple primary and multiple secondary into overlay
	key1 := "/test/key1"
	key2 := "/test/key2"
	key3 := "/test/key3"
	key4 := "/test/key4"
	obj2 := obj1
	obj2.Name = "testObj2"
	obj3 := obj1
	obj3.Name = "testObj3"
	obj4 := obj1
	obj4.Name = "testObj4"
	err := ov.CreatePrimary(ctx, "TestService", "TestMethod", "/testURI/testobj1", key1, &obj1, &obj1)
	AssertOk(t, err, "expecting CreatePrimary to pass")
	err = ov.CreatePrimary(ctx, "TestService", "TestMethod", "/testURI/testobj2", key2, &obj2, &obj2)
	AssertOk(t, err, "expecting CreatePrimary to pass")
	err = ov.Create(ctx, key3, &obj3)
	AssertOk(t, err, "expecting Create to pass")
	err = ov.Create(ctx, key4, &obj4)
	AssertOk(t, err, "expecting Create to pass")
	reslt, err := ov.Verify(ctx)
	AssertOk(t, err, "expecting verify to pass")
	Assert(t, len(reslt.Items) == 2, "expecting 2 objects in result, got %d", len(reslt.Items))
	Assert(t, len(reslt.Failed) == 0, "expecting no failures")
	Assert(t, handleCount == 2, "expecting 2 handle invocations, got %d", handleCount)
	handleCount = 0
	// Verify with Errors in Primary
	errorObjs["testObj2"] = &handlerResp{
		err: errors.New("testOBj2 error"),
		obj: nil,
	}
	reslt, err = ov.Verify(ctx)
	AssertOk(t, err, "expecting verify to pass")
	Assert(t, len(reslt.Items) == 2, "expecting 2 objects in result, got %d", len(reslt.Items))
	Assert(t, len(reslt.Failed) == 1, "expecting failures got %d [ %v]", len(reslt.Failed), reslt.Failed)
	Assert(t, strings.Contains(reslt.Failed[0].Errors[0].Error(), "testOBj2 error"), "Not expected error (%s)", reslt.Failed[0].Errors[0].Error())
	Assert(t, handleCount == 2, "expecting 2 handle invocations, got %d", handleCount)
	handleCount = 0

	// Verify with Errors in secondary
	delete(errorObjs, "testObj2")
	errorObjs["testObj3"] = &handlerResp{
		err: errors.New("testObj3 error"),
		obj: nil,
	}
	reslt, err = ov.Verify(ctx)
	AssertOk(t, err, "expecting verify to pass")
	Assert(t, len(reslt.Items) == 2, "expecting 2 objects in result, got %d", len(reslt.Items))
	Assert(t, len(reslt.Failed) == 0, "expecting failures got %d", len(reslt.Failed))
	Assert(t, handleCount == 2, "expecting 2 handle invocations, got %d", handleCount)
	handleCount = 0
}

func TestClearBuffer(t *testing.T) {
	c := &cachemocks.FakeCache{FakeKvStore: cachemocks.FakeKvStore{}, Kvconn: &cachemocks.FakeKvStore{}}
	o, _ := NewOverlay("tenant1", "testGet", "/base/", c, mocks.NewFakeServer(), false)
	defer DelOverlay("tenant1", "testGet")
	ov := o.(*overlay)
	ctx := context.TODO()
	getObj := false
	retObj := &apitest.TestObj{}
	retObj.Name = "testObj"
	retObj.Spec = "return Object"
	c.FakeKvStore.Getfn = func(ctx context.Context, key string, into runtime.Object) error {
		if getObj {
			retObj.Clone(into)
			return nil
		}
		return errors.New("Not Found")
	}

	obj1 := apitest.TestObj{}
	obj1.Name = "testObj1"
	obj1.Spec = "Spec 1"
	obj2 := obj1
	obj2.Name = "testObj2"
	obj3 := obj1
	obj3.Name = "testObj3"
	obj4 := obj1
	obj4.Name = "testObj4"
	key1 := "/test/key1"
	key2 := "/test/key2"
	key3 := "/test/key3"
	key4 := "/test/key4"
	uri1 := "/testURI/testobj1"
	uri2 := "/testURI/testobj2"
	err := ov.CreatePrimary(ctx, "TestService", "TestMethod", uri1, key1, &obj1, &obj1)
	AssertOk(t, err, "expecting CreatePrimary to pass")
	err = ov.CreatePrimary(ctx, "TestService", "TestMethod", uri2, key2, &obj2, &obj2)
	AssertOk(t, err, "expecting CreatePrimary to pass")
	err = ov.CreatePrimary(ctx, "TestService", "TestMethod", "/testURI/testobj3", key3, &obj3, &obj3)
	AssertOk(t, err, "expecting CreatePrimary to pass")
	err = ov.CreatePrimary(ctx, "TestService", "TestMethod", "/testURI/testobj4", key4, &obj4, &obj4)
	Assert(t, len(ov.overlay) == 4, "expecting number of entries in overlay to be 4 got %d", len(ov.overlay))

	// Delete some of the entries in the overlay
	clear := []apiintf.OverlayKey{{URI: uri1}, {URI: uri2}}
	err = ov.ClearBuffer(ctx, clear)
	AssertOk(t, err, "expecting Clear to pass")
	Assert(t, len(ov.overlay) == 2, "expecting number of entries in overlay to be 4 got %d", len(ov.overlay))

	// Clear all entries
	err = ov.ClearBuffer(ctx, nil)
	AssertOk(t, err, "expecting Clear to pass")
	Assert(t, len(ov.overlay) == 0, "expecting number of entries in overlay to be 4 got %d", len(ov.overlay))
}

func TestCommit(t *testing.T) {
	fkv := &cachemocks.FakeKvStore{}
	str := &cachemocks.FakeStore{}
	// c := &cachemocks.FakeCache{fakeKvStore: fakeKvStore{}, kvconn: fkv}
	fakeqs := &mocks2.FakeWatchPrefixes{}
	c := &cache{
		store:  str,
		pool:   &connPool{},
		queues: fakeqs,
		logger: log.GetNewLogger(log.GetDefaultConfig("cacheTest")),
		active: true,
	}
	c.pool.AddToPool(fkv)
	o, _ := NewOverlay("tenant1", "testGet", "/base/", c, mocks.NewFakeServer(), false)
	txn := &cachemocks.FakeTxn{}
	fkv.Txn = txn
	ov := o.(*overlay)
	ctx := context.TODO()
	server := mocks.NewFakeServer()
	service := mocks.NewFakeService().(*mocks.FakeService)
	method := mocks.NewFakeMethod(false).(*mocks.FakeMethod)
	service.AddMethod("TestMethod", method)
	server.SvcMap["TestService"] = service
	ov.server = server
	var handleError error
	obj1 := apitest.TestObj{}
	obj1.Name = "testObj1"
	obj1.Spec = "Spec 1"

	retObj := &apitest.TestObj{}
	retObj.Name = "testObj"
	retObj.Spec = "return Object"
	errorObjs := make(map[string]*handlerResp)
	name2key := make(map[string]string)
	handleCount := 0
	handleFunc := func(hctx context.Context, i interface{}) (interface{}, error) {
		handleCount++
		r := i.(apitest.TestObj)
		t.Logf("handler got obj %s", r.Name)
		var verVer int64
		dm := getDryRun(hctx)
		if dm != nil {
			verVer = dm.verVer
		}
		if v, ok := name2key[r.Name]; ok {
			ovobj := ov.overlay[v]
			ovobj.verVer = verVer
		}
		if v := errorObjs[r.Name]; v != nil {
			if v.callFn != nil {
				t.Logf("handler calling function for %s", r.Name)
				return v.callFn(hctx)
			}
			t.Logf("handler returning error for %s", r.Name)
			return v.obj, v.err
		}

		return retObj, handleError
	}
	method.HandleMethod = handleFunc

	getObj := false
	fkv.Getfn = func(ctx context.Context, key string, into runtime.Object) error {
		if getObj {
			retObj.Clone(into)
			return nil
		}
		return errors.New("Not Found")
	}
	var retErr []error
	var errCount int
	commitFn := func(ctx context.Context) (kvstore.TxnResponse, error) {
		resp := kvstore.TxnResponse{}
		resp.Succeeded = true
		if len(retErr) == 0 {
			return resp, nil
		}
		err := retErr[errCount%len(retErr)]
		if err != nil {
			kerr := err.(*kvstore.KVError)
			fmt.Printf("*** returning error [%v] [%+v]\n", kerr.Code, kerr)
		}

		errCount++
		return resp, err
	}

	getfn := func(key string) (runtime.Object, error) {
		return nil, errors.New("Not Found")
	}
	str.Getfn = getfn
	txn.Commitfn = commitFn
	// insert multiple primary and multiple secondary into overlay
	key1 := "/test/key1"
	key2 := "/test/key2"
	key3 := "/test/key3"
	key4 := "/test/key4"
	obj2 := obj1
	obj2.Name = "testObj2"
	obj3 := obj1
	obj3.Name = "testObj3"
	obj4 := obj1
	obj4.Name = "testObj4"
	name2key["testObj1"] = key1
	name2key["testObj2"] = key2
	name2key["testObj3"] = key3
	name2key["testObj4"] = key4
	obj3Hdlr := func(inctx context.Context) (interface{}, error) {
		ov.Create(inctx, key3, &obj3)
		return obj3, nil
	}
	errorObjs["testObj2"] = &handlerResp{
		err:    nil,
		obj:    obj3,
		callFn: obj3Hdlr,
	}
	err := ov.CreatePrimary(ctx, "TestService", "TestMethod", "/testURI/testobj1", key1, &obj1, &obj1)
	AssertOk(t, err, "expecting CreatePrimary to pass")
	err = ov.CreatePrimary(ctx, "TestService", "TestMethod", "/testURI/testobj2", key2, &obj2, &obj2)
	AssertOk(t, err, "expecting CreatePrimary to pass")
	err = ov.Create(ctx, key3, &obj3)
	AssertOk(t, err, "expecting Create to pass")
	err = ov.Create(ctx, key4, &obj4)

	// Verify Commit should pass
	err = ov.Commit(ctx, nil)
	AssertOk(t, err, "expecting Commit to pass")
	Assert(t, len(txn.Ops) == 5, "expecgting 4 operations in the transaction got %v", len(txn.Ops))

	// commit should pass with retry
	ov.ephemeral = true
	retErr = append(retErr, &kvstore.KVError{Code: kvstore.ErrCodeTxnFailed})
	retErr = append(retErr, nil)
	err = ov.Commit(ctx, nil)
	AssertOk(t, err, "expecting Commit to pass")

	Assert(t, len(ov.overlay) == 0, "expecting overlay to be cleaned up after commit")
	// Verify all 4 objects are present in the transaction
	baseKey := "/base/tenant/tenant1/buffer/testGet"
	keyMap := map[string]bool{key1: false, key2: false, baseKey + key1: false, baseKey + key2: false, key3: false}
	for _, v := range txn.Ops {
		if _, ok := keyMap[v.Key]; !ok {
			t.Fatalf("Did not find key [%s] in transaction", v.Key)
		}
		delete(keyMap, v.Key)
	}
	Assert(t, len(keyMap) == 0, "did not find following keys in transaction %v", keyMap)
	DelOverlay("tenant1", "testGet")

}

func TestLargeBufferCommit(t *testing.T) {
	fkv := &cachemocks.FakeKvStore{}
	str := &cachemocks.FakeStore{}
	// c := &cachemocks.FakeCache{fakeKvStore: fakeKvStore{}, kvconn: fkv}
	fakeqs := &mocks2.FakeWatchPrefixes{}
	c := &cache{
		store:  str,
		pool:   &connPool{},
		queues: fakeqs,
		logger: log.GetNewLogger(log.GetDefaultConfig("cacheTest")),
		active: true,
	}
	c.pool.AddToPool(fkv)
	o, _ := NewOverlay("tenant1", "testGet", "/base/", c, mocks.NewFakeServer(), false)
	txn := &cachemocks.FakeTxn{}
	fkv.Txn = txn
	ov := o.(*overlay)
	ctx := context.TODO()
	server := mocks.NewFakeServer()
	service := mocks.NewFakeService().(*mocks.FakeService)
	method := mocks.NewFakeMethod(false).(*mocks.FakeMethod)
	service.AddMethod("TestMethod", method)
	server.SvcMap["TestService"] = service
	ov.server = server
	var handleError error
	obj1 := apitest.TestObj{}
	obj1.Name = "testObj1"
	obj1.Spec = "Spec 1"

	retObj := &apitest.TestObj{}
	retObj.Name = "testObj"
	retObj.Spec = "return Object"
	errorObjs := make(map[string]*handlerResp)
	name2key := make(map[string]string)
	handleCount := 0
	handleFunc := func(hctx context.Context, i interface{}) (interface{}, error) {
		handleCount++
		r := i.(apitest.TestObj)
		t.Logf("handler got obj %s", r.Name)
		var verVer int64
		dm := getDryRun(hctx)
		if dm != nil {
			verVer = dm.verVer
		}
		if v, ok := name2key[r.Name]; ok {
			ovobj := ov.overlay[v]
			ovobj.verVer = verVer
		}
		if v := errorObjs[r.Name]; v != nil {
			if v.callFn != nil {
				t.Logf("handler calling function for %s", r.Name)
				return v.callFn(hctx)
			}
			t.Logf("handler returning error for %s", r.Name)
			return v.obj, v.err
		}

		return retObj, handleError
	}
	method.HandleMethod = handleFunc

	getObj := false
	fkv.Getfn = func(ctx context.Context, key string, into runtime.Object) error {
		if getObj {
			retObj.Clone(into)
			return nil
		}
		return errors.New("Not Found")
	}
	var retErr []error
	var errCount int
	commitFn := func(ctx context.Context) (kvstore.TxnResponse, error) {
		resp := kvstore.TxnResponse{}
		resp.Succeeded = true
		if len(retErr) == 0 {
			return resp, nil
		}
		err := retErr[errCount%len(retErr)]
		errCount++
		return resp, err
	}

	getfn := func(key string) (runtime.Object, error) {
		return nil, errors.New("Not Found")
	}
	str.Getfn = getfn
	txn.Commitfn = commitFn
	// insert multiple primary and multiple secondary into overlay
	keys := [100]string{}
	objs := [100]apitest.TestObj{}
	for i := 0; i < 100; i++ {
		keys[i] = fmt.Sprintf("/test/key%d", i)
		objs[i] = apitest.TestObj{
			ObjectMeta: api.ObjectMeta{
				Name: fmt.Sprintf("testObj-%d", i),
			},
			Spec: fmt.Sprintf("spec for obj %d", i),
		}
		name2key[objs[i].Name] = keys[i]
		if i%2 == 0 {
			err := ov.CreatePrimary(ctx, "TestService", "TestMethod", fmt.Sprintf("/testURI/testobj%d", i), keys[i], &objs[i], &objs[i])
			AssertOk(t, err, "expecting CreatePrimary to pass")
		} else {
			err := ov.Create(ctx, keys[i], &objs[i])
			AssertOk(t, err, "expecting Create to pass")
		}
	}

	ov.maxOvEntries = 4
	// Verify Commit should pass
	err := ov.Commit(ctx, nil)
	AssertOk(t, err, "expecting Commit to pass")
	Assert(t, len(txn.Ops) == 100, "expecgting 100 operations in the transaction got %v", len(txn.Ops))

	// commit should pass with retry
	ov.ephemeral = true
	retErr = append(retErr, &kvstore.KVError{Code: kvstore.ErrCodeTxnFailed})
	retErr = append(retErr, nil)
	err = ov.Commit(ctx, nil)
	AssertOk(t, err, "expecting Commit to pass")

	Assert(t, len(ov.overlay) == 0, "expecting overlay to be cleaned up after commit")
	DelOverlay("tenant1", "testGet")

}

func TestOverlayRestore(t *testing.T) {
	fakeKv := &cachemocks.FakeKvStore{}
	fserver := mocks.NewFakeServer()
	fservice := mocks.NewFakeService()
	fmethod1 := mocks.NewFakeMethod(true)
	fmethod2 := mocks.NewFakeMethod(true)
	fservice.AddMethod("method1", fmethod1)
	fservice.AddMethod("method2", fmethod2)
	fserver.SvcMap["service1"] = fservice.(*mocks.FakeService)
	c := &cachemocks.FakeCache{FakeKvStore: cachemocks.FakeKvStore{}, Kvconn: fakeKv}
	o1, _ := NewOverlay("tenant1", "testCreate1", "/base/", c, fserver, false)
	defer DelOverlay("tenant1", "testCreate1")
	ov1 := o1.(*overlay)
	o2, _ := NewOverlay("tenant1", "testCreate2", "/base/", c, fserver, false)
	defer DelOverlay("tenant1", "testCreate2")
	ov2 := o2.(*overlay)
	ctx := context.TODO()

	// Create empty
	fakeDB := make(map[string]overlaypb.BufferItem)
	fakeKv.Createfn = func(Ctx context.Context, key string, in runtime.Object) error {
		obj := in.(*overlaypb.BufferItem)
		fakeDB[key] = *obj
		return nil
	}
	fakeKv.Listfn = func(ctx context.Context, key string, into runtime.Object) error {
		obj := into.(*overlaypb.BufferItemList)
		for _, v := range fakeDB {
			r := v
			obj.Items = append(obj.Items, &r)
		}
		return nil
	}
	c.FakeKvStore.Getfn = func(ctx context.Context, key string, into runtime.Object) error {
		return errors.New("NotFound")
	}
	crObj1 := &apitest.TestObj{
		ObjectMeta: api.ObjectMeta{Name: "Object1"},
		Spec:       "Spec1",
	}
	key1 := "/test/key1"
	crObj2 := &apitest.TestObj{
		ObjectMeta: api.ObjectMeta{Name: "Object2"},
		Spec:       "Spec2",
	}
	key2 := "/test/key2"
	crObj3 := &apitest.TestObj{
		ObjectMeta: api.ObjectMeta{Name: "Object3"},
		Spec:       "Spec3",
	}
	key3 := "/test/key3"

	err := ov1.CreatePrimary(ctx, "service1", "method1", "/test/uri1", key1, crObj1, crObj1)
	AssertOk(t, err, "failed to create in overlay")
	Assert(t, c.FakeKvStore.Creates == 0, "not expecting write to KV store got %d writes", c.FakeKvStore.Creates)
	Assert(t, fakeKv.Creates == 1, "expecting 1 write to KV store got %d writes", fakeKv.Creates)

	err = ov1.CreatePrimary(ctx, "service1", "method2", "/test/uri2", key2, crObj2, crObj2)
	AssertOk(t, err, "failed to create in overlay")
	Assert(t, c.FakeKvStore.Creates == 0, "not expecting write to KV store got %d writes", c.FakeKvStore.Creates)
	Assert(t, fakeKv.Creates == 2, "expecting 2 writes to KV store got %d writes", fakeKv.Creates)

	err = ov2.CreatePrimary(ctx, "service1", "method1", "/test/uri3", key3, crObj3, crObj3)
	AssertOk(t, err, "failed to create in overlay")
	Assert(t, c.FakeKvStore.Creates == 0, "not expecting write to KV store got %d writes", c.FakeKvStore.Creates)
	Assert(t, fakeKv.Creates == 3, "not expecting 3 writes to KV store got %d writes", fakeKv.Creates)

	// delete the overlays db
	overlaysSingleton.ovMap = make(map[string]apiintf.OverlayInterface)

	restoreOverlays(ctx, "/base/", c, fserver)
	Assert(t, len(overlaysSingleton.ovMap) == 2, "expecting overlays to be restored found [%d][%+v]", len(overlaysSingleton.ovMap))
	Assert(t, len(ov1.overlay) == 2, "expecting 2 objects in overlay got [%d]", len(ov1.overlay))
	Assert(t, len(ov2.overlay) == 1, "expecting 1 objects in overlay got [%d]", len(ov2.overlay))
	// Verify contents of a key
	expMap := map[string]*overlayObj{
		"/test/key1": &overlayObj{
			oper:        operCreate,
			primary:     true,
			serviceName: "service1",
			methodName:  "method1",
			key:         "/test/key1",
			URI:         "/test/uri1",
			orig:        crObj1,
			val:         crObj1,
		},
		"/test/key2": &overlayObj{
			oper:        operCreate,
			primary:     true,
			serviceName: "service1",
			methodName:  "method2",
			key:         "/test/key2",
			URI:         "/test/uri2",
			orig:        crObj2,
			val:         crObj2,
		},
	}

	for k, v := range ov1.overlay {
		v1, ok := expMap[k]
		Assert(t, ok, "key not expected [%v]", k)
		Assert(t, v.methodName == v1.methodName, "Method name does not match got[%v] want [%v]", v.methodName, v1.methodName)
		Assert(t, v.serviceName == v1.serviceName, "serviceName does not match got[%v] want [%v]", v.serviceName, v1.serviceName)
		Assert(t, v.key == v1.key, "key does not match got[%v] want [%v]", v.key, v1.key)
		Assert(t, v.URI == v1.URI, "uri does not match got[%v] want [%v]", v.URI, v1.URI)
		Assert(t, reflect.DeepEqual(v.orig, v1.orig), "object [%v] does not match \n got[%+v]\nwant[%+v]", k, v.orig, v1.orig)
	}
}

func TestOverlayTxn(t *testing.T) {
	fakeKv := &cachemocks.FakeKvStore{}
	fserver := mocks.NewFakeServer()
	fservice := mocks.NewFakeService()
	fmethod1 := mocks.NewFakeMethod(true)
	fmethod2 := mocks.NewFakeMethod(true)
	fservice.AddMethod("method1", fmethod1)
	fservice.AddMethod("method2", fmethod2)
	fserver.SvcMap["service1"] = fservice.(*mocks.FakeService)
	c := &cachemocks.FakeCache{FakeKvStore: cachemocks.FakeKvStore{}, Kvconn: fakeKv}
	ov, _ := NewOverlay("tenant1", "testCreate1", "/base/", c, fserver, false)
	defer DelOverlay("tenant1", "testCreate1")
	fov := ov.(*overlay)
	txn1 := overlayTxn{ov: fov, context: context.TODO()}
	txn2 := overlayTxn{ov: fov, context: context.TODO()}
	ctx := context.Background()
	dctx := setDryRun(ctx, 10)
	wctx := setPreCommitApply(ctx)
	Assert(t, isPreCommitApply(wctx), "did not return true for CommitApply")
	Assert(t, !isPreCommitApply(dctx), "returned true for CommitApply")
	Assert(t, IsDryRun(dctx), "returned true for CommitApply")
	Assert(t, !IsDryRun(ctx), "returned true for CommitApply")
	key := "/test/abc"
	testobj := &apitest.TestObj{}

	err := txn1.Create(key, testobj)
	AssertOk(t, err, "Create failed")
	err = txn2.Create(key, testobj)
	AssertOk(t, err, "Create failed")

	err = txn1.Update(key, testobj)
	AssertOk(t, err, "update failed")
	err = txn2.Update(key, testobj)
	AssertOk(t, err, "update failed")

	err = txn1.Delete(key)
	AssertOk(t, err, "delete failed")
	err = txn2.Delete(key)
	AssertOk(t, err, "delete failed")

	err = txn1.Touch(key)
	AssertOk(t, err, "touch failed")
	err = txn2.Touch(key)
	AssertOk(t, err, "touch failed")

	Assert(t, !txn1.IsEmpty(), "should not be empty")
}
