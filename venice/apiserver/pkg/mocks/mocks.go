package mocks

import (
	"context"
	"errors"
	"fmt"

	"google.golang.org/grpc"

	"github.com/pensando/sw/api"
	apisrv "github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
)

// FakeService is a mock for Service
type FakeService struct {
	retMethod apisrv.Method
}

// Disable disables the Service
func (s *FakeService) Disable() {}

// Enable enables the Service and all its methods.
func (s *FakeService) Enable() {}

// GetMethod returns the Method object registered given its name. Returns nil when not found
func (s *FakeService) GetMethod(t string) apisrv.Method {
	return s.retMethod
}

// GetCrudService returns the Auto generated CRUD service method for a given (service, operation)
func (s *FakeService) GetCrudService(in string, oper apisrv.APIOperType) apisrv.Method {
	return s.retMethod
}

// AddMethod add a method to the list of methods served by the Service.
func (s *FakeService) AddMethod(n string, m apisrv.Method) apisrv.Method {
	s.retMethod = m
	return m
}

// NewFakeService creates a new FakeService
func NewFakeService() apisrv.Service {
	return &FakeService{}
}

// FakeMethod is used as mock Method for testing.
type FakeMethod struct {
	Pres    int
	Posts   int
	Skipkv  bool
	Enabled bool
	ReqMsg  apisrv.Message
	RespMsg apisrv.Message
}

// Enable is a mock method for testing
func (m *FakeMethod) Enable() { m.Enabled = true }

// Disable is a mock method for testing
func (m *FakeMethod) Disable() { m.Enabled = false }

// WithRateLimiter is a mock method for testing
func (m *FakeMethod) WithRateLimiter() apisrv.Method { return m }

// WithPreCommitHook is a mock method for testing
func (m *FakeMethod) WithPreCommitHook(fn apisrv.PreCommitFunc) apisrv.Method { return m }

// WithPostCommitHook is a mock method for testing
func (m *FakeMethod) WithPostCommitHook(fn apisrv.PostCommitFunc) apisrv.Method { return m }

// WithResponseWriter is a mock method for testing
func (m *FakeMethod) WithResponseWriter(fn apisrv.ResponseWriterFunc) apisrv.Method { return m }

// WithOper is a mock method for testing
func (m *FakeMethod) WithOper(oper apisrv.APIOperType) apisrv.Method { return m }

// WithVersion is a mock method for testing
func (m *FakeMethod) WithVersion(ver string) apisrv.Method { return m }

// GetRequestType is a mock method for testing
func (m *FakeMethod) GetRequestType() apisrv.Message { return m.ReqMsg }

// GetResponseType is a mock method for testing
func (m *FakeMethod) GetResponseType() apisrv.Message { return m.RespMsg }

// HandleInvocation is a mock method for testing
func (m *FakeMethod) HandleInvocation(ctx context.Context, i interface{}) (interface{}, error) {
	return nil, nil
}

// NewFakeMethod creates a new FakeMethod
func NewFakeMethod(skipkv bool) apisrv.Method {
	return &FakeMethod{Skipkv: skipkv}
}

// SetFakeMethodReqType sets the reqtype for the FakeMethod
func SetFakeMethodReqType(msg apisrv.Message, method apisrv.Method) bool {
	f, ok := method.(*FakeMethod)
	if !ok {
		return false
	}
	f.ReqMsg = msg
	return true
}

// SetFakeMethodRespType sets the response type for the FakeMethod
func SetFakeMethodRespType(msg apisrv.Message, method apisrv.Method) error {
	f, ok := method.(*FakeMethod)
	if !ok {
		return fmt.Errorf("not fake method")
	}
	f.RespMsg = msg
	return nil
}

// PrecommitFunc is a mock method for testing
func (m *FakeMethod) PrecommitFunc(ctx context.Context, kvs kvstore.Interface, txn kvstore.Txn, key string, oper apisrv.APIOperType, i interface{}) (interface{}, bool, error) {
	m.Pres++
	if m.Skipkv {
		return i, false, nil
	}
	return i, true, nil
}

// PostcommitfFunc is a mock method for testing
func (m *FakeMethod) PostcommitfFunc(ctx context.Context, oper apisrv.APIOperType, i interface{}) {
	m.Posts++
}

// RespWriterFunc is a mock method for testing
func (m *FakeMethod) RespWriterFunc(ctx context.Context, kvs kvstore.Interface, prefix string, i interface{}, o interface{}, oper apisrv.APIOperType) (interface{}, error) {
	return "TestResponse", nil
}

