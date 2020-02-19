// Code generated by MockGen. DO NOT EDIT.
// Source: ../generated/workload/svc_workload_crudinterface.go

// Package mock is a generated GoMock package.
package mock

import (
	context "context"
	reflect "reflect"

	gomock "github.com/golang/mock/gomock"

	api "github.com/pensando/sw/api"
	workload "github.com/pensando/sw/api/generated/workload"
	interfaces "github.com/pensando/sw/api/interfaces"
	kvstore "github.com/pensando/sw/venice/utils/kvstore"
)

// MockWorkloadV1EndpointInterface is a mock of (network.WorkloadV1EndpointInterface)interface
type MockWorkloadV1EndpointInterface struct {
	ctrl     *gomock.Controller
	recorder *MockWorkloadV1EndpointInterfaceMockRecorder
}

// MockWorkloadV1EndpointInterfaceMockRecorder is the mock recorder for MockWorkloadV1EndpointInterface
type MockWorkloadV1EndpointInterfaceMockRecorder struct {
	mock *MockWorkloadV1EndpointInterface
}

// NewMockWorkloadV1EndpointInterface creates a new mock instance
func NewMockWorkloadV1EndpointInterface(ctrl *gomock.Controller) *MockWorkloadV1EndpointInterface {
	mock := &MockWorkloadV1EndpointInterface{ctrl: ctrl}
	mock.recorder = &MockWorkloadV1EndpointInterfaceMockRecorder{mock}
	return mock
}

// EXPECT returns an object that allows the caller to indicate expected use
func (m *MockWorkloadV1EndpointInterface) EXPECT() *MockWorkloadV1EndpointInterfaceMockRecorder {
	return m.recorder
}

// Create mocks base method
func (m *MockWorkloadV1EndpointInterface) Create(ctx context.Context, in *workload.Endpoint) (*workload.Endpoint, error) {
	ret := m.ctrl.Call(m, "Create", ctx, in)
	ret0, _ := ret[0].(*workload.Endpoint)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// Create indicates an expected call of Create
func (mr *MockWorkloadV1EndpointInterfaceMockRecorder) Create(ctx, in interface{}) *gomock.Call {
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "Create", reflect.TypeOf((*MockWorkloadV1EndpointInterface)(nil).Create), ctx, in)
}

// Update mocks base method
func (m *MockWorkloadV1EndpointInterface) Update(ctx context.Context, in *workload.Endpoint) (*workload.Endpoint, error) {
	ret := m.ctrl.Call(m, "Update", ctx, in)
	ret0, _ := ret[0].(*workload.Endpoint)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// Update indicates an expected call of Update
func (mr *MockWorkloadV1EndpointInterfaceMockRecorder) Update(ctx, in interface{}) *gomock.Call {
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "Update", reflect.TypeOf((*MockWorkloadV1EndpointInterface)(nil).Update), ctx, in)
}

// UpdateStatus mocks base method
func (m *MockWorkloadV1EndpointInterface) UpdateStatus(ctx context.Context, in *workload.Endpoint) (*workload.Endpoint, error) {
	ret := m.ctrl.Call(m, "UpdateStatus", ctx, in)
	ret0, _ := ret[0].(*workload.Endpoint)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// UpdateStatus indicates an expected call of UpdateStatus
func (mr *MockWorkloadV1EndpointInterfaceMockRecorder) UpdateStatus(ctx, in interface{}) *gomock.Call {
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "UpdateStatus", reflect.TypeOf((*MockWorkloadV1EndpointInterface)(nil).UpdateStatus), ctx, in)
}

// Label mocks base method
func (m *MockWorkloadV1EndpointInterface) Label(ctx context.Context, in *api.Label) (*workload.Endpoint, error) {
	ret := m.ctrl.Call(m, "Label", ctx, in)
	ret0, _ := ret[0].(*workload.Endpoint)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// Label indicates an expected call of Label
func (mr *MockWorkloadV1EndpointInterfaceMockRecorder) Label(ctx, in interface{}) *gomock.Call {
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "Label", reflect.TypeOf((*MockWorkloadV1EndpointInterface)(nil).Label), ctx, in)
}

// Get mocks base method
func (m *MockWorkloadV1EndpointInterface) Get(ctx context.Context, objMeta *api.ObjectMeta) (*workload.Endpoint, error) {
	ret := m.ctrl.Call(m, "Get", ctx, objMeta)
	ret0, _ := ret[0].(*workload.Endpoint)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// Get indicates an expected call of Get
func (mr *MockWorkloadV1EndpointInterfaceMockRecorder) Get(ctx, objMeta interface{}) *gomock.Call {
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "Get", reflect.TypeOf((*MockWorkloadV1EndpointInterface)(nil).Get), ctx, objMeta)
}

