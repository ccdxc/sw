// Code generated by MockGen. DO NOT EDIT.
// Source: accel_rgroup.pb.go

// Package halproto is a generated GoMock package.
package halproto

import (
	reflect "reflect"

	gomock "github.com/golang/mock/gomock"
	context "golang.org/x/net/context"
	grpc "google.golang.org/grpc"
)

// MockAccelRGroupClient is a mock of AccelRGroupClient interface
type MockAccelRGroupClient struct {
	ctrl     *gomock.Controller
	recorder *MockAccelRGroupClientMockRecorder
}

// MockAccelRGroupClientMockRecorder is the mock recorder for MockAccelRGroupClient
type MockAccelRGroupClientMockRecorder struct {
	mock *MockAccelRGroupClient
}

// NewMockAccelRGroupClient creates a new mock instance
func NewMockAccelRGroupClient(ctrl *gomock.Controller) *MockAccelRGroupClient {
	mock := &MockAccelRGroupClient{ctrl: ctrl}
	mock.recorder = &MockAccelRGroupClientMockRecorder{mock}
	return mock
}

// EXPECT returns an object that allows the caller to indicate expected use
func (m *MockAccelRGroupClient) EXPECT() *MockAccelRGroupClientMockRecorder {
	return m.recorder
}

// AccelRGroupAdd mocks base method
func (m *MockAccelRGroupClient) AccelRGroupAdd(ctx context.Context, in *AccelRGroupAddRequestMsg, opts ...grpc.CallOption) (*AccelRGroupAddResponseMsg, error) {
	varargs := []interface{}{ctx, in}
	for _, a := range opts {
		varargs = append(varargs, a)
	}
	ret := m.ctrl.Call(m, "AccelRGroupAdd", varargs...)
	ret0, _ := ret[0].(*AccelRGroupAddResponseMsg)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// AccelRGroupAdd indicates an expected call of AccelRGroupAdd
func (mr *MockAccelRGroupClientMockRecorder) AccelRGroupAdd(ctx, in interface{}, opts ...interface{}) *gomock.Call {
	varargs := append([]interface{}{ctx, in}, opts...)
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "AccelRGroupAdd", reflect.TypeOf((*MockAccelRGroupClient)(nil).AccelRGroupAdd), varargs...)
}

// AccelRGroupDel mocks base method
func (m *MockAccelRGroupClient) AccelRGroupDel(ctx context.Context, in *AccelRGroupDelRequestMsg, opts ...grpc.CallOption) (*AccelRGroupDelResponseMsg, error) {
	varargs := []interface{}{ctx, in}
	for _, a := range opts {
		varargs = append(varargs, a)
	}
	ret := m.ctrl.Call(m, "AccelRGroupDel", varargs...)
	ret0, _ := ret[0].(*AccelRGroupDelResponseMsg)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// AccelRGroupDel indicates an expected call of AccelRGroupDel
func (mr *MockAccelRGroupClientMockRecorder) AccelRGroupDel(ctx, in interface{}, opts ...interface{}) *gomock.Call {
	varargs := append([]interface{}{ctx, in}, opts...)
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "AccelRGroupDel", reflect.TypeOf((*MockAccelRGroupClient)(nil).AccelRGroupDel), varargs...)
}

// AccelRGroupRingAdd mocks base method
func (m *MockAccelRGroupClient) AccelRGroupRingAdd(ctx context.Context, in *AccelRGroupRingAddRequestMsg, opts ...grpc.CallOption) (*AccelRGroupRingAddResponseMsg, error) {
	varargs := []interface{}{ctx, in}
	for _, a := range opts {
		varargs = append(varargs, a)
	}
	ret := m.ctrl.Call(m, "AccelRGroupRingAdd", varargs...)
	ret0, _ := ret[0].(*AccelRGroupRingAddResponseMsg)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// AccelRGroupRingAdd indicates an expected call of AccelRGroupRingAdd
func (mr *MockAccelRGroupClientMockRecorder) AccelRGroupRingAdd(ctx, in interface{}, opts ...interface{}) *gomock.Call {
	varargs := append([]interface{}{ctx, in}, opts...)
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "AccelRGroupRingAdd", reflect.TypeOf((*MockAccelRGroupClient)(nil).AccelRGroupRingAdd), varargs...)
}

