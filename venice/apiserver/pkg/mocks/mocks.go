package mocks

import (
	"context"
	"errors"
	"fmt"

	"google.golang.org/grpc"

	"github.com/pensando/sw/api/graph"

	"github.com/pensando/sw/api"
	apiintf "github.com/pensando/sw/api/interfaces"
	apisrv "github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/utils/kvstore"
	compliance "github.com/pensando/sw/venice/utils/kvstore/compliance"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
)

// FakeServer is a mock for a server
type FakeServer struct {
	SvcMap map[string]*FakeService
}

// Register is used by backends to register during init()
func (f *FakeServer) Register(name string, svc apisrv.ServiceBackend) (apisrv.ServiceBackend, error) {
	return nil, nil
}

// RegisterMessages registers messages defined in a backend.
func (f *FakeServer) RegisterMessages(svc string, msgs map[string]apisrv.Message) {}

// RegisterService registers a service with name in "name"
func (f *FakeServer) RegisterService(name string, svc apisrv.Service) {}

// RegisterHooksCb registers a callback to register hooks for the service svcName
func (f *FakeServer) RegisterHooksCb(svcName string, fn apisrv.ServiceHookCb) {}

// RegisterRestoreCallback is used to restore state by hooks on bootup/restart
func (f *FakeServer) RegisterRestoreCallback(cb apisrv.RestoreCb) {}

// GetService returns a registered service given the name.
func (f *FakeServer) GetService(name string) apisrv.Service {
	return f.SvcMap[name]
}

// GetMessage returns a registered mesage given the kind and service name.
func (f *FakeServer) GetMessage(svc, kind string) apisrv.Message {
	return nil
}

// CreateOverlay creates a new overlay on top of API server cache
func (f *FakeServer) CreateOverlay(tenant, name, basePath string) (apiintf.CacheInterface, error) {
	return nil, nil
}

// Run starts the "eventloop" for the API server.
func (f *FakeServer) Run(config apisrv.Config) {}

// Stop sends a stop signal to the API server
func (f *FakeServer) Stop() {}

// WaitRunning blocks till the API server is completely initialized
func (f *FakeServer) WaitRunning() {}

// GetAddr returns the address at which the API server is listening
//   returns error if the API server is not initialized
func (f *FakeServer) GetAddr() (string, error) { return "", nil }

// GetVersion returns the native version of the API server
func (f *FakeServer) GetVersion() string {
	return ""
}

// GetGraphDB returns the graph DB in use by the Server
func (f *FakeServer) GetGraphDB() graph.Interface {
	return nil
}

// RuntimeFlags returns runtime flags in use by the Server
func (f *FakeServer) RuntimeFlags() apisrv.Flags {
	return apisrv.Flags{}
}

// SetRuntimeControls sets the runtime controls for the server
func (f *FakeServer) SetRuntimeControls(apisrv.Controls) {}

// GetResolvers returns the list of resolvers known to API server for use by hooks that need to non-local work.
//  returns nil on error
func (f *FakeServer) GetResolvers() []string {
	return nil
}

// NewFakeServer returns a new FakeServer
func NewFakeServer() *FakeServer {
	return &FakeServer{SvcMap: make(map[string]*FakeService)}
}

// FakeService is a mock for Service
type FakeService struct {
	name      string
	retMethod map[string]apisrv.Method
}

// Name returns the name for this service
func (s *FakeService) Name() string {
	return s.name
}

// Disable disables the Service
func (s *FakeService) Disable() {}

// Enable enables the Service and all its methods.
func (s *FakeService) Enable() {}

// GetMethod returns the Method object registered given its name. Returns nil when not found
func (s *FakeService) GetMethod(t string) apisrv.Method {
	return s.retMethod[t]
}

// GetCrudService returns the Auto generated CRUD service method for a given (service, operation)
func (s *FakeService) GetCrudService(in string, oper apiintf.APIOperType) apisrv.Method {
	return s.retMethod[in]
}

// AddMethod add a method to the list of methods served by the Service.
func (s *FakeService) AddMethod(n string, m apisrv.Method) apisrv.Method {
	s.retMethod[n] = m
	return m
}

// WithKvWatchFunc  watches for all objects served by this service.
func (s *FakeService) WithKvWatchFunc(fn apisrv.WatchSvcKvFunc) apisrv.Service {
	return s
}