// Delete mocks base method
func (m *MockWorkloadV1EndpointInterface) Delete(ctx context.Context, objMeta *api.ObjectMeta) (*workload.Endpoint, error) {
	ret := m.ctrl.Call(m, "Delete", ctx, objMeta)
	ret0, _ := ret[0].(*workload.Endpoint)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// Delete indicates an expected call of Delete
func (mr *MockWorkloadV1EndpointInterfaceMockRecorder) Delete(ctx, objMeta interface{}) *gomock.Call {
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "Delete", reflect.TypeOf((*MockWorkloadV1EndpointInterface)(nil).Delete), ctx, objMeta)
}

// List mocks base method
func (m *MockWorkloadV1EndpointInterface) List(ctx context.Context, options *api.ListWatchOptions) ([]*workload.Endpoint, error) {
	ret := m.ctrl.Call(m, "List", ctx, options)
	ret0, _ := ret[0].([]*workload.Endpoint)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// List indicates an expected call of List
func (mr *MockWorkloadV1EndpointInterfaceMockRecorder) List(ctx, options interface{}) *gomock.Call {
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "List", reflect.TypeOf((*MockWorkloadV1EndpointInterface)(nil).List), ctx, options)
}

// Watch mocks base method
func (m *MockWorkloadV1EndpointInterface) Watch(ctx context.Context, options *api.ListWatchOptions) (kvstore.Watcher, error) {
	ret := m.ctrl.Call(m, "Watch", ctx, options)
	ret0, _ := ret[0].(kvstore.Watcher)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// Watch indicates an expected call of Watch
func (mr *MockWorkloadV1EndpointInterfaceMockRecorder) Watch(ctx, options interface{}) *gomock.Call {
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "Watch", reflect.TypeOf((*MockWorkloadV1EndpointInterface)(nil).Watch), ctx, options)
}

// Allowed mocks base method
func (m *MockWorkloadV1EndpointInterface) Allowed(oper interfaces.APIOperType) bool {
	ret := m.ctrl.Call(m, "Allowed", oper)
	ret0, _ := ret[0].(bool)
	return ret0
}

// Allowed indicates an expected call of Allowed
func (mr *MockWorkloadV1EndpointInterfaceMockRecorder) Allowed(oper interface{}) *gomock.Call {
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "Allowed", reflect.TypeOf((*MockWorkloadV1EndpointInterface)(nil).Allowed), oper)
}

// MockWorkloadV1WorkloadInterface is a mock of (network.WorkloadV1WorkloadInterface)interface
type MockWorkloadV1WorkloadInterface struct {
	ctrl     *gomock.Controller
	recorder *MockWorkloadV1WorkloadInterfaceMockRecorder
}

// MockWorkloadV1WorkloadInterfaceMockRecorder is the mock recorder for MockWorkloadV1WorkloadInterface
type MockWorkloadV1WorkloadInterfaceMockRecorder struct {
	mock *MockWorkloadV1WorkloadInterface
}

// NewMockWorkloadV1WorkloadInterface creates a new mock instance
func NewMockWorkloadV1WorkloadInterface(ctrl *gomock.Controller) *MockWorkloadV1WorkloadInterface {
	mock := &MockWorkloadV1WorkloadInterface{ctrl: ctrl}
	mock.recorder = &MockWorkloadV1WorkloadInterfaceMockRecorder{mock}
	return mock
}

// EXPECT returns an object that allows the caller to indicate expected use
func (m *MockWorkloadV1WorkloadInterface) EXPECT() *MockWorkloadV1WorkloadInterfaceMockRecorder {
	return m.recorder
}

// Create mocks base method
func (m *MockWorkloadV1WorkloadInterface) Create(ctx context.Context, in *workload.Workload) (*workload.Workload, error) {
	ret := m.ctrl.Call(m, "Create", ctx, in)
	ret0, _ := ret[0].(*workload.Workload)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// Create indicates an expected call of Create
func (mr *MockWorkloadV1WorkloadInterfaceMockRecorder) Create(ctx, in interface{}) *gomock.Call {
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "Create", reflect.TypeOf((*MockWorkloadV1WorkloadInterface)(nil).Create), ctx, in)
}