// AccelRGroupRingDel mocks base method
func (m *MockAccelRGroupClient) AccelRGroupRingDel(ctx context.Context, in *AccelRGroupRingDelRequestMsg, opts ...grpc.CallOption) (*AccelRGroupRingDelResponseMsg, error) {
	varargs := []interface{}{ctx, in}
	for _, a := range opts {
		varargs = append(varargs, a)
	}
	ret := m.ctrl.Call(m, "AccelRGroupRingDel", varargs...)
	ret0, _ := ret[0].(*AccelRGroupRingDelResponseMsg)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// AccelRGroupRingDel indicates an expected call of AccelRGroupRingDel
func (mr *MockAccelRGroupClientMockRecorder) AccelRGroupRingDel(ctx, in interface{}, opts ...interface{}) *gomock.Call {
	varargs := append([]interface{}{ctx, in}, opts...)
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "AccelRGroupRingDel", reflect.TypeOf((*MockAccelRGroupClient)(nil).AccelRGroupRingDel), varargs...)
}

// AccelRGroupResetSet mocks base method
func (m *MockAccelRGroupClient) AccelRGroupResetSet(ctx context.Context, in *AccelRGroupResetSetRequestMsg, opts ...grpc.CallOption) (*AccelRGroupResetSetResponseMsg, error) {
	varargs := []interface{}{ctx, in}
	for _, a := range opts {
		varargs = append(varargs, a)
	}
	ret := m.ctrl.Call(m, "AccelRGroupResetSet", varargs...)
	ret0, _ := ret[0].(*AccelRGroupResetSetResponseMsg)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// AccelRGroupResetSet indicates an expected call of AccelRGroupResetSet
func (mr *MockAccelRGroupClientMockRecorder) AccelRGroupResetSet(ctx, in interface{}, opts ...interface{}) *gomock.Call {
	varargs := append([]interface{}{ctx, in}, opts...)
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "AccelRGroupResetSet", reflect.TypeOf((*MockAccelRGroupClient)(nil).AccelRGroupResetSet), varargs...)
}

// AccelRGroupEnableSet mocks base method
func (m *MockAccelRGroupClient) AccelRGroupEnableSet(ctx context.Context, in *AccelRGroupEnableSetRequestMsg, opts ...grpc.CallOption) (*AccelRGroupEnableSetResponseMsg, error) {
	varargs := []interface{}{ctx, in}
	for _, a := range opts {
		varargs = append(varargs, a)
	}
	ret := m.ctrl.Call(m, "AccelRGroupEnableSet", varargs...)
	ret0, _ := ret[0].(*AccelRGroupEnableSetResponseMsg)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// AccelRGroupEnableSet indicates an expected call of AccelRGroupEnableSet
func (mr *MockAccelRGroupClientMockRecorder) AccelRGroupEnableSet(ctx, in interface{}, opts ...interface{}) *gomock.Call {
	varargs := append([]interface{}{ctx, in}, opts...)
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "AccelRGroupEnableSet", reflect.TypeOf((*MockAccelRGroupClient)(nil).AccelRGroupEnableSet), varargs...)
}

// AccelRGroupPndxSet mocks base method
func (m *MockAccelRGroupClient) AccelRGroupPndxSet(ctx context.Context, in *AccelRGroupPndxSetRequestMsg, opts ...grpc.CallOption) (*AccelRGroupPndxSetResponseMsg, error) {
	varargs := []interface{}{ctx, in}
	for _, a := range opts {
		varargs = append(varargs, a)
	}
	ret := m.ctrl.Call(m, "AccelRGroupPndxSet", varargs...)
	ret0, _ := ret[0].(*AccelRGroupPndxSetResponseMsg)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// AccelRGroupPndxSet indicates an expected call of AccelRGroupPndxSet
func (mr *MockAccelRGroupClientMockRecorder) AccelRGroupPndxSet(ctx, in interface{}, opts ...interface{}) *gomock.Call {
	varargs := append([]interface{}{ctx, in}, opts...)
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "AccelRGroupPndxSet", reflect.TypeOf((*MockAccelRGroupClient)(nil).AccelRGroupPndxSet), varargs...)
}

// AccelRGroupInfoGet mocks base method
func (m *MockAccelRGroupClient) AccelRGroupInfoGet(ctx context.Context, in *AccelRGroupInfoGetRequestMsg, opts ...grpc.CallOption) (*AccelRGroupInfoGetResponseMsg, error) {
	varargs := []interface{}{ctx, in}
	for _, a := range opts {
		varargs = append(varargs, a)
	}
	ret := m.ctrl.Call(m, "AccelRGroupInfoGet", varargs...)
	ret0, _ := ret[0].(*AccelRGroupInfoGetResponseMsg)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// AccelRGroupInfoGet indicates an expected call of AccelRGroupInfoGet
func (mr *MockAccelRGroupClientMockRecorder) AccelRGroupInfoGet(ctx, in interface{}, opts ...interface{}) *gomock.Call {
	varargs := append([]interface{}{ctx, in}, opts...)
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "AccelRGroupInfoGet", reflect.TypeOf((*MockAccelRGroupClient)(nil).AccelRGroupInfoGet), varargs...)
}

