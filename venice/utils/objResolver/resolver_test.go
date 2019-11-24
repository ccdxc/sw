package objResolver

import (
	"context"
	"errors"
	"fmt"
	"strconv"
	"sync"

	"testing"
	"time"

	"github.com/pensando/sw/api"
	apiintf "github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
	. "github.com/pensando/sw/venice/utils/testutils"
)

type ctkitBaseCtx struct {
	//resolveState resolveState //resolve state
	ResolveCtx
}

type testObjData struct {
	fieldA string
	fieldB string
	fieldC string
}

type testObj struct {
	sync.Mutex
	api.TypeMeta
	api.ObjectMeta
	testObjData
	depMap     map[string]*apiintf.ReferenceObj
	revDepMap  map[string]*apiintf.ReferenceObj
	HandlerCtx interface{}
	ct         *ctrler
}

type testObjCtx struct {
	ctkitBaseCtx
	event  kvstore.WatchEventType
	obj    *testObj    //
	newObj *testObjCtx //update
}
type kindStore struct {
	sync.Mutex
	objects map[string]apiintf.CtkitObject
}

func (ctx *testObjCtx) Lock() {
	ctx.obj.Lock()
}

func (ctx *testObjCtx) Unlock() {
	ctx.obj.Unlock()
}

func (ctx *testObjCtx) GetKind() string {
	return ctx.obj.TypeMeta.Kind
}

func (ctx *testObjCtx) GetKey() string {
	return ctx.obj.GetKey()
}

func (ctx *testObjCtx) SetEvent(event kvstore.WatchEventType) {
	ctx.event = event
}

func (ctx *testObjCtx) References() map[string]apiintf.ReferenceObj {
	resp := make(map[string]apiintf.ReferenceObj)
	ctx.obj.References(ctx.obj.GetObjectMeta().Name, ctx.obj.GetObjectMeta().Namespace, resp)
	return resp
}

func (ctx *testObjCtx) SetNewObj(newObj apiintf.CtkitObject) {
	if newObj == nil {
		ctx.newObj = nil
	} else {
		ctx.newObj = newObj.(*testObjCtx)
		ctx.newObj.obj.HandlerCtx = ctx.obj.HandlerCtx
	}
}

func (ctx *testObjCtx) GetNewObj() apiintf.CtkitObject {
	return ctx.newObj
}

func (ctx *testObjCtx) Copy(apiintf.CtkitObject) {
	ctx.obj.testObjData = ctx.newObj.obj.testObjData
}

func (ctx *testObjCtx) RuntimeObject() runtime.Object {
	/*var v interface{}
	v = ctx.obj
	return v.(runtime.Object)*/
	return nil
}

type ctrler struct {
	sync.Mutex
	objResolver *ObjectResolver       // obj resolver for ctkit
	kinds       map[string]*kindStore // DB of all kinds
}

func (ct *ctrler) resolveObject(event kvstore.WatchEventType, workObj apiintf.CtkitObject) error {
	return ct.objResolver.Resolve(event, workObj)
}

type Controller interface {
}

func (ct *ctrler) Reset() {
	ct.objResolver = NewObjectResolver(ct)
	ct.kinds = make(map[string]*kindStore)

}
func (ct *ctrler) getKindStore(kind string) *kindStore {

	ct.Lock()
	defer ct.Unlock()

	ks, ok := ct.kinds[kind]
	if !ok {
		ks = &kindStore{
			objects: make(map[string]apiintf.CtkitObject),
		}
		ct.kinds[kind] = ks
	}
	return ct.kinds[kind]
}

func (ct *ctrler) GetObjectStore(kind string) apiintf.ObjectStore {
	return ct.getKindStore(kind)

}

func (ks *kindStore) GetObject(key string) (apiintf.CtkitObject, error) {

	ks.Lock()
	defer ks.Unlock()

	obj, ok := ks.objects[key]
	if !ok {
		fmt.Printf("Object not found object %v\n", key)
		return nil, fmt.Errorf("Object %s not found", key)
	}

	return obj, nil
}

func (ks *kindStore) AddObject(obj apiintf.CtkitObject) error {

	ks.Lock()
	defer ks.Unlock()

	ks.objects[obj.GetKey()] = obj

	return nil
}

func (ks *kindStore) DeleteObject(key string) error {

	ks.Lock()
	defer ks.Unlock()

	obj, ok := ks.objects[key]
	if !ok {
		fmt.Printf("Object not found object %v\n", key)
		return fmt.Errorf("Object %s not found", key)
	}

	delete(ks.objects, obj.GetKey())
	return nil
}

func (ct *ctrler) ResolvedRun(obj apiintf.CtkitObject) {
	//Call back
	go obj.WorkFunc(context.Background())
}

func newCtrler() *ctrler {
	ctrl := &ctrler{}
	ctrl.objResolver = NewObjectResolver(ctrl)
	ctrl.kinds = make(map[string]*kindStore)
	return ctrl
}

var ctrlerInst *ctrler

func init() {
	ctrlerInst = newCtrler()
}

