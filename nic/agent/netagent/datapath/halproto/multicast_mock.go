// Code generated by MockGen. DO NOT EDIT.
// Source: multicast.pb.go

// Package halproto is a generated GoMock package.
package halproto

import (
	reflect "reflect"

	gomock "github.com/golang/mock/gomock"
	context "golang.org/x/net/context"
	grpc "google.golang.org/grpc"
)

// MockisOifList_PdInfo is a mock of isOifList_PdInfo interface
type MockisOifList_PdInfo struct {
	ctrl     *gomock.Controller
	recorder *MockisOifList_PdInfoMockRecorder
}

// MockisOifList_PdInfoMockRecorder is the mock recorder for MockisOifList_PdInfo
type MockisOifList_PdInfoMockRecorder struct {
	mock *MockisOifList_PdInfo
}

// NewMockisOifList_PdInfo creates a new mock instance
func NewMockisOifList_PdInfo(ctrl *gomock.Controller) *MockisOifList_PdInfo {
	mock := &MockisOifList_PdInfo{ctrl: ctrl}
	mock.recorder = &MockisOifList_PdInfoMockRecorder{mock}
	return mock
}

// EXPECT returns an object that allows the caller to indicate expected use
func (m *MockisOifList_PdInfo) EXPECT() *MockisOifList_PdInfoMockRecorder {
	return m.recorder
}

// isOifList_PdInfo mocks base method
func (m *MockisOifList_PdInfo) isOifList_PdInfo() {
	m.ctrl.Call(m, "isOifList_PdInfo")
}

// isOifList_PdInfo indicates an expected call of isOifList_PdInfo
func (mr *MockisOifList_PdInfoMockRecorder) isOifList_PdInfo() *gomock.Call {
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "isOifList_PdInfo", reflect.TypeOf((*MockisOifList_PdInfo)(nil).isOifList_PdInfo))
}

// MarshalTo mocks base method
func (m *MockisOifList_PdInfo) MarshalTo(arg0 []byte) (int, error) {
	ret := m.ctrl.Call(m, "MarshalTo", arg0)
	ret0, _ := ret[0].(int)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// MarshalTo indicates an expected call of MarshalTo
func (mr *MockisOifList_PdInfoMockRecorder) MarshalTo(arg0 interface{}) *gomock.Call {
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "MarshalTo", reflect.TypeOf((*MockisOifList_PdInfo)(nil).MarshalTo), arg0)
}

// Size mocks base method
func (m *MockisOifList_PdInfo) Size() int {
	ret := m.ctrl.Call(m, "Size")
	ret0, _ := ret[0].(int)
	return ret0
}

// Size indicates an expected call of Size
func (mr *MockisOifList_PdInfoMockRecorder) Size() *gomock.Call {
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "Size", reflect.TypeOf((*MockisOifList_PdInfo)(nil).Size))
}

// MockisMulticastEntryStatus_PdInfo is a mock of isMulticastEntryStatus_PdInfo interface
type MockisMulticastEntryStatus_PdInfo struct {
	ctrl     *gomock.Controller
	recorder *MockisMulticastEntryStatus_PdInfoMockRecorder
}

// MockisMulticastEntryStatus_PdInfoMockRecorder is the mock recorder for MockisMulticastEntryStatus_PdInfo
type MockisMulticastEntryStatus_PdInfoMockRecorder struct {
	mock *MockisMulticastEntryStatus_PdInfo
}

// NewMockisMulticastEntryStatus_PdInfo creates a new mock instance
func NewMockisMulticastEntryStatus_PdInfo(ctrl *gomock.Controller) *MockisMulticastEntryStatus_PdInfo {
	mock := &MockisMulticastEntryStatus_PdInfo{ctrl: ctrl}
	mock.recorder = &MockisMulticastEntryStatus_PdInfoMockRecorder{mock}
	return mock
}

// EXPECT returns an object that allows the caller to indicate expected use
func (m *MockisMulticastEntryStatus_PdInfo) EXPECT() *MockisMulticastEntryStatus_PdInfoMockRecorder {
	return m.recorder
}

// isMulticastEntryStatus_PdInfo mocks base method
func (m *MockisMulticastEntryStatus_PdInfo) isMulticastEntryStatus_PdInfo() {
	m.ctrl.Call(m, "isMulticastEntryStatus_PdInfo")
}

// isMulticastEntryStatus_PdInfo indicates an expected call of isMulticastEntryStatus_PdInfo
func (mr *MockisMulticastEntryStatus_PdInfoMockRecorder) isMulticastEntryStatus_PdInfo() *gomock.Call {
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "isMulticastEntryStatus_PdInfo", reflect.TypeOf((*MockisMulticastEntryStatus_PdInfo)(nil).isMulticastEntryStatus_PdInfo))
}

