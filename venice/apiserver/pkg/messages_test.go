package apisrvpkg

import (
	"context"
	"reflect"
	"testing"

	oldcontext "golang.org/x/net/context"
	"google.golang.org/grpc/metadata"

	"github.com/pensando/sw/api/interfaces"

	"github.com/pensando/sw/api"
	apisrv "github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/apiserver/pkg/mocks"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/runtime"
)

// TestPrepareMessage
// Tests the version transform functionality of the message.
// Tests both when there is a valid transformation and when the
// transformation is not registered.
func TestPrepareMessage(t *testing.T) {
	f := mocks.NewFakeMessage("test.TestType1", "/test", true).(*mocks.FakeMessage)
	m := NewMessage("TestType1").WithTransform("v1", "v2", f.TransformCb)
	msg := mocks.FakeMessage{}
	m.PrepareMsg("v1", "v2", msg)
	if len(f.CalledTxfms) != 1 {
		t.Errorf("Expecting 1 transform found %v", len(f.CalledTxfms))
	}

	if f.CalledTxfms[0] != "v1-v2" {
		t.Errorf("Expecting [v1-v2] transform found %v", f.CalledTxfms[0])
	}

	// Call with unregistered transform
	r, err := m.PrepareMsg("v2", "v1", msg)
	if r != nil || err == nil {
		t.Errorf("Expecting transform to fail but it did not")
	}
}

type fakeGrpcStream struct {
	ctx oldcontext.Context
}

func (s fakeGrpcStream) Context() oldcontext.Context {
	return s.ctx
}
func (s fakeGrpcStream) SendMsg(in interface{}) error {
	return nil
}
func (s fakeGrpcStream) RecvMsg(in interface{}) error {
	return nil
}
func (s fakeGrpcStream) SetHeader(_ metadata.MD) error {
	return nil
}
func (s fakeGrpcStream) SendHeader(_ metadata.MD) error {
	return nil
}
func (s fakeGrpcStream) SetTrailer(_ metadata.MD) {
}

