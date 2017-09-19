package apisrvpkg

import (
	"context"
	"errors"
	"testing"

	oldcontext "golang.org/x/net/context"
	"google.golang.org/grpc"
	"google.golang.org/grpc/metadata"

	"github.com/pensando/sw/api"
	apisrv "github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
)

// fakeMessage is used as a mock object for testing.
type fakeMessage struct {
	calledTxfms    []string
	kvpath         string
	validateRslt   bool
	validateCalled int
	defaultCalled  int
	kvwrites       int
	kvreads        int
	kvdels         int
	kvlists        int
	kvwatch        int
	txnwrites      int
	txngets        int
	txndels        int
	objverwrite    int
}

var txnTestKey = "/txn/testobj"

func (m *fakeMessage) WithTransform(from, to string, fn apisrv.TransformFunc) apisrv.Message { return m }
func (m *fakeMessage) WithValidate(fn apisrv.ValidateFunc) apisrv.Message                    { return m }
func (m *fakeMessage) WithDefaulter(fn apisrv.DefaulterFunc) apisrv.Message                  { return m }
func (m *fakeMessage) WithKeyGenerator(fn apisrv.KeyGenFunc) apisrv.Message                  { return m }
func (m *fakeMessage) WithKvUpdater(fn apisrv.UpdateKvFunc) apisrv.Message                   { return m }
func (m *fakeMessage) WithKvGetter(fn apisrv.GetFromKvFunc) apisrv.Message                   { return m }
func (m *fakeMessage) WithKvDelFunc(fn apisrv.DelFromKvFunc) apisrv.Message                  { return m }
func (m *fakeMessage) WithResponseWriter(fn apisrv.ResponseWriterFunc) apisrv.Message        { return m }
func (m *fakeMessage) WithKvTxnDelFunc(fn apisrv.DelFromKvTxnFunc) apisrv.Message            { return m }
func (m *fakeMessage) WithKvTxnUpdater(fn apisrv.UpdateKvTxnFunc) apisrv.Message             { return m }
func (m *fakeMessage) WithKvListFunc(fn apisrv.ListFromKvFunc) apisrv.Message                { return m }
func (m *fakeMessage) WithKvWatchFunc(fn apisrv.WatchKvFunc) apisrv.Message                  { return m }
func (m *fakeMessage) WithObjectVersionWriter(fn apisrv.SetObjectVersionFunc) apisrv.Message { return m }
func (m *fakeMessage) GetKind() string                                                       { return "" }
func (m *fakeMessage) WriteObjVersion(i interface{}, version string) interface{}             { return i }
func (m *fakeMessage) ListFromKv(ctx context.Context, options *api.ListWatchOptions, prefix string) (interface{}, error) {
	m.kvlists++
	return nil, nil
}
func (m *fakeMessage) WatchFromKv(options *api.ListWatchOptions, stream grpc.ServerStream, prefix string) error {
	m.kvwatch++
	return nil
}
func (m *fakeMessage) DelFromKv(ctx context.Context, key string) (interface{}, error) {
	m.kvdels++
	return nil, nil
}
func (m *fakeMessage) DelFromKvTxn(ctx context.Context, txn kvstore.Txn, key string) error {
	m.txndels++
	txn.Delete(key)
	return nil
}
func (m *fakeMessage) WriteToKvTxn(ctx context.Context, txn kvstore.Txn, i interface{}, prerfix string, create bool) error {
	m.txnwrites++
	msg := i.(*kvstore.TestObj)
	if create {
		txn.Create(txnTestKey, msg)
	} else {
		txn.Update(txnTestKey, msg)
	}
	return nil
}
func (m *fakeMessage) GetKVKey(i interface{}, prefix string) (string, error) {
	return m.kvpath, nil
}
func (m *fakeMessage) WriteToKv(ctx context.Context, i interface{}, prerfix string, create bool) (interface{}, error) {
	m.kvwrites++
	return i, nil
}
func (m *fakeMessage) GetFromKv(ctx context.Context, key string) (interface{}, error) {
	m.kvreads++
	return nil, nil
}
func (m *fakeMessage) PrepareMsg(from, to string, i interface{}) (interface{}, error) {
	m.calledTxfms = append(m.calledTxfms, from+"-"+to)
	return i, nil
}
func (m *fakeMessage) Default(i interface{}) interface{} {
	return i
}
func (m *fakeMessage) Validate(i interface{}) error {
	if m.validateRslt {
		return nil
	}
	return errors.New("Setup to fail validation")
}

func newFakeMessage(kvPath string, validateResult bool) apisrv.Message {
	r := fakeMessage{kvpath: kvPath, validateRslt: validateResult}
	return &r
}

func (m *fakeMessage) transformCb(from, to string, i interface{}) interface{} {
	r, _ := m.PrepareMsg(from, to, i)
	return r
}