// WatchFromKv implements the watch function from KV store and bridges it to the grpc stream
func (s *FakeService) WatchFromKv(options *api.ListWatchOptions, stream grpc.ServerStream, svcprefix string) error {
	return nil
}

// WithCrudServices registers crud objects served by the service
func (s *FakeService) WithCrudServices(msgs []apisrv.Message) apisrv.Service {
	return s
}

// NewFakeService creates a new FakeService
func NewFakeService() apisrv.Service {
	return &FakeService{retMethod: make(map[string]apisrv.Method)}
}

// FakeMethod is used as mock Method for testing.
type FakeMethod struct {
	Service       apisrv.Service
	Pres          int
	Posts         int
	RWriters      int
	MakeURIs      int
	RollackCalled int
	Skipkv        bool
	Enabled       bool
	ReqMsg        apisrv.Message
	RespMsg       apisrv.Message
	HandleMethod  func(context.Context, interface{}) (interface{}, error)
}

// Enable is a mock method for testing
func (m *FakeMethod) Enable() { m.Enabled = true }

// Disable is a mock method for testing
func (m *FakeMethod) Disable() { m.Enabled = false }

// GetService returns the parent service
func (m *FakeMethod) GetService() apisrv.Service { return m.Service }

// WithRateLimiter is a mock method for testing
func (m *FakeMethod) WithRateLimiter() apisrv.Method { return m }

// WithPreCommitHook is a mock method for testing
func (m *FakeMethod) WithPreCommitHook(fn apisrv.PreCommitFunc) apisrv.Method {
	m.Pres++
	return m
}

// WithPostCommitHook is a mock method for testing
func (m *FakeMethod) WithPostCommitHook(fn apisrv.PostCommitFunc) apisrv.Method {
	m.Posts++
	return m
}

// WithResponseWriter is a mock method for testing
func (m *FakeMethod) WithResponseWriter(fn apisrv.ResponseWriterFunc) apisrv.Method {
	m.RWriters++
	return m
}

// WithOper is a mock method for testing
func (m *FakeMethod) WithOper(oper apiintf.APIOperType) apisrv.Method { return m }

// WithVersion is a mock method for testing
func (m *FakeMethod) WithVersion(ver string) apisrv.Method { return m }

// WithMakeURI set the URI maker function for the method
func (m *FakeMethod) WithMakeURI(fn apisrv.MakeURIFunc) apisrv.Method {
	return m
}

// WithMethDbKey set the URI maker function for the method
func (m *FakeMethod) WithMethDbKey(fn apisrv.MakeMethDbKeyFunc) apisrv.Method {
	return m
}

// WithResourceAllocHook registers a resource allocation callback.
func (m *FakeMethod) WithResourceAllocHook(fn apisrv.ResourceAllocFunc) apisrv.Method {
	return m
}

// GetPrefix is a mock method for testing
func (m *FakeMethod) GetPrefix() string {
	return ""
}

// GetRequestType is a mock method for testing
func (m *FakeMethod) GetRequestType() apisrv.Message { return m.ReqMsg }

// GetResponseType is a mock method for testing
func (m *FakeMethod) GetResponseType() apisrv.Message { return m.RespMsg }

// HandleInvocation is a mock method for testing
func (m *FakeMethod) HandleInvocation(ctx context.Context, i interface{}) (interface{}, error) {
	if m.HandleMethod != nil {
		return m.HandleMethod(ctx, i)
	}
	return nil, nil
}