// Update mocks base method
func (m *MockWorkloadV1WorkloadInterface) Update(ctx context.Context, in *workload.Workload) (*workload.Workload, error) {
	ret := m.ctrl.Call(m, "Update", ctx, in)
	ret0, _ := ret[0].(*workload.Workload)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// Update indicates an expected call of Update
func (mr *MockWorkloadV1WorkloadInterfaceMockRecorder) Update(ctx, in interface{}) *gomock.Call {
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "Update", reflect.TypeOf((*MockWorkloadV1WorkloadInterface)(nil).Update), ctx, in)
}

// UpdateStatus mocks base method
func (m *MockWorkloadV1WorkloadInterface) UpdateStatus(ctx context.Context, in *workload.Workload) (*workload.Workload, error) {
	ret := m.ctrl.Call(m, "UpdateStatus", ctx, in)
	ret0, _ := ret[0].(*workload.Workload)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// UpdateStatus indicates an expected call of UpdateStatus
func (mr *MockWorkloadV1WorkloadInterfaceMockRecorder) UpdateStatus(ctx, in interface{}) *gomock.Call {
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "UpdateStatus", reflect.TypeOf((*MockWorkloadV1WorkloadInterface)(nil).UpdateStatus), ctx, in)
}

// Label mocks base method
func (m *MockWorkloadV1WorkloadInterface) Label(ctx context.Context, in *api.Label) (*workload.Workload, error) {
	ret := m.ctrl.Call(m, "Label", ctx, in)
	ret0, _ := ret[0].(*workload.Workload)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// Label indicates an expected call of Label
func (mr *MockWorkloadV1WorkloadInterfaceMockRecorder) Label(ctx, in interface{}) *gomock.Call {
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "Label", reflect.TypeOf((*MockWorkloadV1WorkloadInterface)(nil).Label), ctx, in)
}

// Get mocks base method
func (m *MockWorkloadV1WorkloadInterface) Get(ctx context.Context, objMeta *api.ObjectMeta) (*workload.Workload, error) {
	ret := m.ctrl.Call(m, "Get", ctx, objMeta)
	ret0, _ := ret[0].(*workload.Workload)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// Get indicates an expected call of Get
func (mr *MockWorkloadV1WorkloadInterfaceMockRecorder) Get(ctx, objMeta interface{}) *gomock.Call {
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "Get", reflect.TypeOf((*MockWorkloadV1WorkloadInterface)(nil).Get), ctx, objMeta)
}

// Delete mocks base method
func (m *MockWorkloadV1WorkloadInterface) Delete(ctx context.Context, objMeta *api.ObjectMeta) (*workload.Workload, error) {
	ret := m.ctrl.Call(m, "Delete", ctx, objMeta)
	ret0, _ := ret[0].(*workload.Workload)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// Delete indicates an expected call of Delete
func (mr *MockWorkloadV1WorkloadInterfaceMockRecorder) Delete(ctx, objMeta interface{}) *gomock.Call {
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "Delete", reflect.TypeOf((*MockWorkloadV1WorkloadInterface)(nil).Delete), ctx, objMeta)
}

// List mocks base method
func (m *MockWorkloadV1WorkloadInterface) List(ctx context.Context, options *api.ListWatchOptions) ([]*workload.Workload, error) {
	ret := m.ctrl.Call(m, "List", ctx, options)
	ret0, _ := ret[0].([]*workload.Workload)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// List indicates an expected call of List
func (mr *MockWorkloadV1WorkloadInterfaceMockRecorder) List(ctx, options interface{}) *gomock.Call {
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "List", reflect.TypeOf((*MockWorkloadV1WorkloadInterface)(nil).List), ctx, options)
}

// Watch mocks base method
func (m *MockWorkloadV1WorkloadInterface) Watch(ctx context.Context, options *api.ListWatchOptions) (kvstore.Watcher, error) {
	ret := m.ctrl.Call(m, "Watch", ctx, options)
	ret0, _ := ret[0].(kvstore.Watcher)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// Watch indicates an expected call of Watch
func (mr *MockWorkloadV1WorkloadInterfaceMockRecorder) Watch(ctx, options interface{}) *gomock.Call {
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "Watch", reflect.TypeOf((*MockWorkloadV1WorkloadInterface)(nil).Watch), ctx, options)
}

// Allowed mocks base method
func (m *MockWorkloadV1WorkloadInterface) Allowed(oper interfaces.APIOperType) bool {
	ret := m.ctrl.Call(m, "Allowed", oper)
	ret0, _ := ret[0].(bool)
	return ret0
}

// Allowed indicates an expected call of Allowed
func (mr *MockWorkloadV1WorkloadInterfaceMockRecorder) Allowed(oper interface{}) *gomock.Call {
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "Allowed", reflect.TypeOf((*MockWorkloadV1WorkloadInterface)(nil).Allowed), oper)
}