type eventsRcvd struct {
	sync.Mutex
	evKindMap map[kvstore.WatchEventType]map[string]int
}

var evtRcvd eventsRcvd

func (e *eventsRcvd) Reset() {
	e.Lock()
	defer e.Unlock()
	e.evKindMap = make(map[kvstore.WatchEventType]map[string]int)
	e.evKindMap[kvstore.Created] = make(map[string]int)
	e.evKindMap[kvstore.Deleted] = make(map[string]int)
	e.evKindMap[kvstore.Updated] = make(map[string]int)
}

func (e *eventsRcvd) Update(evt kvstore.WatchEventType, kind string) {
	e.Lock()
	defer e.Unlock()
	kindMap, _ := e.evKindMap[evt]
	kindMap[kind]++
}

func (e *eventsRcvd) Equal(other *eventsRcvd) bool {
	e.Lock()
	defer e.Unlock()

	for evt, kindMap := range e.evKindMap {
		otherKindMap, _ := other.evKindMap[evt]
		if len(otherKindMap) != len(kindMap) {
			return false
		}
		for kind, cnt := range kindMap {
			if otherKindMap[kind] != cnt {
				return false
			}
		}
	}
	return true
}

func (e *eventsRcvd) Diff(exp *eventsRcvd) {
	e.Lock()
	defer e.Unlock()

	for evt, kindMap := range e.evKindMap {
		expKindMap, _ := exp.evKindMap[evt]
		for kind, cnt := range kindMap {
			expKindCnt, ok := expKindMap[kind]
			if !ok {
				log.Infof("Event : %v Kind : %v Exp : %v Actual : %v", evt, kind, expKindCnt, cnt)

			} else if expKindCnt != cnt {
				log.Infof("Event : %v Kind : %v Exp : %v Actual : %v", evt, kind, expKindCnt, cnt)
			}
		}
		//Check for kinds not present but where expected
		for kind, cnt := range expKindMap {
			otherKindCnt, ok := kindMap[kind]
			if !ok {
				log.Infof("Event : %v Kind : %v Exp : %v Actual : %v", evt, kind, cnt, otherKindCnt)
				log.Infof("MAP : %v ", e.evKindMap)
			}
		}
	}
}

func (ctx *testObjCtx) WorkFunc(context context.Context) error {
	evt := ctx.event
	kind := ctx.obj.GetKind()

	evtRcvd.Update(evt, kind)
	ctx.obj.ct.resolveObject(ctx.event, ctx)
	return nil
}

var (
	testObjStore map[string][]*testObj
)

func (obj *testObj) References(tenant string, path string, resp map[string]apiintf.ReferenceObj) {
	for key, value := range obj.depMap {
		value := value
		resp[key] = *value
	}
}

func (obj *testObj) Referrers(tenant string, path string, resp map[string]apiintf.ReferenceObj) {
	for key, value := range obj.revDepMap {
		value := value
		resp[key] = *value
	}
}

func getObjectName(kind string, i int) string {
	return "test-" + kind + "-" + strconv.Itoa(i)
}

func generateObjects(kind string, numOfObjects int) []*testObj {
	objs := []*testObj{}
	for i := 0; i < numOfObjects; i++ {
		obj := &testObj{
			TypeMeta: api.TypeMeta{Kind: kind},
			ObjectMeta: api.ObjectMeta{
				Tenant: "tenant",
				Name:   getObjectName(kind, i),
			},
			depMap:    make(map[string]*apiintf.ReferenceObj),
			revDepMap: make(map[string]*apiintf.ReferenceObj),
			ct:        ctrlerInst,
		}

		objs = append(objs, obj)
	}
	return objs
}

type reference struct {
	kind            string
	numberOfObjects int
}
type objReference struct {
	obj  testObj
	refs []reference
}

type relation struct {
	sKind string
	dKind string
	start int
	end   int
}

func initObjectStore() {
	testObjStore = make(map[string][]*testObj)

	aObjects := generateObjects("a", 100)
	testObjStore["a"] = aObjects
	bObjects := generateObjects("b", 100)
	testObjStore["b"] = bObjects
	cObjects := generateObjects("c", 100)
	testObjStore["c"] = cObjects
	dObjects := generateObjects("d", 100)
	testObjStore["d"] = dObjects
}

type doOperation func(key string, obj apiintf.CtkitObject, refs map[string]apiintf.ReferenceObj) error

func sendObjects(evt kvstore.WatchEventType, kind string, start, end int) error {
	// add an object
	if start == 0 && end == 0 {
		end = len(testObjStore[kind])
	}
	objs := testObjStore[kind][start:end]
	errs := make(chan error, len(objs))
	for _, obj := range objs {
		obj := obj
		go func() {
			var err error
			refs := make(map[string]apiintf.ReferenceObj)
			obj.References(obj.GetObjectMeta().GetTenant(), "", refs)
			ctx := &testObjCtx{event: evt, obj: obj}
			switch evt {
			case kvstore.Created:
				err = ctrlerInst.objResolver.ProcessAdd(ctx)
			case kvstore.Updated:
				err = ctrlerInst.objResolver.ProcessUpdate(ctx)
			case kvstore.Deleted:
				err = ctrlerInst.objResolver.ProcessDelete(ctx)
			}
			errs <- err
		}()
	}

	for ii := 0; ii < len(objs); ii++ {
		err := <-errs
		if err != nil {
			return err
		}
	}
	return nil
}