// MarshalTo mocks base method
func (m *MockisMulticastEntryStatus_PdInfo) MarshalTo(arg0 []byte) (int, error) {
	ret := m.ctrl.Call(m, "MarshalTo", arg0)
	ret0, _ := ret[0].(int)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// MarshalTo indicates an expected call of MarshalTo
func (mr *MockisMulticastEntryStatus_PdInfoMockRecorder) MarshalTo(arg0 interface{}) *gomock.Call {
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "MarshalTo", reflect.TypeOf((*MockisMulticastEntryStatus_PdInfo)(nil).MarshalTo), arg0)
}

// Size mocks base method
func (m *MockisMulticastEntryStatus_PdInfo) Size() int {
	ret := m.ctrl.Call(m, "Size")
	ret0, _ := ret[0].(int)
	return ret0
}

// Size indicates an expected call of Size
func (mr *MockisMulticastEntryStatus_PdInfoMockRecorder) Size() *gomock.Call {
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "Size", reflect.TypeOf((*MockisMulticastEntryStatus_PdInfo)(nil).Size))
}

// MockMulticastClient is a mock of MulticastClient interface
type MockMulticastClient struct {
	ctrl     *gomock.Controller
	recorder *MockMulticastClientMockRecorder
}

// MockMulticastClientMockRecorder is the mock recorder for MockMulticastClient
type MockMulticastClientMockRecorder struct {
	mock *MockMulticastClient
}

// NewMockMulticastClient creates a new mock instance
func NewMockMulticastClient(ctrl *gomock.Controller) *MockMulticastClient {
	mock := &MockMulticastClient{ctrl: ctrl}
	mock.recorder = &MockMulticastClientMockRecorder{mock}
	return mock
}

// EXPECT returns an object that allows the caller to indicate expected use
func (m *MockMulticastClient) EXPECT() *MockMulticastClientMockRecorder {
	return m.recorder
}

// MulticastEntryCreate mocks base method
func (m *MockMulticastClient) MulticastEntryCreate(ctx context.Context, in *MulticastEntryRequestMsg, opts ...grpc.CallOption) (*MulticastEntryResponseMsg, error) {
	varargs := []interface{}{ctx, in}
	for _, a := range opts {
		varargs = append(varargs, a)
	}
	ret := m.ctrl.Call(m, "MulticastEntryCreate", varargs...)
	ret0, _ := ret[0].(*MulticastEntryResponseMsg)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// MulticastEntryCreate indicates an expected call of MulticastEntryCreate
func (mr *MockMulticastClientMockRecorder) MulticastEntryCreate(ctx, in interface{}, opts ...interface{}) *gomock.Call {
	varargs := append([]interface{}{ctx, in}, opts...)
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "MulticastEntryCreate", reflect.TypeOf((*MockMulticastClient)(nil).MulticastEntryCreate), varargs...)
}

// MulticastEntryUpdate mocks base method
func (m *MockMulticastClient) MulticastEntryUpdate(ctx context.Context, in *MulticastEntryRequestMsg, opts ...grpc.CallOption) (*MulticastEntryResponseMsg, error) {
	varargs := []interface{}{ctx, in}
	for _, a := range opts {
		varargs = append(varargs, a)
	}
	ret := m.ctrl.Call(m, "MulticastEntryUpdate", varargs...)
	ret0, _ := ret[0].(*MulticastEntryResponseMsg)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// MulticastEntryUpdate indicates an expected call of MulticastEntryUpdate
func (mr *MockMulticastClientMockRecorder) MulticastEntryUpdate(ctx, in interface{}, opts ...interface{}) *gomock.Call {
	varargs := append([]interface{}{ctx, in}, opts...)
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "MulticastEntryUpdate", reflect.TypeOf((*MockMulticastClient)(nil).MulticastEntryUpdate), varargs...)
}

// MulticastEntryDelete mocks base method
func (m *MockMulticastClient) MulticastEntryDelete(ctx context.Context, in *MulticastEntryDeleteRequestMsg, opts ...grpc.CallOption) (*MulticastEntryDeleteResponseMsg, error) {
	varargs := []interface{}{ctx, in}
	for _, a := range opts {
		varargs = append(varargs, a)
	}
	ret := m.ctrl.Call(m, "MulticastEntryDelete", varargs...)
	ret0, _ := ret[0].(*MulticastEntryDeleteResponseMsg)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// MulticastEntryDelete indicates an expected call of MulticastEntryDelete
func (mr *MockMulticastClientMockRecorder) MulticastEntryDelete(ctx, in interface{}, opts ...interface{}) *gomock.Call {
	varargs := append([]interface{}{ctx, in}, opts...)
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "MulticastEntryDelete", reflect.TypeOf((*MockMulticastClient)(nil).MulticastEntryDelete), varargs...)
}