// TestMessageWith
// Tests the various Hooks for the message
func TestMessageWith(t *testing.T) {
	MustGetAPIServer()
	f := mocks.NewFakeMessage("test.TestType1", "/test", true).(*mocks.FakeMessage)
	m := NewMessage("TestType1").WithValidate(f.ValidateFunc).WithNormalizer(f.NormalizerFunc)
	m = m.WithKvUpdater(f.KvUpdateFunc).WithKvGetter(f.KvGetFunc).WithKvDelFunc(f.KvDelFunc).WithObjectVersionWriter(f.ObjverwriteFunc)
	m = m.WithKvTxnUpdater(f.TxnUpdateFunc).WithKvTxnDelFunc(f.DelFromKvTxnFunc).WithSelfLinkWriter(f.SelfLinkWriterFunc)
	m = m.WithKvWatchFunc(f.KvwatchFunc).WithKvListFunc(f.KvListFunc).WithReplaceStatusFunction(f.GetUpdateStatusFunc())
	m = m.WithUUIDWriter(f.CreateUUID).WithReplaceSpecFunction(f.GetUpdateSpecFunc()).WithGetRuntimeObject(f.GetRuntimeObject)
	m = m.WithModTimeWriter(f.WriteModTime).WithCreationTimeWriter(f.WriteCreationTime).WithObjectVersionWriter(f.WriteObjVersion)
	m = m.WithReferencesGetter(f.GetReferencesFunc).WithKeyGenerator(f.KeyGeneratorFunc).WithUpdateMetaFunction(f.GetUpdateMetaFunc())
	stx := mocks.ObjStorageTransformer{}
	m = m.WithStorageTransformer(&stx)
	singletonAPISrv.runstate.running = true
	m.Validate(nil, "", true, false)
	var kv kvstore.Interface
	if f.ValidateCalled != 1 {
		t.Errorf("Expecting 1 validation found %d", f.ValidateCalled)
	}
	m.Normalize(nil)
	if f.DefaultCalled != 1 {
		t.Errorf("Expecting 1 call to Defaulter function found %d", f.DefaultCalled)
	}

	m.GetFromKv(context.TODO(), kv, "testkey")
	if f.Kvreads != 1 {
		t.Errorf("Expecting 1 call to KV read found %d", f.Kvreads)
	}

	m.WriteToKv(context.TODO(), kv, nil, "testprefix", true, true)
	if f.Kvwrites != 1 {
		t.Errorf("Expecting 1 call to KV Write found %d", f.Kvwrites)
	}

	m.WriteToKvTxn(context.TODO(), nil, nil, nil, "testprefix", true, false)
	if f.Txnwrites != 1 {
		t.Errorf("Expecting 1 call to Txn write found %d", f.Txnwrites)
	}
	m.DelFromKv(context.TODO(), kv, "testKey")
	if f.Kvdels != 1 {
		t.Errorf("Expecting 1 call to KV Del found %d", f.Kvdels)
	}

	m.DelFromKvTxn(context.TODO(), nil, "testKey")
	if f.Txndels != 1 {
		t.Errorf("Expecting 1 call to Txn Del found %d", f.Txndels)
	}
	m.CreateUUID(nil)
	if f.Uuidwrite != 1 {
		t.Errorf("Expecting 1 call to CreateUUID found %d", f.Uuidwrite)
	}

	m.UpdateSelfLink("", "", "", nil)
	if f.SelfLinkWrites != 1 {
		t.Errorf("Expecgting 1 call to UpdateSelfLink found %d", f.SelfLinkWrites)
	}
	ctx := context.TODO()
	fn := m.GetUpdateSpecFunc()
	if fn == nil {
		t.Fatalf("UpdateSpecFunc returned nil")
	}
	fn1 := m.GetUpdateStatusFunc()
	if fn1 == nil {
		t.Fatalf("UpdateStatusFunc returned nil")
	}
	obj := TestType1{}
	f.RuntimeObj = &obj
	robj := m.GetRuntimeObject(obj)
	if _, ok := robj.(runtime.Object); !ok {
		t.Fatalf("failed to get runtime object")
	}
	m.WriteCreationTime(obj)
	if f.CreateTimeWrites != 1 {
		t.Errorf("WriteCreation time failed")
	}
	m.WriteModTime(obj)
	if f.ModTimeWrite != 1 {
		t.Errorf("WriteModTime time failed")
	}
	m.WriteObjVersion(obj, "123")
	if f.ObjVerWrites != 1 {
		t.Errorf("WriteObjVersion time failed got %d", f.Objverwrite)
	}
	m.TransformToStorage(ctx, apiintf.CreateOper, nil)
	if stx.TransformToStorageCalled != 1 {
		t.Errorf("Expecting 1 call to TransformToStorage, found %d", stx.TransformToStorageCalled)
	}
	m.TransformFromStorage(ctx, apiintf.CreateOper, nil)
	if stx.TransformFromStorageCalled != 1 {
		t.Errorf("Expecting 1 call to TransformFromStorage, found %d", stx.TransformFromStorageCalled)
	}
	// Add the same storage transformer a second time. Now each calls increments the counter by 2.
	m.WithStorageTransformer(&stx)
	m.TransformToStorage(ctx, apiintf.UpdateOper, nil)
	if stx.TransformToStorageCalled != 3 {
		t.Errorf("Expecting 3 calls to TransformToStorage, found %d", stx.TransformToStorageCalled)
	}
	m.TransformFromStorage(ctx, apiintf.UpdateOper, nil)
	if stx.TransformFromStorageCalled != 3 {
		t.Errorf("Expecting 3 calls to TransformFromStorage, found %d", stx.TransformFromStorageCalled)
	}
	if m.GetKind() != "TestType1" {
		t.Errorf("Expecting kind %s, found %s", "TestType1", m.GetKind())
	}
	m.ListFromKv(ctx, nil, nil, "")
	if f.Kvlists != 1 {
		t.Errorf("expecting 1 list call got [%v]", f.Kvlists)
	}
	m.GetKVKey(nil, "")
	if f.KeyGens != 1 {
		t.Errorf("expecting 1 call to generate key got [%v]", f.KeyGens)
	}

	if m.GetUpdateMetaFunc() == nil {
		t.Errorf("expecting UpdateMetaFunc to be set")
	}

	md := metadata.Pairs(apisrv.RequestParamVersion, "v1",
		apisrv.RequestParamMethod, "WATCH")
	ctx = metadata.NewIncomingContext(ctx, md)
	stream := fakeGrpcStream{ctx: ctx}
	opts := api.ListWatchOptions{}
	err := m.WatchFromKv(&opts, stream, "test")
	if err != nil {
		t.Errorf("watch returned error (%s)", err)
	}

	if m.GetUpdateMetaFunc() == nil {
		t.Errorf("expecting UpdateMetaFunc to be set")
	}
	refMap := map[string]apiintf.ReferenceObj{
		"/testmsg/path": {RefType: apiintf.NamedReference, Refs: []string{"/test/reference1", "/test/reference2"}},
	}
	f.RefMap = refMap
	refs, err := m.GetReferences(nil)
	if err != nil {
		t.Errorf("failed to get references (%s)", err)
	}
	if !reflect.DeepEqual(refMap, refs) {
		t.Errorf("returned references does not match got\n[%+v\nwant\n[%+v]", refs, refMap)
	}
}