func addReferences(obj *testObj, refs []*testObj) {
	for _, ref := range refs {
		if _, ok := obj.depMap[ref.TypeMeta.Kind]; !ok {
			obj.depMap[ref.TypeMeta.Kind] = &apiintf.ReferenceObj{}
		}
		entry, _ := obj.depMap[ref.TypeMeta.Kind]
		entry.Refs = append(entry.Refs, ref.GetObjectMeta().GetKey())
		entry.RefKind = ref.TypeMeta.Kind

		if _, ok := ref.revDepMap[obj.TypeMeta.Kind]; !ok {
			ref.revDepMap[obj.TypeMeta.Kind] = &apiintf.ReferenceObj{}
		}
		entry, _ = ref.revDepMap[obj.TypeMeta.Kind]
		entry.Refs = append(entry.Refs, obj.GetObjectMeta().GetKey())
		entry.RefKind = ref.TypeMeta.Kind
	}
}

func generateObjectReferences(relations []relation) {

	for _, rel := range relations {
		sObjects := testObjStore[rel.sKind]
		dObjects := testObjStore[rel.dKind]
		for _, sobject := range sObjects {
			addReferences(sobject, dObjects[rel.start:rel.end])
		}
	}
}

func verifyObjects(t *testing.T, evtExp *eventsRcvd,
	duration time.Duration) error {

	timedOutEvent := time.After(duration)
	for true {
		select {
		case <-timedOutEvent:
			evtRcvd.Diff(evtExp)
			return errors.New("Event expectation failed")
		default:
			if evtRcvd.Equal(evtExp) {
				evtRcvd.Diff(evtExp)
				return nil
			}
			time.Sleep(50 * time.Millisecond)

		}
	}
	return nil
}