// FakeMessage is used as a mock object for testing.
type FakeMessage struct {
	CalledTxfms    []string
	Kvpath         string
	ValidateRslt   bool
	ValidateCalled int
	DefaultCalled  int
	Kvwrites       int
	Kvreads        int
	Kvdels         int
	Kvlists        int
	Kvwatch        int
	Txnwrites      int
	Txngets        int
	Txndels        int
	Objverwrite    int
	Uuidwrite      int
	KvObj          interface{}
}

// TxnTestKey is the kvstore key used by mock functions
var TxnTestKey = "/txn/testobj"

// WithTransform is a mock method for testing
func (m *FakeMessage) WithTransform(from, to string, fn apisrv.TransformFunc) apisrv.Message { return m }

// WithValidate is a mock method for testing
func (m *FakeMessage) WithValidate(fn apisrv.ValidateFunc) apisrv.Message { return m }

// WithDefaulter is a mock method for testing
func (m *FakeMessage) WithDefaulter(fn apisrv.DefaulterFunc) apisrv.Message { return m }

// WithKeyGenerator is a mock method for testing
func (m *FakeMessage) WithKeyGenerator(fn apisrv.KeyGenFunc) apisrv.Message { return m }

// WithKvUpdater is a mock method for testing
func (m *FakeMessage) WithKvUpdater(fn apisrv.UpdateKvFunc) apisrv.Message { return m }

// WithKvGetter is a mock method for testing
func (m *FakeMessage) WithKvGetter(fn apisrv.GetFromKvFunc) apisrv.Message { return m }

// WithKvDelFunc is a mock method for testing
func (m *FakeMessage) WithKvDelFunc(fn apisrv.DelFromKvFunc) apisrv.Message { return m }

// WithResponseWriter is a mock method for testing
func (m *FakeMessage) WithResponseWriter(fn apisrv.ResponseWriterFunc) apisrv.Message { return m }

// WithKvTxnDelFunc is a mock method for testing
func (m *FakeMessage) WithKvTxnDelFunc(fn apisrv.DelFromKvTxnFunc) apisrv.Message { return m }

// WithKvTxnUpdater is a mock method for testing
func (m *FakeMessage) WithKvTxnUpdater(fn apisrv.UpdateKvTxnFunc) apisrv.Message { return m }

// WithKvListFunc is a mock method for testing
func (m *FakeMessage) WithKvListFunc(fn apisrv.ListFromKvFunc) apisrv.Message { return m }

// WithKvWatchFunc is a mock method for testing
func (m *FakeMessage) WithKvWatchFunc(fn apisrv.WatchKvFunc) apisrv.Message { return m }

// WithObjectVersionWriter is a mock method for testing
func (m *FakeMessage) WithObjectVersionWriter(fn apisrv.SetObjectVersionFunc) apisrv.Message { return m }

// WithUUIDWriter is a mock method for testing
func (m *FakeMessage) WithUUIDWriter(fn apisrv.CreateUUIDFunc) apisrv.Message { return m }

// WithCreationTimeWriter is a mock method for testing
func (m *FakeMessage) WithCreationTimeWriter(fn apisrv.SetCreationTimeFunc) apisrv.Message { return m }

// WithModTimeWriter is a mock method for testing
func (m *FakeMessage) WithModTimeWriter(fn apisrv.SetModTimeFunc) apisrv.Message { return m }

// GetKind is a mock method for testing
func (m *FakeMessage) GetKind() string { return "" }

// WriteObjVersion is a mock method for testing
func (m *FakeMessage) WriteObjVersion(i interface{}, version string) interface{} { return i }

// ListFromKv is a mock method for testing
func (m *FakeMessage) ListFromKv(ctx context.Context, kvs kvstore.Interface, options *api.ListWatchOptions, prefix string) (interface{}, error) {
	m.Kvlists++
	return nil, nil
}

// WatchFromKv is a mock method for testing
func (m *FakeMessage) WatchFromKv(options *api.ListWatchOptions, stream grpc.ServerStream, prefix string) error {
	m.Kvwatch++
	return nil
}

// DelFromKv is a mock method for testing
func (m *FakeMessage) DelFromKv(ctx context.Context, kv kvstore.Interface, key string) (interface{}, error) {
	m.Kvdels++
	return nil, nil
}

// DelFromKvTxn is a mock method for testing
func (m *FakeMessage) DelFromKvTxn(ctx context.Context, txn kvstore.Txn, key string) error {
	m.Txndels++
	txn.Delete(key)
	return nil
}

// WriteToKvTxn is a mock method for testing
func (m *FakeMessage) WriteToKvTxn(ctx context.Context, txn kvstore.Txn, i interface{}, prerfix string, create bool) error {
	m.Txnwrites++
	msg := i.(*kvstore.TestObj)
	if create {
		txn.Create(TxnTestKey, msg)
	} else {
		txn.Update(TxnTestKey, msg)
	}
	return nil
}