// AccelRGroupIndicesGet mocks base method
func (m *MockAccelRGroupClient) AccelRGroupIndicesGet(ctx context.Context, in *AccelRGroupIndicesGetRequestMsg, opts ...grpc.CallOption) (*AccelRGroupIndicesGetResponseMsg, error) {
	varargs := []interface{}{ctx, in}
	for _, a := range opts {
		varargs = append(varargs, a)
	}
	ret := m.ctrl.Call(m, "AccelRGroupIndicesGet", varargs...)
	ret0, _ := ret[0].(*AccelRGroupIndicesGetResponseMsg)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// AccelRGroupIndicesGet indicates an expected call of AccelRGroupIndicesGet
func (mr *MockAccelRGroupClientMockRecorder) AccelRGroupIndicesGet(ctx, in interface{}, opts ...interface{}) *gomock.Call {
	varargs := append([]interface{}{ctx, in}, opts...)
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "AccelRGroupIndicesGet", reflect.TypeOf((*MockAccelRGroupClient)(nil).AccelRGroupIndicesGet), varargs...)
}

// AccelRGroupMetricsGet mocks base method
func (m *MockAccelRGroupClient) AccelRGroupMetricsGet(ctx context.Context, in *AccelRGroupMetricsGetRequestMsg, opts ...grpc.CallOption) (*AccelRGroupMetricsGetResponseMsg, error) {
	varargs := []interface{}{ctx, in}
	for _, a := range opts {
		varargs = append(varargs, a)
	}
	ret := m.ctrl.Call(m, "AccelRGroupMetricsGet", varargs...)
	ret0, _ := ret[0].(*AccelRGroupMetricsGetResponseMsg)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// AccelRGroupMetricsGet indicates an expected call of AccelRGroupMetricsGet
func (mr *MockAccelRGroupClientMockRecorder) AccelRGroupMetricsGet(ctx, in interface{}, opts ...interface{}) *gomock.Call {
	varargs := append([]interface{}{ctx, in}, opts...)
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "AccelRGroupMetricsGet", reflect.TypeOf((*MockAccelRGroupClient)(nil).AccelRGroupMetricsGet), varargs...)
}

// AccelRGroupMiscGet mocks base method
func (m *MockAccelRGroupClient) AccelRGroupMiscGet(ctx context.Context, in *AccelRGroupMiscGetRequestMsg, opts ...grpc.CallOption) (*AccelRGroupMiscGetResponseMsg, error) {
	varargs := []interface{}{ctx, in}
	for _, a := range opts {
		varargs = append(varargs, a)
	}
	ret := m.ctrl.Call(m, "AccelRGroupMiscGet", varargs...)
	ret0, _ := ret[0].(*AccelRGroupMiscGetResponseMsg)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// AccelRGroupMiscGet indicates an expected call of AccelRGroupMiscGet
func (mr *MockAccelRGroupClientMockRecorder) AccelRGroupMiscGet(ctx, in interface{}, opts ...interface{}) *gomock.Call {
	varargs := append([]interface{}{ctx, in}, opts...)
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "AccelRGroupMiscGet", reflect.TypeOf((*MockAccelRGroupClient)(nil).AccelRGroupMiscGet), varargs...)
}

// MockAccelRGroupServer is a mock of AccelRGroupServer interface
type MockAccelRGroupServer struct {
	ctrl     *gomock.Controller
	recorder *MockAccelRGroupServerMockRecorder
}

// MockAccelRGroupServerMockRecorder is the mock recorder for MockAccelRGroupServer
type MockAccelRGroupServerMockRecorder struct {
	mock *MockAccelRGroupServer
}

// NewMockAccelRGroupServer creates a new mock instance
func NewMockAccelRGroupServer(ctrl *gomock.Controller) *MockAccelRGroupServer {
	mock := &MockAccelRGroupServer{ctrl: ctrl}
	mock.recorder = &MockAccelRGroupServerMockRecorder{mock}
	return mock
}

// EXPECT returns an object that allows the caller to indicate expected use
func (m *MockAccelRGroupServer) EXPECT() *MockAccelRGroupServerMockRecorder {
	return m.recorder
}