// MakeURI generates the URI for the method.
func (m *FakeMethod) MakeURI(i interface{}) (string, error) {
	return "", nil
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
func (m *FakeMethod) PrecommitFunc(ctx context.Context, kvs kvstore.Interface, txn kvstore.Txn, key string, oper apiintf.APIOperType, dryrun bool, i interface{}) (interface{}, bool, error) {
	m.Pres++
	if m.Skipkv {
		return i, false, nil
	}
	return i, true, nil
}

// PostcommitfFunc is a mock method for testing
func (m *FakeMethod) PostcommitfFunc(ctx context.Context, oper apiintf.APIOperType, i interface{}, dryrun bool) {
	m.Posts++
}

// RespWriterFunc is a mock method for testing
func (m *FakeMethod) RespWriterFunc(ctx context.Context, kvs kvstore.Interface, prefix string, i, o, resp interface{}, oper apiintf.APIOperType) (interface{}, error) {
	return "TestResponse", nil
}

// MakeURIFunc is a mock method for testing
func (m *FakeMethod) MakeURIFunc(i interface{}) (string, error) {
	m.MakeURIs++
	return "", nil
}

// ResourceAllocFunc  is a mock method for testing
func (m *FakeMethod) ResourceAllocFunc(ctx context.Context, i interface{}, kvstore kvstore.Interface, key string, dryrun bool) (apisrv.ResourceRollbackFn, error) {
	return m.ResourceRollbackFunc, nil

}

// ResourceRollbackFunc  is a mock method for testing
func (m *FakeMethod) ResourceRollbackFunc(ctx context.Context, i interface{}, kvstore kvstore.Interface, key string, dryrun bool) {
	m.RollackCalled++
}

// FakeMessage is used as a mock object for testing.
type FakeMessage struct {
	kind                string
	CalledTxfms         []string
	Kvpath              string
	ValidateRslt        bool
	IgnoreTxnWrite      bool
	ValidateCalled      int
	DefaultCalled       int
	Kvwrites            int
	Kvreads             int
	Kvdels              int
	Kvlists             int
	Kvwatch             int
	Txnwrites           int
	Txngets             int
	Txndels             int
	Objverwrite         int
	Uuidwrite           int
	SelfLinkWrites      int
	CreateTimeWrites    int
	ModTimeWrite        int
	ObjVerWrites        int
	GetReferencesCalled int
	KeyGens             int
	RuntimeObj          runtime.Object
	RefMap              map[string]apiintf.ReferenceObj
	UpdateSpecFunc      func(context.Context, interface{}) kvstore.UpdateFunc
	UpdateStatusFunc    func(interface{}) kvstore.UpdateFunc

	listFromKvFunc    apisrv.ListFromKvFunc
	getFromKvFunc     apisrv.GetFromKvFunc
	getReferencesFunc apisrv.GetReferencesFunc
}

// TxnTestKey is the kvstore key used by mock functions
var TxnTestKey = "/txn/testobj"

// WithTransform is a mock method for testing
func (m *FakeMessage) WithTransform(from, to string, fn apisrv.TransformFunc) apisrv.Message { return m }

// WithValidate is a mock method for testing
func (m *FakeMessage) WithValidate(fn apisrv.ValidateFunc) apisrv.Message { return m }

// WithNormalizer is a mock method for testing
func (m *FakeMessage) WithNormalizer(fn apisrv.NormalizerFunc) apisrv.Message { return m }

// WithKeyGenerator is a mock method for testing
func (m *FakeMessage) WithKeyGenerator(fn apisrv.KeyGenFunc) apisrv.Message { return m }

// WithKvUpdater is a mock method for testing
func (m *FakeMessage) WithKvUpdater(fn apisrv.UpdateKvFunc) apisrv.Message { return m }

// WithKvGetter is a mock method for testing
func (m *FakeMessage) WithKvGetter(fn apisrv.GetFromKvFunc) apisrv.Message {
	m.getFromKvFunc = fn
	return m
}

// WithKvDelFunc is a mock method for testing
func (m *FakeMessage) WithKvDelFunc(fn apisrv.DelFromKvFunc) apisrv.Message { return m }

// WithResponseWriter is a mock method for testing
func (m *FakeMessage) WithResponseWriter(fn apisrv.ResponseWriterFunc) apisrv.Message { return m }

// WithKvTxnDelFunc is a mock method for testing
func (m *FakeMessage) WithKvTxnDelFunc(fn apisrv.DelFromKvTxnFunc) apisrv.Message { return m }

// WithKvTxnUpdater is a mock method for testing
func (m *FakeMessage) WithKvTxnUpdater(fn apisrv.UpdateKvTxnFunc) apisrv.Message { return m }

// WithKvListFunc is a mock method for testing
func (m *FakeMessage) WithKvListFunc(fn apisrv.ListFromKvFunc) apisrv.Message {
	m.listFromKvFunc = fn
	return m
}

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

// WithSelfLinkWriter updates the selflink in the object
func (m *FakeMessage) WithSelfLinkWriter(fn apisrv.UpdateSelfLinkFunc) apisrv.Message {
	return m
}

// WithStorageTransformer is a mock method for testing
func (m *FakeMessage) WithStorageTransformer(stx apisrv.ObjStorageTransformer) apisrv.Message {
	return m
}

// WithUpdateMetaFunction is a consistent update function for updating the Object Meta
func (m *FakeMessage) WithUpdateMetaFunction(fn func(ctx context.Context, in interface{}, create bool) kvstore.UpdateFunc) apisrv.Message {
	return m
}

// WithReplaceSpecFunction is a consistent update function for replacing the Spec
func (m *FakeMessage) WithReplaceSpecFunction(fn func(context.Context, interface{}) kvstore.UpdateFunc) apisrv.Message {
	m.UpdateSpecFunc = fn
	return m
}

// WithReplaceStatusFunction is a consistent update function for replacing the Status
func (m *FakeMessage) WithReplaceStatusFunction(fn func(interface{}) kvstore.UpdateFunc) apisrv.Message {
	m.UpdateStatusFunc = fn
	return m
}

// WithGetRuntimeObject gets the runtime object
func (m *FakeMessage) WithGetRuntimeObject(func(interface{}) runtime.Object) apisrv.Message { return m }

// WithReferencesGetter registers a GetReferencesFunc to the message
func (m *FakeMessage) WithReferencesGetter(fn apisrv.GetReferencesFunc) apisrv.Message {
	m.getReferencesFunc = fn
	return m
}

// GetKind is a mock method for testing
func (m *FakeMessage) GetKind() string { return m.kind }

// WriteObjVersion is a mock method for testing
func (m *FakeMessage) WriteObjVersion(i interface{}, version string) interface{} {
	m.ObjVerWrites++
	return i
}

// ListFromKv is a mock method for testing
func (m *FakeMessage) ListFromKv(ctx context.Context, kvs kvstore.Interface, options *api.ListWatchOptions, prefix string) (interface{}, error) {
	m.Kvlists++
	if m.listFromKvFunc != nil {
		return m.listFromKvFunc(ctx, kvs, options, prefix)
	}
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
func (m *FakeMessage) WriteToKvTxn(ctx context.Context, kvs kvstore.Interface, txn kvstore.Txn, i interface{}, prerfix string, create, updateSpec bool) error {
	m.Txnwrites++
	if !m.IgnoreTxnWrite {
		msg := i.(compliance.TestObj)
		if create {
			txn.Create(TxnTestKey, &msg)
		} else {
			txn.Update(TxnTestKey, &msg)
		}
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
	if m.getFromKvFunc != nil {
		return m.getFromKvFunc(ctx, kv, key)
	}
	return nil, nil
}

// PrepareMsg is a mock method for testing
func (m *FakeMessage) PrepareMsg(from, to string, i interface{}) (interface{}, error) {
	m.CalledTxfms = append(m.CalledTxfms, from+"-"+to)
	return i, nil
}

// Normalize is a mock method for testing
func (m *FakeMessage) Normalize(i interface{}) interface{} {
	return i
}

// Validate is a mock method for testing
func (m *FakeMessage) Validate(i interface{}, ver string, ignoreStatus bool, ignoreSpec bool) []error {
	if m.ValidateRslt {
		return nil
	}
	return []error{errors.New("Setup to fail validation")}
}

// UpdateSelfLink update the object with the self link provided
func (m *FakeMessage) UpdateSelfLink(path, ver, prefix string, i interface{}) (interface{}, error) {
	return i, nil
}

// TransformToStorage is a mock method for testing
func (m *FakeMessage) TransformToStorage(ctx context.Context, oper apiintf.APIOperType, i interface{}) (interface{}, error) {
	return i, nil
}

// TransformFromStorage is a mock method for testing
func (m *FakeMessage) TransformFromStorage(ctx context.Context, oper apiintf.APIOperType, i interface{}) (interface{}, error) {
	return i, nil
}

// TransformCb is a mock method for testing
func (m *FakeMessage) TransformCb(from, to string, i interface{}) interface{} {
	r, _ := m.PrepareMsg(from, to, i)
	return r
}

// ValidateFunc is a mock method for testing
func (m *FakeMessage) ValidateFunc(i interface{}, ver string, ignstatus, ignoreSpec bool) []error {
	m.ValidateCalled++
	return nil
}

// NormalizerFunc is a mock method for testing
func (m *FakeMessage) NormalizerFunc(i interface{}) interface{} {
	m.DefaultCalled++
	return i
}

// KvUpdateFunc is a mock method for testing
func (m *FakeMessage) KvUpdateFunc(ctx context.Context, kv kvstore.Interface, i interface{}, prefix string, create bool, updateFn kvstore.UpdateFunc) (interface{}, error) {
	m.Kvwrites++
	return i, nil
}

// TxnUpdateFunc is a mock method for testing
func (m *FakeMessage) TxnUpdateFunc(ctx context.Context, kvs kvstore.Interface, txn kvstore.Txn, i interface{}, prefix string, create bool, updatefn kvstore.UpdateFunc) error {
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

// SelfLinkWriterFunc is mock method for testing
func (m *FakeMessage) SelfLinkWriterFunc(path, ver, prefix string, i interface{}) (interface{}, error) {
	m.SelfLinkWrites++
	return i, nil
}

// GetReferencesFunc is a mock method for testing
func (m *FakeMessage) GetReferencesFunc(interface{}) (map[string]apiintf.ReferenceObj, error) {
	m.GetReferencesCalled++
	return m.RefMap, nil
}

// KeyGeneratorFunc is a mock method for testing
func (m *FakeMessage) KeyGeneratorFunc(i interface{}, prefix string) string {
	m.KeyGens++
	return ""
}

// CreateUUID is a mock method for testing
func (m *FakeMessage) CreateUUID(i interface{}) (interface{}, error) {
	m.Uuidwrite++
	return i, nil
}

// WriteCreationTime is a mock method for testing
func (m *FakeMessage) WriteCreationTime(i interface{}) (interface{}, error) {
	m.CreateTimeWrites++
	return i, nil
}

// WriteModTime is a mock method for testing
func (m *FakeMessage) WriteModTime(i interface{}) (interface{}, error) {
	m.ModTimeWrite++
	return i, nil
}

// GetUpdateMetaFunc returns the Update function for meta update
func (m *FakeMessage) GetUpdateMetaFunc() func(context.Context, interface{}, bool) kvstore.UpdateFunc {
	return func(ctx context.Context, i interface{}, create bool) kvstore.UpdateFunc {
		return func(old runtime.Object) (runtime.Object, error) {
			return old, nil
		}
	}
}

// GetUpdateSpecFunc returns the Update function for Spec update
func (m *FakeMessage) GetUpdateSpecFunc() func(context.Context, interface{}) kvstore.UpdateFunc {
	if m.UpdateSpecFunc != nil {
		return m.UpdateSpecFunc
	}
	return func(ctx context.Context, i interface{}) kvstore.UpdateFunc {
		return func(old runtime.Object) (runtime.Object, error) {
			return old, nil
		}
	}
}

// GetUpdateStatusFunc returns the Update function for Status update
func (m *FakeMessage) GetUpdateStatusFunc() func(interface{}) kvstore.UpdateFunc {
	if m.UpdateStatusFunc != nil {
		return m.UpdateStatusFunc
	}
	return func(i interface{}) kvstore.UpdateFunc {
		return func(old runtime.Object) (runtime.Object, error) {
			return old, nil
		}
	}
}

// GetRuntimeObject retursn the runtime.Object
func (m *FakeMessage) GetRuntimeObject(in interface{}) runtime.Object {
	if in != nil {
		return m.RuntimeObj
	}
	return nil
}

// GetReferences fetches the references for the object.
func (m *FakeMessage) GetReferences(in interface{}) (map[string]apiintf.ReferenceObj, error) {
	if m.getReferencesFunc != nil {
		return m.getReferencesFunc(in)
	}
	return nil, nil
}

// NewFakeMessage create a new FakeMessage
func NewFakeMessage(kind, Kvpath string, validateResult bool) apisrv.Message {
	r := FakeMessage{
		kind:         kind,
		Kvpath:       Kvpath,
		ValidateRslt: validateResult,
	}
	return &r
}

// ObjStorageTransformer is a mock storage transformer for testing
// It only counts the invocations of each method
type ObjStorageTransformer struct {
	TransformToStorageCalled   int
	TransformFromStorageCalled int
}

// TransformFromStorage is a mock method for testing
func (m *ObjStorageTransformer) TransformFromStorage(ctx context.Context, i interface{}) (interface{}, error) {
	m.TransformFromStorageCalled++
	return i, nil
}

// TransformToStorage is a mock method for testing
func (m *ObjStorageTransformer) TransformToStorage(ctx context.Context, i interface{}) (interface{}, error) {
	m.TransformToStorageCalled++
	return i, nil
}