// StartMigration mocks base method
func (m *MockWorkloadV1WorkloadInterface) StartMigration(ctx context.Context, in *workload.Workload) (*workload.Workload, error) {
	ret := m.ctrl.Call(m, "StartMigration", ctx, in)
	ret0, _ := ret[0].(*workload.Workload)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// StartMigration indicates an expected call of StartMigration
func (mr *MockWorkloadV1WorkloadInterfaceMockRecorder) StartMigration(ctx, in interface{}) *gomock.Call {
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "StartMigration", reflect.TypeOf((*MockWorkloadV1WorkloadInterface)(nil).StartMigration), ctx, in)
}

// FinishMigration mocks base method
func (m *MockWorkloadV1WorkloadInterface) FinishMigration(ctx context.Context, in *workload.Workload) (*workload.Workload, error) {
	ret := m.ctrl.Call(m, "FinishMigration", ctx, in)
	ret0, _ := ret[0].(*workload.Workload)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// FinishMigration indicates an expected call of FinishMigration
func (mr *MockWorkloadV1WorkloadInterfaceMockRecorder) FinishMigration(ctx, in interface{}) *gomock.Call {
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "FinishMigration", reflect.TypeOf((*MockWorkloadV1WorkloadInterface)(nil).FinishMigration), ctx, in)
}

// AbortMigration mocks base method
func (m *MockWorkloadV1WorkloadInterface) AbortMigration(ctx context.Context, in *workload.Workload) (*workload.Workload, error) {
	ret := m.ctrl.Call(m, "AbortMigration", ctx, in)
	ret0, _ := ret[0].(*workload.Workload)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// AbortMigration indicates an expected call of AbortMigration
func (mr *MockWorkloadV1WorkloadInterfaceMockRecorder) AbortMigration(ctx, in interface{}) *gomock.Call {
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "AbortMigration", reflect.TypeOf((*MockWorkloadV1WorkloadInterface)(nil).AbortMigration), ctx, in)
}

// MockWorkloadV1Interface is a mock of WorkloadV1Interface interface
type MockWorkloadV1Interface struct {
	ctrl     *gomock.Controller
	recorder *MockWorkloadV1InterfaceMockRecorder
}

// MockWorkloadV1InterfaceMockRecorder is the mock recorder for MockWorkloadV1Interface
type MockWorkloadV1InterfaceMockRecorder struct {
	mock *MockWorkloadV1Interface
}

// NewMockWorkloadV1Interface creates a new mock instance
func NewMockWorkloadV1Interface(ctrl *gomock.Controller) *MockWorkloadV1Interface {
	mock := &MockWorkloadV1Interface{ctrl: ctrl}
	mock.recorder = &MockWorkloadV1InterfaceMockRecorder{mock}
	return mock
}

// EXPECT returns an object that allows the caller to indicate expected use
func (m *MockWorkloadV1Interface) EXPECT() *MockWorkloadV1InterfaceMockRecorder {
	return m.recorder
}

// Endpoint mocks base method
func (m *MockWorkloadV1Interface) Endpoint() workload.WorkloadV1EndpointInterface {
	ret := m.ctrl.Call(m, "Endpoint")
	ret0, _ := ret[0].(workload.WorkloadV1EndpointInterface)
	return ret0
}

// Endpoint indicates an expected call of Endpoint
func (mr *MockWorkloadV1InterfaceMockRecorder) Endpoint() *gomock.Call {
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "Endpoint", reflect.TypeOf((*MockWorkloadV1Interface)(nil).Endpoint))
}

// Workload mocks base method
func (m *MockWorkloadV1Interface) Workload() workload.WorkloadV1WorkloadInterface {
	ret := m.ctrl.Call(m, "Workload")
	ret0, _ := ret[0].(workload.WorkloadV1WorkloadInterface)
	return ret0
}

// Workload indicates an expected call of Workload
func (mr *MockWorkloadV1InterfaceMockRecorder) Workload() *gomock.Call {
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "Workload", reflect.TypeOf((*MockWorkloadV1Interface)(nil).Workload))
}

// Watch mocks base method
func (m *MockWorkloadV1Interface) Watch(ctx context.Context, options *api.ListWatchOptions) (kvstore.Watcher, error) {
	ret := m.ctrl.Call(m, "Watch", ctx, options)
	ret0, _ := ret[0].(kvstore.Watcher)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// Watch indicates an expected call of Watch
func (mr *MockWorkloadV1InterfaceMockRecorder) Watch(ctx, options interface{}) *gomock.Call {
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "Watch", reflect.TypeOf((*MockWorkloadV1Interface)(nil).Watch), ctx, options)
}