// GetKVKey is a mock method for testing
func (m *FakeMessage) GetKVKey(i interface{}, prefix string) (string, error) {
	return m.Kvpath, nil
}

// WriteToKv is a mock method for testing
func (m *FakeMessage) WriteToKv(ctx context.Context, kv kvstore.Interface, i interface{}, prerfix string, create, ignStatus bool) (interface{}, error) {
	m.Kvwrites++
	return i, nil
}

// GetFromKv is a mock method for testing
func (m *FakeMessage) GetFromKv(ctx context.Context, kv kvstore.Interface, key string) (interface{}, error) {
	m.Kvreads++
	return m.KvObj, nil
}

// PrepareMsg is a mock method for testing
func (m *FakeMessage) PrepareMsg(from, to string, i interface{}) (interface{}, error) {
	m.CalledTxfms = append(m.CalledTxfms, from+"-"+to)
	return i, nil
}

// Default is a mock method for testing
func (m *FakeMessage) Default(i interface{}) interface{} {
	return i
}

// Validate is a mock method for testing
func (m *FakeMessage) Validate(i interface{}, ver string, ignoreStatus bool) error {
	if m.ValidateRslt {
		return nil
	}
	return errors.New("Setup to fail validation")
}

// TransformCb is a mock method for testing
func (m *FakeMessage) TransformCb(from, to string, i interface{}) interface{} {
	r, _ := m.PrepareMsg(from, to, i)
	return r
}

// ValidateFunc is a mock method for testing
func (m *FakeMessage) ValidateFunc(i interface{}, ver string, ignstatus bool) error {
	m.ValidateCalled++
	return nil
}

// DefaultFunc is a mock method for testing
func (m *FakeMessage) DefaultFunc(i interface{}) interface{} {
	m.DefaultCalled++
	return i
}

// KvUpdateFunc is a mock method for testing
func (m *FakeMessage) KvUpdateFunc(ctx context.Context, kv kvstore.Interface, i interface{}, prefix string, create bool, ignstatus bool) (interface{}, error) {
	m.Kvwrites++
	return i, nil
}

// TxnUpdateFunc is a mock method for testing
func (m *FakeMessage) TxnUpdateFunc(ctx context.Context, kvstore kvstore.Txn, i interface{}, prefix string, create bool) error {
	m.Txnwrites++
	return nil
}

// KvGetFunc is a mock method for testing
func (m *FakeMessage) KvGetFunc(ctx context.Context, kv kvstore.Interface, key string) (interface{}, error) {
	m.Kvreads++
	return nil, nil
}

// KvDelFunc is a mock method for testing
func (m *FakeMessage) KvDelFunc(ctx context.Context, kv kvstore.Interface, key string) (interface{}, error) {
	m.Kvdels++
	return nil, nil
}

// DelFromKvTxnFunc is a mock method for testing
func (m *FakeMessage) DelFromKvTxnFunc(ctx context.Context, kvstore kvstore.Txn, key string) error {
	m.Txndels++
	return nil
}

// ObjverwriteFunc is a mock method for testing
func (m *FakeMessage) ObjverwriteFunc(i interface{}, version string) interface{} {
	m.Objverwrite++
	return i
}

// KvwatchFunc is a mock method for testing
func (m *FakeMessage) KvwatchFunc(l log.Logger, options *api.ListWatchOptions, kvs kvstore.Interface, stream interface{}, txfn func(from, to string, i interface{}) (interface{}, error), version, svcprefix string) error {
	m.Kvwatch++
	return nil
}

// KvListFunc is a mock method for testing
func (m *FakeMessage) KvListFunc(ctx context.Context, kvs kvstore.Interface, options *api.ListWatchOptions, prefix string) (interface{}, error) {
	m.Kvlists++
	return nil, nil
}

// CreateUUID is a mock method for testing
func (m *FakeMessage) CreateUUID(i interface{}) (interface{}, error) {
	m.Uuidwrite++
	return i, nil
}

// WriteCreationTime is a mock method for testing
func (m *FakeMessage) WriteCreationTime(i interface{}) (interface{}, error) {
	return i, nil
}

// WriteModTime is a mock method for testing
func (m *FakeMessage) WriteModTime(i interface{}) (interface{}, error) {
	return i, nil
}

// NewFakeMessage create a new FakeMessage
func NewFakeMessage(Kvpath string, validateResult bool) apisrv.Message {
	r := FakeMessage{Kvpath: Kvpath, ValidateRslt: validateResult}
	return &r
}

// SetFakeMessageKvObj sets the KV object returned by the FakeMessage
func SetFakeMessageKvObj(obj interface{}, i apisrv.Message) bool {
	m, ok := i.(*FakeMessage)
	if !ok {
		return false
	}
	m.KvObj = obj
	return true
}