func (m *fakeMessage) validateFunc(i interface{}) error {
	m.validateCalled++
	return nil
}

func (m *fakeMessage) defaultFunc(i interface{}) interface{} {
	m.defaultCalled++
	return i
}

func (m *fakeMessage) kvUpdateFunc(ctx context.Context, kv kvstore.Interface, i interface{}, prefix string, create bool) (interface{}, error) {
	m.kvwrites++
	return i, nil
}

func (m *fakeMessage) txnUpdateFunc(ctx context.Context, kvstore kvstore.Txn, i interface{}, prefix string, create bool) error {
	m.txnwrites++
	return nil
}

func (m *fakeMessage) kvGetFunc(ctx context.Context, kv kvstore.Interface, key string) (interface{}, error) {
	m.kvreads++
	return nil, nil
}

func (m *fakeMessage) kvDelFunc(ctx context.Context, kv kvstore.Interface, key string) (interface{}, error) {
	m.kvdels++
	return nil, nil
}

func (m *fakeMessage) delFromKvTxnFunc(ctx context.Context, kvstore kvstore.Txn, key string) error {
	m.txndels++
	return nil
}

func (m *fakeMessage) objVerWrite(i interface{}, version string) interface{} {
	m.objverwrite++
	return i
}

func (m *fakeMessage) kvWatchFunc(l log.Logger, options *api.ListWatchOptions, kvs kvstore.Interface, stream interface{}, txfn func(from, to string, i interface{}) (interface{}, error), version, svcprefix string) error {
	m.kvwatch++
	return nil
}

func (m *fakeMessage) kvListFunc(ctx context.Context, kvs kvstore.Interface, options *api.ListWatchOptions, prefix string) (interface{}, error) {
	m.kvlists++
	return nil, nil
}

// TestPrepareMessage
// Tests the version transform functionality of the message.
// Tests both when there is a valid transformation and when the
// transformation is not registered.
func TestPrepareMessage(t *testing.T) {
	f := newFakeMessage("/test", true).(*fakeMessage)
	m := NewMessage("TestType1").WithTransform("v1", "v2", f.transformCb)
	msg := fakeMessage{}
	m.PrepareMsg("v1", "v2", msg)
	if len(f.calledTxfms) != 1 {
		t.Errorf("Expecting 1 transform found %v", len(f.calledTxfms))
	}

	if f.calledTxfms[0] != "v1-v2" {
		t.Errorf("Expecting [v1-v2] transform found %v", f.calledTxfms[0])
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
	f := newFakeMessage("/test", true).(*fakeMessage)
	m := NewMessage("TestType1").WithValidate(f.validateFunc).WithDefaulter(f.defaultFunc)
	m = m.WithKvUpdater(f.kvUpdateFunc).WithKvGetter(f.kvGetFunc).WithKvDelFunc(f.kvDelFunc).WithObjectVersionWriter(f.objVerWrite)
	m = m.WithKvTxnUpdater(f.txnUpdateFunc).WithKvTxnDelFunc(f.delFromKvTxnFunc)
	m = m.WithKvWatchFunc(f.kvWatchFunc).WithKvListFunc(f.kvListFunc)
	m.Validate(nil)
	if f.validateCalled != 1 {
		t.Errorf("Expecting validation count of %v found", f.validateCalled)
	}
	m.Default(nil)
	if f.defaultCalled != 1 {
		t.Errorf("Expecting 1 call to Defaulter function found %d", f.defaultCalled)
	}

	m.GetFromKv(context.TODO(), "testkey")
	if f.kvreads != 1 {
		t.Errorf("Expecting 1 call to KV read found %d", f.kvreads)
	}

	m.WriteToKv(context.TODO(), nil, "testprefix", true)
	if f.kvwrites != 1 {
		t.Errorf("Expecting 1 call to KV Write found %d", f.kvwrites)
	}

	m.WriteToKvTxn(context.TODO(), nil, nil, "testprefix", true)
	if f.txnwrites != 1 {
		t.Errorf("Expecting 1 call to Txn write found %d", f.kvdels)
	}
	m.DelFromKv(context.TODO(), "testKey")
	if f.kvdels != 1 {
		t.Errorf("Expecting 1 call to KV Del found %d", f.kvdels)
	}

	m.DelFromKvTxn(context.TODO(), nil, "testKey")
	if f.txndels != 1 {
		t.Errorf("Expecting 1 call to KV Del found %d", f.kvdels)
	}
	ctx := context.TODO()
	md := metadata.Pairs(apisrv.RequestParamVersion, "v1",
		apisrv.RequestParamMethod, "WATCH")
	ctx = metadata.NewContext(ctx, md)
	stream := fakeGrpcStream{ctx: ctx}
	opts := api.ListWatchOptions{}
	err := m.WatchFromKv(&opts, stream, "test")
	if err != nil {
		t.Errorf("watch returned error (%s)", err)
	}
}
