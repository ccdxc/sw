package apisrvpkg

import (
	"context"
	"testing"

	oldcontext "golang.org/x/net/context"
	"google.golang.org/grpc/metadata"

	"github.com/pensando/sw/api"
	apisrv "github.com/pensando/sw/venice/apiserver"
	mocks "github.com/pensando/sw/venice/apiserver/pkg/mocks"
	"github.com/pensando/sw/venice/utils/kvstore"
)

// TestPrepareMessage
// Tests the version transform functionality of the message.
// Tests both when there is a valid transformation and when the
// transformation is not registered.
func TestPrepareMessage(t *testing.T) {
	f := mocks.NewFakeMessage("/test", true).(*mocks.FakeMessage)
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
	f := mocks.NewFakeMessage("/test", true).(*mocks.FakeMessage)
	m := NewMessage("TestType1").WithValidate(f.ValidateFunc).WithDefaulter(f.DefaultFunc)
	m = m.WithKvUpdater(f.KvUpdateFunc).WithKvGetter(f.KvGetFunc).WithKvDelFunc(f.KvDelFunc).WithObjectVersionWriter(f.ObjverwriteFunc)
	m = m.WithKvTxnUpdater(f.TxnUpdateFunc).WithKvTxnDelFunc(f.DelFromKvTxnFunc).WithSelfLinkWriter(f.SelfLinkWriterFunc)
	m = m.WithKvWatchFunc(f.KvwatchFunc).WithKvListFunc(f.KvListFunc)
	m = m.WithUUIDWriter(f.CreateUUID)
	singletonAPISrv.runstate.running = true
	m.Validate(nil, "", true)
	var kv kvstore.Interface
	if f.ValidateCalled != 1 {
		t.Errorf("Expecting 1 validation found %d", f.ValidateCalled)
	}
	m.Default(nil)
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

	m.WriteToKvTxn(context.TODO(), nil, nil, "testprefix", true)
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

	m.UpdateSelfLink("", nil)
	if f.SelfLinkWrites != 1 {
		t.Errorf("Expecgting 1 call to UpdateSelfLink found %d", f.SelfLinkWrites)
	}
	ctx := context.TODO()
	md := metadata.Pairs(apisrv.RequestParamVersion, "v1",
		apisrv.RequestParamMethod, "WATCH")
	ctx = metadata.NewIncomingContext(ctx, md)
	stream := fakeGrpcStream{ctx: ctx}
	opts := api.ListWatchOptions{}
	err := m.WatchFromKv(&opts, stream, "test")
	if err != nil {
		t.Errorf("watch returned error (%s)", err)
	}
}