// AccelRGroupAdd mocks base method
func (m *MockAccelRGroupServer) AccelRGroupAdd(arg0 context.Context, arg1 *AccelRGroupAddRequestMsg) (*AccelRGroupAddResponseMsg, error) {
	ret := m.ctrl.Call(m, "AccelRGroupAdd", arg0, arg1)
	ret0, _ := ret[0].(*AccelRGroupAddResponseMsg)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// AccelRGroupAdd indicates an expected call of AccelRGroupAdd
func (mr *MockAccelRGroupServerMockRecorder) AccelRGroupAdd(arg0, arg1 interface{}) *gomock.Call {
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "AccelRGroupAdd", reflect.TypeOf((*MockAccelRGroupServer)(nil).AccelRGroupAdd), arg0, arg1)
}

// AccelRGroupDel mocks base method
func (m *MockAccelRGroupServer) AccelRGroupDel(arg0 context.Context, arg1 *AccelRGroupDelRequestMsg) (*AccelRGroupDelResponseMsg, error) {
	ret := m.ctrl.Call(m, "AccelRGroupDel", arg0, arg1)
	ret0, _ := ret[0].(*AccelRGroupDelResponseMsg)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// AccelRGroupDel indicates an expected call of AccelRGroupDel
func (mr *MockAccelRGroupServerMockRecorder) AccelRGroupDel(arg0, arg1 interface{}) *gomock.Call {
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "AccelRGroupDel", reflect.TypeOf((*MockAccelRGroupServer)(nil).AccelRGroupDel), arg0, arg1)
}

// AccelRGroupRingAdd mocks base method
func (m *MockAccelRGroupServer) AccelRGroupRingAdd(arg0 context.Context, arg1 *AccelRGroupRingAddRequestMsg) (*AccelRGroupRingAddResponseMsg, error) {
	ret := m.ctrl.Call(m, "AccelRGroupRingAdd", arg0, arg1)
	ret0, _ := ret[0].(*AccelRGroupRingAddResponseMsg)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// AccelRGroupRingAdd indicates an expected call of AccelRGroupRingAdd
func (mr *MockAccelRGroupServerMockRecorder) AccelRGroupRingAdd(arg0, arg1 interface{}) *gomock.Call {
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "AccelRGroupRingAdd", reflect.TypeOf((*MockAccelRGroupServer)(nil).AccelRGroupRingAdd), arg0, arg1)
}

// AccelRGroupRingDel mocks base method
func (m *MockAccelRGroupServer) AccelRGroupRingDel(arg0 context.Context, arg1 *AccelRGroupRingDelRequestMsg) (*AccelRGroupRingDelResponseMsg, error) {
	ret := m.ctrl.Call(m, "AccelRGroupRingDel", arg0, arg1)
	ret0, _ := ret[0].(*AccelRGroupRingDelResponseMsg)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// AccelRGroupRingDel indicates an expected call of AccelRGroupRingDel
func (mr *MockAccelRGroupServerMockRecorder) AccelRGroupRingDel(arg0, arg1 interface{}) *gomock.Call {
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "AccelRGroupRingDel", reflect.TypeOf((*MockAccelRGroupServer)(nil).AccelRGroupRingDel), arg0, arg1)
}

// AccelRGroupResetSet mocks base method
func (m *MockAccelRGroupServer) AccelRGroupResetSet(arg0 context.Context, arg1 *AccelRGroupResetSetRequestMsg) (*AccelRGroupResetSetResponseMsg, error) {
	ret := m.ctrl.Call(m, "AccelRGroupResetSet", arg0, arg1)
	ret0, _ := ret[0].(*AccelRGroupResetSetResponseMsg)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// AccelRGroupResetSet indicates an expected call of AccelRGroupResetSet
func (mr *MockAccelRGroupServerMockRecorder) AccelRGroupResetSet(arg0, arg1 interface{}) *gomock.Call {
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "AccelRGroupResetSet", reflect.TypeOf((*MockAccelRGroupServer)(nil).AccelRGroupResetSet), arg0, arg1)
}

// AccelRGroupEnableSet mocks base method
func (m *MockAccelRGroupServer) AccelRGroupEnableSet(arg0 context.Context, arg1 *AccelRGroupEnableSetRequestMsg) (*AccelRGroupEnableSetResponseMsg, error) {
	ret := m.ctrl.Call(m, "AccelRGroupEnableSet", arg0, arg1)
	ret0, _ := ret[0].(*AccelRGroupEnableSetResponseMsg)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// AccelRGroupEnableSet indicates an expected call of AccelRGroupEnableSet
func (mr *MockAccelRGroupServerMockRecorder) AccelRGroupEnableSet(arg0, arg1 interface{}) *gomock.Call {
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "AccelRGroupEnableSet", reflect.TypeOf((*MockAccelRGroupServer)(nil).AccelRGroupEnableSet), arg0, arg1)
}