func TestObjResolverDepAddTest_1(t *testing.T) {
	initObjectStore()

	var evtsExp eventsRcvd
	evtsExp.Reset()
	evtRcvd.Reset()
	ctrlerInst.Reset()

	maxObj := 100
	relations := []relation{
		{"a", "b", 0, maxObj},
	}

	generateObjectReferences(relations)

	err := sendObjects(kvstore.Created, "a", 0, maxObj)
	AssertOk(t, err, "Error creating object")

	//Make sure we receive no objects
	err = verifyObjects(t, &evtsExp, time.Duration(100*time.Millisecond))
	Assert(t, err == nil, "Error verifying objects")

	err = sendObjects(kvstore.Created, "b", 0, maxObj)
	AssertOk(t, err, "Error creating object")

	//Make sure we receive objects of b as it has no deps
	evtsExp.evKindMap[kvstore.Created]["a"] = maxObj
	evtsExp.evKindMap[kvstore.Created]["b"] = maxObj
	err = verifyObjects(t, &evtsExp, time.Duration(100*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

}

func TestObjResolverDepAddTest_2(t *testing.T) {
	initObjectStore()

	var evtsExp eventsRcvd
	evtsExp.Reset()
	evtRcvd.Reset()
	ctrlerInst.Reset()

	maxObj := 100
	relations := []relation{
		{"a", "b", 0, maxObj},
		{"a", "c", 0, maxObj},
	}

	generateObjectReferences(relations)

	err := sendObjects(kvstore.Created, "a", 0, maxObj)
	AssertOk(t, err, "Error creating object")

	//Make sure we receive no objects
	err = verifyObjects(t, &evtsExp, time.Duration(100*time.Millisecond))
	Assert(t, err == nil, "Error verifying objects")

	err = sendObjects(kvstore.Created, "b", 0, maxObj)
	AssertOk(t, err, "Error creating object")

	evtsExp.evKindMap[kvstore.Created]["b"] = maxObj
	err = verifyObjects(t, &evtsExp, time.Duration(100*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(kvstore.Created, "c", 0, maxObj)
	AssertOk(t, err, "Error creating object")

	evtsExp.evKindMap[kvstore.Created]["a"] = maxObj
	evtsExp.evKindMap[kvstore.Created]["c"] = maxObj
	err = verifyObjects(t, &evtsExp, time.Duration(100*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

}

func TestObjResolverDepAddTest_3(t *testing.T) {
	initObjectStore()

	var evtsExp eventsRcvd
	evtsExp.Reset()
	evtRcvd.Reset()
	ctrlerInst.Reset()

	maxObj := 100
	relations := []relation{
		{"a", "b", 0, maxObj},
		{"b", "c", 0, maxObj},
		{"c", "d", 0, maxObj},
	}

	generateObjectReferences(relations)

	err := sendObjects(kvstore.Created, "a", 0, maxObj)
	AssertOk(t, err, "Error creating object")

	//Make sure we receive no objects
	err = verifyObjects(t, &evtsExp, time.Duration(100*time.Millisecond))
	Assert(t, err == nil, "Error verifying objects")

	err = sendObjects(kvstore.Created, "b", 0, maxObj)
	AssertOk(t, err, "Error creating object")

	err = verifyObjects(t, &evtsExp, time.Duration(100*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(kvstore.Created, "c", 0, maxObj)
	AssertOk(t, err, "Error creating object")
	err = verifyObjects(t, &evtsExp, time.Duration(100*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(kvstore.Created, "d", 0, maxObj)
	AssertOk(t, err, "Error creating object")
	evtsExp.evKindMap[kvstore.Created]["a"] = maxObj
	evtsExp.evKindMap[kvstore.Created]["b"] = maxObj
	evtsExp.evKindMap[kvstore.Created]["d"] = maxObj
	evtsExp.evKindMap[kvstore.Created]["c"] = maxObj
	err = verifyObjects(t, &evtsExp, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

}

func TestObjResolverDepAddTest_4(t *testing.T) {
	initObjectStore()

	var evtsExp eventsRcvd
	evtsExp.Reset()
	evtRcvd.Reset()
	ctrlerInst.Reset()

	maxObj := 100
	relations := []relation{
		{"a", "b", 0, maxObj},
		{"b", "c", 0, maxObj},
	}

	generateObjectReferences(relations)

	err := sendObjects(kvstore.Created, "a", 0, maxObj)
	AssertOk(t, err, "Error creating object")

	//Make sure we receive no objects
	err = verifyObjects(t, &evtsExp, time.Duration(100*time.Millisecond))
	Assert(t, err == nil, "Error verifying objects")

	err = sendObjects(kvstore.Created, "b", 0, maxObj)
	AssertOk(t, err, "Error creating object")

	err = verifyObjects(t, &evtsExp, time.Duration(100*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(kvstore.Created, "c", 0, 50)
	AssertOk(t, err, "Error creating object")
	evtsExp.evKindMap[kvstore.Created]["c"] = 50
	err = verifyObjects(t, &evtsExp, time.Duration(100*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(kvstore.Created, "c", 50, 99)
	AssertOk(t, err, "Error creating object")
	evtsExp.evKindMap[kvstore.Created]["c"] = 99
	err = verifyObjects(t, &evtsExp, time.Duration(100*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(kvstore.Created, "c", 99, 100)
	AssertOk(t, err, "Error creating object")
	evtsExp.evKindMap[kvstore.Created]["a"] = maxObj
	evtsExp.evKindMap[kvstore.Created]["b"] = maxObj
	evtsExp.evKindMap[kvstore.Created]["c"] = maxObj
	err = verifyObjects(t, &evtsExp, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

}

func TestObjResolverDepAddTest_5(t *testing.T) {
	initObjectStore()

	var evtsExp eventsRcvd
	evtsExp.Reset()
	evtRcvd.Reset()
	ctrlerInst.Reset()

	maxObj := 100
	relations := []relation{
		{"a", "b", 0, maxObj},
		{"b", "c", 0, maxObj},
		{"c", "d", 0, maxObj},
	}

	generateObjectReferences(relations)

	err := sendObjects(kvstore.Created, "a", 0, maxObj)
	AssertOk(t, err, "Error creating object")

	//Make sure we receive no objects
	err = verifyObjects(t, &evtsExp, time.Duration(100*time.Millisecond))
	Assert(t, err == nil, "Error verifying objects")

	err = sendObjects(kvstore.Created, "d", 0, maxObj)
	AssertOk(t, err, "Error creating object")

	evtsExp.evKindMap[kvstore.Created]["d"] = maxObj
	err = verifyObjects(t, &evtsExp, time.Duration(100*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(kvstore.Created, "c", 0, maxObj)
	AssertOk(t, err, "Error creating object")

	evtsExp.evKindMap[kvstore.Created]["d"] = maxObj
	evtsExp.evKindMap[kvstore.Created]["c"] = maxObj
	err = verifyObjects(t, &evtsExp, time.Duration(100*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(kvstore.Created, "b", 0, maxObj)
	AssertOk(t, err, "Error creating object")

	evtsExp.evKindMap[kvstore.Created]["a"] = maxObj
	evtsExp.evKindMap[kvstore.Created]["b"] = maxObj
	evtsExp.evKindMap[kvstore.Created]["c"] = maxObj
	evtsExp.evKindMap[kvstore.Created]["d"] = maxObj
	err = verifyObjects(t, &evtsExp, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

}

func TestObjResolverDepDelTest_1(t *testing.T) {
	initObjectStore()

	var evtsExp eventsRcvd
	evtsExp.Reset()
	evtRcvd.Reset()
	ctrlerInst.Reset()

	maxObj := 100
	relations := []relation{
		{"a", "b", 0, maxObj},
	}

	generateObjectReferences(relations)

	err := sendObjects(kvstore.Created, "a", 0, maxObj)
	AssertOk(t, err, "Error creating object")

	//Make sure we receive no objects
	err = verifyObjects(t, &evtsExp, time.Duration(1*time.Millisecond))
	Assert(t, err == nil, "Error verifying objects")

	err = sendObjects(kvstore.Created, "b", 0, maxObj)
	AssertOk(t, err, "Error creating object")

	//Make sure we receive objects of b as it has no deps
	evtsExp.evKindMap[kvstore.Created]["a"] = maxObj
	evtsExp.evKindMap[kvstore.Created]["b"] = maxObj
	err = verifyObjects(t, &evtsExp, time.Duration(100*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(kvstore.Deleted, "b", 0, maxObj)
	AssertOk(t, err, "Error creating object")
	err = verifyObjects(t, &evtsExp, time.Duration(100*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	//Make sure we receive objects of b as it has no deps
	err = sendObjects(kvstore.Deleted, "a", 0, maxObj)
	AssertOk(t, err, "Error creating object")
	evtsExp.evKindMap[kvstore.Created]["a"] = maxObj
	evtsExp.evKindMap[kvstore.Created]["b"] = maxObj
	evtsExp.evKindMap[kvstore.Deleted]["a"] = maxObj
	evtsExp.evKindMap[kvstore.Deleted]["b"] = maxObj
	err = verifyObjects(t, &evtsExp, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

}

func TestObjResolverDepDelTest_3(t *testing.T) {
	initObjectStore()

	var evtsExp eventsRcvd
	evtsExp.Reset()
	evtRcvd.Reset()
	ctrlerInst.Reset()

	maxObj := 100
	relations := []relation{
		{"a", "b", 0, maxObj},
		{"b", "c", 0, maxObj},
		{"c", "d", 0, maxObj},
	}

	generateObjectReferences(relations)

	err := sendObjects(kvstore.Created, "a", 0, maxObj)
	err = sendObjects(kvstore.Created, "b", 0, maxObj)
	err = sendObjects(kvstore.Created, "c", 0, maxObj)
	err = sendObjects(kvstore.Created, "d", 0, maxObj)
	AssertOk(t, err, "Error creating object")
	evtsExp.evKindMap[kvstore.Created]["a"] = maxObj
	evtsExp.evKindMap[kvstore.Created]["b"] = maxObj
	evtsExp.evKindMap[kvstore.Created]["c"] = maxObj
	evtsExp.evKindMap[kvstore.Created]["d"] = maxObj
	err = verifyObjects(t, &evtsExp, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(kvstore.Deleted, "b", 0, maxObj)
	AssertOk(t, err, "Error creating object")
	err = verifyObjects(t, &evtsExp, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(kvstore.Deleted, "c", 0, maxObj)
	AssertOk(t, err, "Error creating object")
	err = verifyObjects(t, &evtsExp, time.Duration(100*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(kvstore.Deleted, "d", 0, maxObj)
	AssertOk(t, err, "Error creating object")
	err = verifyObjects(t, &evtsExp, time.Duration(100*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(kvstore.Deleted, "a", 0, maxObj)
	AssertOk(t, err, "Error creating object")

	evtsExp.evKindMap[kvstore.Deleted]["a"] = maxObj
	evtsExp.evKindMap[kvstore.Deleted]["b"] = maxObj
	evtsExp.evKindMap[kvstore.Deleted]["c"] = maxObj
	evtsExp.evKindMap[kvstore.Deleted]["d"] = maxObj
	err = verifyObjects(t, &evtsExp, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

}

func TestObjResolverDepAddDelTest_1(t *testing.T) {
	initObjectStore()

	var evtsExp eventsRcvd
	evtsExp.Reset()
	evtRcvd.Reset()
	ctrlerInst.Reset()

	maxObj := 100
	relations := []relation{
		{"a", "b", 0, maxObj},
	}

	generateObjectReferences(relations)

	err := sendObjects(kvstore.Created, "a", 0, maxObj)
	AssertOk(t, err, "Error creating object")

	//Make sure we receive no objects
	err = verifyObjects(t, &evtsExp, time.Duration(1*time.Millisecond))
	Assert(t, err == nil, "Error verifying objects")

	err = sendObjects(kvstore.Created, "b", 0, maxObj)
	AssertOk(t, err, "Error creating object")

	//Make sure we receive objects of b as it has no deps
	evtsExp.evKindMap[kvstore.Created]["a"] = maxObj
	evtsExp.evKindMap[kvstore.Created]["b"] = maxObj
	err = verifyObjects(t, &evtsExp, time.Duration(100*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(kvstore.Deleted, "b", 0, maxObj)
	AssertOk(t, err, "Error creating object")
	err = verifyObjects(t, &evtsExp, time.Duration(100*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(kvstore.Created, "b", 0, maxObj)
	AssertOk(t, err, "Error creating object")
	err = verifyObjects(t, &evtsExp, time.Duration(100*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	//Make sure we receive objects of b as it has no deps
	err = sendObjects(kvstore.Deleted, "a", 0, maxObj)
	AssertOk(t, err, "Error creating object")

	evtsExp.evKindMap[kvstore.Created]["a"] = maxObj
	evtsExp.evKindMap[kvstore.Created]["b"] = maxObj * 2
	evtsExp.evKindMap[kvstore.Deleted]["a"] = maxObj
	evtsExp.evKindMap[kvstore.Deleted]["b"] = maxObj
	err = verifyObjects(t, &evtsExp, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

}

func TestObjResolverDepAddDelTest_2(t *testing.T) {
	initObjectStore()

	var evtsExp eventsRcvd
	evtsExp.Reset()
	evtRcvd.Reset()
	ctrlerInst.Reset()

	maxObj := 100
	relations := []relation{
		{"a", "b", 0, maxObj},
		{"b", "c", 0, maxObj},
	}

	generateObjectReferences(relations)

	err := sendObjects(kvstore.Created, "a", 0, maxObj)
	AssertOk(t, err, "Error creating object")

	//Make sure we receive no objects
	err = verifyObjects(t, &evtsExp, time.Duration(1*time.Millisecond))
	Assert(t, err == nil, "Error verifying objects")

	err = sendObjects(kvstore.Created, "b", 0, maxObj)
	AssertOk(t, err, "Error creating object")

	err = sendObjects(kvstore.Created, "c", 0, maxObj)
	AssertOk(t, err, "Error creating object")

	//Make sure we receive objects of b as it has no deps
	evtsExp.evKindMap[kvstore.Created]["a"] = maxObj
	evtsExp.evKindMap[kvstore.Created]["b"] = maxObj
	evtsExp.evKindMap[kvstore.Created]["c"] = maxObj
	err = verifyObjects(t, &evtsExp, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(kvstore.Deleted, "b", 0, maxObj)
	AssertOk(t, err, "Error creating object")
	err = verifyObjects(t, &evtsExp, time.Duration(100*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(kvstore.Created, "b", 0, maxObj)
	AssertOk(t, err, "Error creating object")
	err = verifyObjects(t, &evtsExp, time.Duration(100*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(kvstore.Deleted, "c", 0, maxObj)
	AssertOk(t, err, "Error creating object")
	err = verifyObjects(t, &evtsExp, time.Duration(100*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(kvstore.Created, "c", 0, maxObj)
	AssertOk(t, err, "Error creating object")
	err = verifyObjects(t, &evtsExp, time.Duration(100*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	//Make sure we receive objects of b as it has no deps
	err = sendObjects(kvstore.Deleted, "a", 0, maxObj)
	AssertOk(t, err, "Error creating object")

	evtsExp.evKindMap[kvstore.Created]["a"] = maxObj
	evtsExp.evKindMap[kvstore.Created]["b"] = maxObj * 2
	evtsExp.evKindMap[kvstore.Created]["c"] = maxObj * 2
	evtsExp.evKindMap[kvstore.Deleted]["a"] = maxObj
	evtsExp.evKindMap[kvstore.Deleted]["b"] = maxObj
	evtsExp.evKindMap[kvstore.Deleted]["c"] = maxObj
	err = verifyObjects(t, &evtsExp, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

}

func TestObjResolverDepAddDelTest_3(t *testing.T) {
	initObjectStore()

	var evtsExp eventsRcvd
	evtsExp.Reset()
	evtRcvd.Reset()
	ctrlerInst.Reset()

	maxObj := 100
	relations := []relation{
		{"a", "b", 0, maxObj},
		{"b", "c", 0, maxObj},
		{"c", "d", 0, maxObj},
	}

	generateObjectReferences(relations)

	err := sendObjects(kvstore.Created, "a", 0, maxObj)
	AssertOk(t, err, "Error creating object")

	//Make sure we receive no objects
	err = verifyObjects(t, &evtsExp, time.Duration(1*time.Millisecond))
	Assert(t, err == nil, "Error verifying objects")

	err = sendObjects(kvstore.Created, "b", 0, maxObj)
	AssertOk(t, err, "Error creating object")

	err = sendObjects(kvstore.Created, "c", 0, maxObj)
	AssertOk(t, err, "Error creating object")

	err = sendObjects(kvstore.Created, "d", 0, maxObj)
	AssertOk(t, err, "Error creating object")

	//Make sure we receive objects of b as it has no deps
	evtsExp.evKindMap[kvstore.Created]["a"] = maxObj
	evtsExp.evKindMap[kvstore.Created]["b"] = maxObj
	evtsExp.evKindMap[kvstore.Created]["c"] = maxObj
	evtsExp.evKindMap[kvstore.Created]["d"] = maxObj
	err = verifyObjects(t, &evtsExp, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(kvstore.Deleted, "b", 0, maxObj)
	AssertOk(t, err, "Error creating object")
	err = verifyObjects(t, &evtsExp, time.Duration(100*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(kvstore.Deleted, "c", 0, maxObj)
	AssertOk(t, err, "Error creating object")
	err = verifyObjects(t, &evtsExp, time.Duration(100*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(kvstore.Deleted, "d", 0, maxObj)
	AssertOk(t, err, "Error creating object")
	err = verifyObjects(t, &evtsExp, time.Duration(100*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(kvstore.Created, "b", 0, maxObj)
	AssertOk(t, err, "Error creating object")
	err = verifyObjects(t, &evtsExp, time.Duration(100*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(kvstore.Created, "c", 0, maxObj)
	AssertOk(t, err, "Error creating object")
	err = verifyObjects(t, &evtsExp, time.Duration(100*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(kvstore.Created, "d", 0, maxObj)
	AssertOk(t, err, "Error creating object")
	err = verifyObjects(t, &evtsExp, time.Duration(100*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	//Make sure we receive objects of b as it has no deps
	err = sendObjects(kvstore.Deleted, "a", 0, maxObj)
	AssertOk(t, err, "Error creating object")

	evtsExp.evKindMap[kvstore.Created]["a"] = maxObj
	evtsExp.evKindMap[kvstore.Created]["b"] = maxObj * 2
	evtsExp.evKindMap[kvstore.Created]["c"] = maxObj * 2
	evtsExp.evKindMap[kvstore.Created]["d"] = maxObj * 2
	evtsExp.evKindMap[kvstore.Deleted]["a"] = maxObj
	evtsExp.evKindMap[kvstore.Deleted]["b"] = maxObj
	evtsExp.evKindMap[kvstore.Deleted]["c"] = maxObj
	evtsExp.evKindMap[kvstore.Deleted]["d"] = maxObj
	err = verifyObjects(t, &evtsExp, time.Duration(1*time.Second))
	AssertOk(t, err, "Error verifying objects")

}

func TestMemdbAddUpdateTest_1(t *testing.T) {
	initObjectStore()

	var evtsExp eventsRcvd
	evtsExp.Reset()
	evtRcvd.Reset()
	ctrlerInst.Reset()

	maxObj := 100
	relations := []relation{
		{"a", "b", 0, maxObj},
		{"b", "c", 0, maxObj / 2},
	}

	generateObjectReferences(relations)

	err := sendObjects(kvstore.Created, "a", 0, maxObj)
	AssertOk(t, err, "Error creating object")

	//Make sure we receive no objects
	err = verifyObjects(t, &evtsExp, time.Duration(1*time.Millisecond))
	Assert(t, err == nil, "Error verifying objects")

	err = sendObjects(kvstore.Created, "b", 0, maxObj)
	AssertOk(t, err, "Error creating object")

	//Make sure no objects recvd
	err = verifyObjects(t, &evtsExp, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(kvstore.Created, "c", maxObj/2, maxObj)
	AssertOk(t, err, "Error creating object")

	//Make sure we receive objects of b as it has no deps
	//evtsExp.evKindMap[kvstore.Created]["a"] = maxObj
	//evtsExp.evKindMap[kvstore.Created]["b"] = maxObj
	evtsExp.evKindMap[kvstore.Created]["c"] = maxObj / 2
	err = verifyObjects(t, &evtsExp, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	initObjectStore()

	relations = []relation{
		{"a", "b", 0, maxObj},
		{"b", "c", maxObj / 2, maxObj},
	}

	generateObjectReferences(relations)

	err = sendObjects(kvstore.Created, "b", 0, maxObj)
	AssertOk(t, err, "Error creating object")

	evtsExp.evKindMap[kvstore.Created]["a"] = maxObj
	evtsExp.evKindMap[kvstore.Created]["b"] = maxObj
	evtsExp.evKindMap[kvstore.Created]["c"] = maxObj / 2
	err = verifyObjects(t, &evtsExp, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

}

func TestMemdbAddUpdateTest_2(t *testing.T) {
	initObjectStore()

	var evtsExp eventsRcvd
	evtsExp.Reset()
	evtRcvd.Reset()
	ctrlerInst.Reset()

	maxObj := 100
	relations := []relation{
		{"a", "b", 0, maxObj},
		{"b", "c", 0, maxObj / 2},
	}

	generateObjectReferences(relations)

	err := sendObjects(kvstore.Created, "a", 0, maxObj)
	AssertOk(t, err, "Error creating object")

	//Make sure we receive no objects
	err = verifyObjects(t, &evtsExp, time.Duration(1*time.Millisecond))
	Assert(t, err == nil, "Error verifying objects")

	err = sendObjects(kvstore.Created, "b", 0, maxObj)
	AssertOk(t, err, "Error creating object")

	//Make sure no objects recvd
	err = verifyObjects(t, &evtsExp, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(kvstore.Created, "c", maxObj/2, maxObj)
	AssertOk(t, err, "Error creating object")

	//Make sure we receive objects of b as it has no deps
	//evtsExp.evKindMap[kvstore.Created]["a"] = maxObj
	//evtsExp.evKindMap[kvstore.Created]["b"] = maxObj
	evtsExp.evKindMap[kvstore.Created]["c"] = maxObj / 2
	err = verifyObjects(t, &evtsExp, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	initObjectStore()

	relations = []relation{
		{"a", "b", 0, maxObj},
		{"b", "c", maxObj / 2, maxObj},
	}

	generateObjectReferences(relations)

	err = sendObjects(kvstore.Created, "b", 0, maxObj)
	AssertOk(t, err, "Error creating object")

	evtsExp.evKindMap[kvstore.Created]["a"] = maxObj
	evtsExp.evKindMap[kvstore.Created]["b"] = maxObj
	evtsExp.evKindMap[kvstore.Created]["c"] = maxObj / 2
	err = verifyObjects(t, &evtsExp, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

}

func TestMemdbAddUpdateTest_3(t *testing.T) {
	initObjectStore()

	var evtsExp eventsRcvd
	evtsExp.Reset()
	evtRcvd.Reset()
	ctrlerInst.Reset()

	maxObj := 100
	relations := []relation{
		{"a", "b", 0, maxObj},
		{"b", "c", 0, maxObj / 2},
	}

	generateObjectReferences(relations)

	err := sendObjects(kvstore.Created, "a", 0, maxObj)
	AssertOk(t, err, "Error creating object")

	//Make sure we receive no objects
	err = verifyObjects(t, &evtsExp, time.Duration(1*time.Millisecond))
	Assert(t, err == nil, "Error verifying objects")

	err = sendObjects(kvstore.Created, "b", 0, maxObj)
	AssertOk(t, err, "Error creating object")

	//Make sure no objects recvd
	err = verifyObjects(t, &evtsExp, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(kvstore.Created, "c", 0, maxObj/2)
	AssertOk(t, err, "Error creating object")

	//Make sure we receive objects of b as it has no deps
	evtsExp.evKindMap[kvstore.Created]["a"] = maxObj
	evtsExp.evKindMap[kvstore.Created]["b"] = maxObj
	evtsExp.evKindMap[kvstore.Created]["c"] = maxObj / 2
	err = verifyObjects(t, &evtsExp, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	initObjectStore()

	relations = []relation{
		{"a", "b", 0, maxObj},
		{"b", "c", 0, maxObj},
	}

	generateObjectReferences(relations)

	err = sendObjects(kvstore.Updated, "b", 0, maxObj)
	AssertOk(t, err, "Error creating object")
	err = verifyObjects(t, &evtsExp, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(kvstore.Created, "c", maxObj/2, maxObj)
	AssertOk(t, err, "Error creating object")

	evtsExp.evKindMap[kvstore.Created]["a"] = maxObj
	evtsExp.evKindMap[kvstore.Created]["b"] = maxObj
	evtsExp.evKindMap[kvstore.Updated]["b"] = maxObj
	evtsExp.evKindMap[kvstore.Created]["c"] = maxObj
	err = verifyObjects(t, &evtsExp, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

}

func TestMemdbAddUpdateTest_4(t *testing.T) {
	initObjectStore()

	var evtsExp eventsRcvd
	evtsExp.Reset()
	evtRcvd.Reset()
	ctrlerInst.Reset()

	maxObj := 100
	relations := []relation{
		{"a", "b", 0, maxObj},
		{"b", "c", 0, maxObj / 2},
	}

	generateObjectReferences(relations)

	err := sendObjects(kvstore.Created, "a", 0, maxObj)
	AssertOk(t, err, "Error creating object")

	//Make sure we receive no objects
	err = verifyObjects(t, &evtsExp, time.Duration(1*time.Millisecond))
	Assert(t, err == nil, "Error verifying objects")

	err = sendObjects(kvstore.Created, "b", 0, maxObj)
	AssertOk(t, err, "Error creating object")

	//Make sure no objects recvd
	err = verifyObjects(t, &evtsExp, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(kvstore.Created, "c", 0, maxObj/2)
	AssertOk(t, err, "Error creating object")

	//Make sure we receive objects of b as it has no deps
	evtsExp.evKindMap[kvstore.Created]["a"] = maxObj
	evtsExp.evKindMap[kvstore.Created]["b"] = maxObj
	evtsExp.evKindMap[kvstore.Created]["c"] = maxObj / 2
	err = verifyObjects(t, &evtsExp, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	initObjectStore()

	relations = []relation{
		{"a", "b", 0, maxObj},
		{"b", "c", 0, maxObj},
	}

	generateObjectReferences(relations)

	err = sendObjects(kvstore.Updated, "b", 0, maxObj)
	AssertOk(t, err, "Error creating object")
	err = verifyObjects(t, &evtsExp, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(kvstore.Deleted, "b", 0, maxObj)
	AssertOk(t, err, "Error creating object")
	err = verifyObjects(t, &evtsExp, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

	err = sendObjects(kvstore.Deleted, "a", 0, maxObj)
	AssertOk(t, err, "Error creating object")

	err = sendObjects(kvstore.Created, "c", maxObj/2, maxObj)
	AssertOk(t, err, "Error creating object")

	evtsExp.evKindMap[kvstore.Created]["a"] = maxObj
	evtsExp.evKindMap[kvstore.Created]["b"] = maxObj
	evtsExp.evKindMap[kvstore.Updated]["b"] = maxObj
	evtsExp.evKindMap[kvstore.Created]["c"] = maxObj
	evtsExp.evKindMap[kvstore.Deleted]["a"] = maxObj
	evtsExp.evKindMap[kvstore.Deleted]["b"] = maxObj
	err = verifyObjects(t, &evtsExp, time.Duration(500*time.Millisecond))
	AssertOk(t, err, "Error verifying objects")

}