// MulticastEntryGet mocks base method
func (m *MockMulticastClient) MulticastEntryGet(ctx context.Context, in *MulticastEntryGetRequestMsg, opts ...grpc.CallOption) (*MulticastEntryGetResponseMsg, error) {
	varargs := []interface{}{ctx, in}
	for _, a := range opts {
		varargs = append(varargs, a)
	}
	ret := m.ctrl.Call(m, "MulticastEntryGet", varargs...)
	ret0, _ := ret[0].(*MulticastEntryGetResponseMsg)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// MulticastEntryGet indicates an expected call of MulticastEntryGet
func (mr *MockMulticastClientMockRecorder) MulticastEntryGet(ctx, in interface{}, opts ...interface{}) *gomock.Call {
	varargs := append([]interface{}{ctx, in}, opts...)
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "MulticastEntryGet", reflect.TypeOf((*MockMulticastClient)(nil).MulticastEntryGet), varargs...)
}

// MockMulticastServer is a mock of MulticastServer interface
type MockMulticastServer struct {
	ctrl     *gomock.Controller
	recorder *MockMulticastServerMockRecorder
}

// MockMulticastServerMockRecorder is the mock recorder for MockMulticastServer
type MockMulticastServerMockRecorder struct {
	mock *MockMulticastServer
}

// NewMockMulticastServer creates a new mock instance
func NewMockMulticastServer(ctrl *gomock.Controller) *MockMulticastServer {
	mock := &MockMulticastServer{ctrl: ctrl}
	mock.recorder = &MockMulticastServerMockRecorder{mock}
	return mock
}

// EXPECT returns an object that allows the caller to indicate expected use
func (m *MockMulticastServer) EXPECT() *MockMulticastServerMockRecorder {
	return m.recorder
}

// MulticastEntryCreate mocks base method
func (m *MockMulticastServer) MulticastEntryCreate(arg0 context.Context, arg1 *MulticastEntryRequestMsg) (*MulticastEntryResponseMsg, error) {
	ret := m.ctrl.Call(m, "MulticastEntryCreate", arg0, arg1)
	ret0, _ := ret[0].(*MulticastEntryResponseMsg)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// MulticastEntryCreate indicates an expected call of MulticastEntryCreate
func (mr *MockMulticastServerMockRecorder) MulticastEntryCreate(arg0, arg1 interface{}) *gomock.Call {
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "MulticastEntryCreate", reflect.TypeOf((*MockMulticastServer)(nil).MulticastEntryCreate), arg0, arg1)
}

// MulticastEntryUpdate mocks base method
func (m *MockMulticastServer) MulticastEntryUpdate(arg0 context.Context, arg1 *MulticastEntryRequestMsg) (*MulticastEntryResponseMsg, error) {
	ret := m.ctrl.Call(m, "MulticastEntryUpdate", arg0, arg1)
	ret0, _ := ret[0].(*MulticastEntryResponseMsg)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// MulticastEntryUpdate indicates an expected call of MulticastEntryUpdate
func (mr *MockMulticastServerMockRecorder) MulticastEntryUpdate(arg0, arg1 interface{}) *gomock.Call {
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "MulticastEntryUpdate", reflect.TypeOf((*MockMulticastServer)(nil).MulticastEntryUpdate), arg0, arg1)
}

// MulticastEntryDelete mocks base method
func (m *MockMulticastServer) MulticastEntryDelete(arg0 context.Context, arg1 *MulticastEntryDeleteRequestMsg) (*MulticastEntryDeleteResponseMsg, error) {
	ret := m.ctrl.Call(m, "MulticastEntryDelete", arg0, arg1)
	ret0, _ := ret[0].(*MulticastEntryDeleteResponseMsg)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// MulticastEntryDelete indicates an expected call of MulticastEntryDelete
func (mr *MockMulticastServerMockRecorder) MulticastEntryDelete(arg0, arg1 interface{}) *gomock.Call {
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "MulticastEntryDelete", reflect.TypeOf((*MockMulticastServer)(nil).MulticastEntryDelete), arg0, arg1)
}

// MulticastEntryGet mocks base method
func (m *MockMulticastServer) MulticastEntryGet(arg0 context.Context, arg1 *MulticastEntryGetRequestMsg) (*MulticastEntryGetResponseMsg, error) {
	ret := m.ctrl.Call(m, "MulticastEntryGet", arg0, arg1)
	ret0, _ := ret[0].(*MulticastEntryGetResponseMsg)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// MulticastEntryGet indicates an expected call of MulticastEntryGet
func (mr *MockMulticastServerMockRecorder) MulticastEntryGet(arg0, arg1 interface{}) *gomock.Call {
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "MulticastEntryGet", reflect.TypeOf((*MockMulticastServer)(nil).MulticastEntryGet), arg0, arg1)
}