// AccelRGroupPndxSet mocks base method
func (m *MockAccelRGroupServer) AccelRGroupPndxSet(arg0 context.Context, arg1 *AccelRGroupPndxSetRequestMsg) (*AccelRGroupPndxSetResponseMsg, error) {
	ret := m.ctrl.Call(m, "AccelRGroupPndxSet", arg0, arg1)
	ret0, _ := ret[0].(*AccelRGroupPndxSetResponseMsg)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// AccelRGroupPndxSet indicates an expected call of AccelRGroupPndxSet
func (mr *MockAccelRGroupServerMockRecorder) AccelRGroupPndxSet(arg0, arg1 interface{}) *gomock.Call {
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "AccelRGroupPndxSet", reflect.TypeOf((*MockAccelRGroupServer)(nil).AccelRGroupPndxSet), arg0, arg1)
}

// AccelRGroupInfoGet mocks base method
func (m *MockAccelRGroupServer) AccelRGroupInfoGet(arg0 context.Context, arg1 *AccelRGroupInfoGetRequestMsg) (*AccelRGroupInfoGetResponseMsg, error) {
	ret := m.ctrl.Call(m, "AccelRGroupInfoGet", arg0, arg1)
	ret0, _ := ret[0].(*AccelRGroupInfoGetResponseMsg)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// AccelRGroupInfoGet indicates an expected call of AccelRGroupInfoGet
func (mr *MockAccelRGroupServerMockRecorder) AccelRGroupInfoGet(arg0, arg1 interface{}) *gomock.Call {
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "AccelRGroupInfoGet", reflect.TypeOf((*MockAccelRGroupServer)(nil).AccelRGroupInfoGet), arg0, arg1)
}

// AccelRGroupIndicesGet mocks base method
func (m *MockAccelRGroupServer) AccelRGroupIndicesGet(arg0 context.Context, arg1 *AccelRGroupIndicesGetRequestMsg) (*AccelRGroupIndicesGetResponseMsg, error) {
	ret := m.ctrl.Call(m, "AccelRGroupIndicesGet", arg0, arg1)
	ret0, _ := ret[0].(*AccelRGroupIndicesGetResponseMsg)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// AccelRGroupIndicesGet indicates an expected call of AccelRGroupIndicesGet
func (mr *MockAccelRGroupServerMockRecorder) AccelRGroupIndicesGet(arg0, arg1 interface{}) *gomock.Call {
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "AccelRGroupIndicesGet", reflect.TypeOf((*MockAccelRGroupServer)(nil).AccelRGroupIndicesGet), arg0, arg1)
}

// AccelRGroupMetricsGet mocks base method
func (m *MockAccelRGroupServer) AccelRGroupMetricsGet(arg0 context.Context, arg1 *AccelRGroupMetricsGetRequestMsg) (*AccelRGroupMetricsGetResponseMsg, error) {
	ret := m.ctrl.Call(m, "AccelRGroupMetricsGet", arg0, arg1)
	ret0, _ := ret[0].(*AccelRGroupMetricsGetResponseMsg)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// AccelRGroupMetricsGet indicates an expected call of AccelRGroupMetricsGet
func (mr *MockAccelRGroupServerMockRecorder) AccelRGroupMetricsGet(arg0, arg1 interface{}) *gomock.Call {
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "AccelRGroupMetricsGet", reflect.TypeOf((*MockAccelRGroupServer)(nil).AccelRGroupMetricsGet), arg0, arg1)
}

// AccelRGroupMiscGet mocks base method
func (m *MockAccelRGroupServer) AccelRGroupMiscGet(arg0 context.Context, arg1 *AccelRGroupMiscGetRequestMsg) (*AccelRGroupMiscGetResponseMsg, error) {
	ret := m.ctrl.Call(m, "AccelRGroupMiscGet", arg0, arg1)
	ret0, _ := ret[0].(*AccelRGroupMiscGetResponseMsg)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// AccelRGroupMiscGet indicates an expected call of AccelRGroupMiscGet
func (mr *MockAccelRGroupServerMockRecorder) AccelRGroupMiscGet(arg0, arg1 interface{}) *gomock.Call {
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "AccelRGroupMiscGet", reflect.TypeOf((*MockAccelRGroupServer)(nil).AccelRGroupMiscGet), arg0, arg1)
}
