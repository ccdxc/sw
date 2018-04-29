// Code generated by MockGen. DO NOT EDIT.
// Source: nwsec.pb.go

package halproto

import (
	reflect "reflect"

	gomock "github.com/golang/mock/gomock"
	context "golang.org/x/net/context"
	grpc "google.golang.org/grpc"
)

// MockisService_L4Info is a mock of isService_L4Info interface
type MockisService_L4Info struct {
	ctrl     *gomock.Controller
	recorder *MockisService_L4InfoMockRecorder
}

// MockisService_L4InfoMockRecorder is the mock recorder for MockisService_L4Info
type MockisService_L4InfoMockRecorder struct {
	mock *MockisService_L4Info
}

// NewMockisService_L4Info creates a new mock instance
func NewMockisService_L4Info(ctrl *gomock.Controller) *MockisService_L4Info {
	mock := &MockisService_L4Info{ctrl: ctrl}
	mock.recorder = &MockisService_L4InfoMockRecorder{mock}
	return mock
}

// EXPECT returns an object that allows the caller to indicate expected use
func (_m *MockisService_L4Info) EXPECT() *MockisService_L4InfoMockRecorder {
	return _m.recorder
}

// isService_L4Info mocks base method
func (_m *MockisService_L4Info) isService_L4Info() {
	_m.ctrl.Call(_m, "isService_L4Info")
}

// isService_L4Info indicates an expected call of isService_L4Info
func (_mr *MockisService_L4InfoMockRecorder) isService_L4Info() *gomock.Call {
	return _mr.mock.ctrl.RecordCallWithMethodType(_mr.mock, "isService_L4Info", reflect.TypeOf((*MockisService_L4Info)(nil).isService_L4Info))
}

// MarshalTo mocks base method
func (_m *MockisService_L4Info) MarshalTo(_param0 []byte) (int, error) {
	ret := _m.ctrl.Call(_m, "MarshalTo", _param0)
	ret0, _ := ret[0].(int)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// MarshalTo indicates an expected call of MarshalTo
func (_mr *MockisService_L4InfoMockRecorder) MarshalTo(arg0 interface{}) *gomock.Call {
	return _mr.mock.ctrl.RecordCallWithMethodType(_mr.mock, "MarshalTo", reflect.TypeOf((*MockisService_L4Info)(nil).MarshalTo), arg0)
}

// Size mocks base method
func (_m *MockisService_L4Info) Size() int {
	ret := _m.ctrl.Call(_m, "Size")
	ret0, _ := ret[0].(int)
	return ret0
}

// Size indicates an expected call of Size
func (_mr *MockisService_L4InfoMockRecorder) Size() *gomock.Call {
	return _mr.mock.ctrl.RecordCallWithMethodType(_mr.mock, "Size", reflect.TypeOf((*MockisService_L4Info)(nil).Size))
}

// MockisAppData_AppOptions is a mock of isAppData_AppOptions interface
type MockisAppData_AppOptions struct {
	ctrl     *gomock.Controller
	recorder *MockisAppData_AppOptionsMockRecorder
}

// MockisAppData_AppOptionsMockRecorder is the mock recorder for MockisAppData_AppOptions
type MockisAppData_AppOptionsMockRecorder struct {
	mock *MockisAppData_AppOptions
}

// NewMockisAppData_AppOptions creates a new mock instance
func NewMockisAppData_AppOptions(ctrl *gomock.Controller) *MockisAppData_AppOptions {
	mock := &MockisAppData_AppOptions{ctrl: ctrl}
	mock.recorder = &MockisAppData_AppOptionsMockRecorder{mock}
	return mock
}

// EXPECT returns an object that allows the caller to indicate expected use
func (_m *MockisAppData_AppOptions) EXPECT() *MockisAppData_AppOptionsMockRecorder {
	return _m.recorder
}

// isAppData_AppOptions mocks base method
func (_m *MockisAppData_AppOptions) isAppData_AppOptions() {
	_m.ctrl.Call(_m, "isAppData_AppOptions")
}

// isAppData_AppOptions indicates an expected call of isAppData_AppOptions
func (_mr *MockisAppData_AppOptionsMockRecorder) isAppData_AppOptions() *gomock.Call {
	return _mr.mock.ctrl.RecordCallWithMethodType(_mr.mock, "isAppData_AppOptions", reflect.TypeOf((*MockisAppData_AppOptions)(nil).isAppData_AppOptions))
}

// MarshalTo mocks base method
func (_m *MockisAppData_AppOptions) MarshalTo(_param0 []byte) (int, error) {
	ret := _m.ctrl.Call(_m, "MarshalTo", _param0)
	ret0, _ := ret[0].(int)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// MarshalTo indicates an expected call of MarshalTo
func (_mr *MockisAppData_AppOptionsMockRecorder) MarshalTo(arg0 interface{}) *gomock.Call {
	return _mr.mock.ctrl.RecordCallWithMethodType(_mr.mock, "MarshalTo", reflect.TypeOf((*MockisAppData_AppOptions)(nil).MarshalTo), arg0)
}

// Size mocks base method
func (_m *MockisAppData_AppOptions) Size() int {
	ret := _m.ctrl.Call(_m, "Size")
	ret0, _ := ret[0].(int)
	return ret0
}

// Size indicates an expected call of Size
func (_mr *MockisAppData_AppOptionsMockRecorder) Size() *gomock.Call {
	return _mr.mock.ctrl.RecordCallWithMethodType(_mr.mock, "Size", reflect.TypeOf((*MockisAppData_AppOptions)(nil).Size))
}

// MockNwSecurityClient is a mock of NwSecurityClient interface
type MockNwSecurityClient struct {
	ctrl     *gomock.Controller
	recorder *MockNwSecurityClientMockRecorder
}

// MockNwSecurityClientMockRecorder is the mock recorder for MockNwSecurityClient
type MockNwSecurityClientMockRecorder struct {
	mock *MockNwSecurityClient
}

// NewMockNwSecurityClient creates a new mock instance
func NewMockNwSecurityClient(ctrl *gomock.Controller) *MockNwSecurityClient {
	mock := &MockNwSecurityClient{ctrl: ctrl}
	mock.recorder = &MockNwSecurityClientMockRecorder{mock}
	return mock
}

// EXPECT returns an object that allows the caller to indicate expected use
func (_m *MockNwSecurityClient) EXPECT() *MockNwSecurityClientMockRecorder {
	return _m.recorder
}

// SecurityProfileCreate mocks base method
func (_m *MockNwSecurityClient) SecurityProfileCreate(ctx context.Context, in *SecurityProfileRequestMsg, opts ...grpc.CallOption) (*SecurityProfileResponseMsg, error) {
	_s := []interface{}{ctx, in}
	for _, _x := range opts {
		_s = append(_s, _x)
	}
	ret := _m.ctrl.Call(_m, "SecurityProfileCreate", _s...)
	ret0, _ := ret[0].(*SecurityProfileResponseMsg)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// SecurityProfileCreate indicates an expected call of SecurityProfileCreate
func (_mr *MockNwSecurityClientMockRecorder) SecurityProfileCreate(arg0, arg1 interface{}, arg2 ...interface{}) *gomock.Call {
	_s := append([]interface{}{arg0, arg1}, arg2...)
	return _mr.mock.ctrl.RecordCallWithMethodType(_mr.mock, "SecurityProfileCreate", reflect.TypeOf((*MockNwSecurityClient)(nil).SecurityProfileCreate), _s...)
}

// SecurityProfileUpdate mocks base method
func (_m *MockNwSecurityClient) SecurityProfileUpdate(ctx context.Context, in *SecurityProfileRequestMsg, opts ...grpc.CallOption) (*SecurityProfileResponseMsg, error) {
	_s := []interface{}{ctx, in}
	for _, _x := range opts {
		_s = append(_s, _x)
	}
	ret := _m.ctrl.Call(_m, "SecurityProfileUpdate", _s...)
	ret0, _ := ret[0].(*SecurityProfileResponseMsg)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// SecurityProfileUpdate indicates an expected call of SecurityProfileUpdate
func (_mr *MockNwSecurityClientMockRecorder) SecurityProfileUpdate(arg0, arg1 interface{}, arg2 ...interface{}) *gomock.Call {
	_s := append([]interface{}{arg0, arg1}, arg2...)
	return _mr.mock.ctrl.RecordCallWithMethodType(_mr.mock, "SecurityProfileUpdate", reflect.TypeOf((*MockNwSecurityClient)(nil).SecurityProfileUpdate), _s...)
}

// SecurityProfileDelete mocks base method
func (_m *MockNwSecurityClient) SecurityProfileDelete(ctx context.Context, in *SecurityProfileDeleteRequestMsg, opts ...grpc.CallOption) (*SecurityProfileDeleteResponseMsg, error) {
	_s := []interface{}{ctx, in}
	for _, _x := range opts {
		_s = append(_s, _x)
	}
	ret := _m.ctrl.Call(_m, "SecurityProfileDelete", _s...)
	ret0, _ := ret[0].(*SecurityProfileDeleteResponseMsg)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// SecurityProfileDelete indicates an expected call of SecurityProfileDelete
func (_mr *MockNwSecurityClientMockRecorder) SecurityProfileDelete(arg0, arg1 interface{}, arg2 ...interface{}) *gomock.Call {
	_s := append([]interface{}{arg0, arg1}, arg2...)
	return _mr.mock.ctrl.RecordCallWithMethodType(_mr.mock, "SecurityProfileDelete", reflect.TypeOf((*MockNwSecurityClient)(nil).SecurityProfileDelete), _s...)
}

// SecurityProfileGet mocks base method
func (_m *MockNwSecurityClient) SecurityProfileGet(ctx context.Context, in *SecurityProfileGetRequestMsg, opts ...grpc.CallOption) (*SecurityProfileGetResponseMsg, error) {
	_s := []interface{}{ctx, in}
	for _, _x := range opts {
		_s = append(_s, _x)
	}
	ret := _m.ctrl.Call(_m, "SecurityProfileGet", _s...)
	ret0, _ := ret[0].(*SecurityProfileGetResponseMsg)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// SecurityProfileGet indicates an expected call of SecurityProfileGet
func (_mr *MockNwSecurityClientMockRecorder) SecurityProfileGet(arg0, arg1 interface{}, arg2 ...interface{}) *gomock.Call {
	_s := append([]interface{}{arg0, arg1}, arg2...)
	return _mr.mock.ctrl.RecordCallWithMethodType(_mr.mock, "SecurityProfileGet", reflect.TypeOf((*MockNwSecurityClient)(nil).SecurityProfileGet), _s...)
}

// SecurityGroupPolicyCreate mocks base method
func (_m *MockNwSecurityClient) SecurityGroupPolicyCreate(ctx context.Context, in *SecurityGroupPolicyRequestMsg, opts ...grpc.CallOption) (*SecurityGroupPolicyResponseMsg, error) {
	_s := []interface{}{ctx, in}
	for _, _x := range opts {
		_s = append(_s, _x)
	}
	ret := _m.ctrl.Call(_m, "SecurityGroupPolicyCreate", _s...)
	ret0, _ := ret[0].(*SecurityGroupPolicyResponseMsg)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// SecurityGroupPolicyCreate indicates an expected call of SecurityGroupPolicyCreate
func (_mr *MockNwSecurityClientMockRecorder) SecurityGroupPolicyCreate(arg0, arg1 interface{}, arg2 ...interface{}) *gomock.Call {
	_s := append([]interface{}{arg0, arg1}, arg2...)
	return _mr.mock.ctrl.RecordCallWithMethodType(_mr.mock, "SecurityGroupPolicyCreate", reflect.TypeOf((*MockNwSecurityClient)(nil).SecurityGroupPolicyCreate), _s...)
}

// SecurityGroupPolicyUpdate mocks base method
func (_m *MockNwSecurityClient) SecurityGroupPolicyUpdate(ctx context.Context, in *SecurityGroupPolicyRequestMsg, opts ...grpc.CallOption) (*SecurityGroupPolicyResponseMsg, error) {
	_s := []interface{}{ctx, in}
	for _, _x := range opts {
		_s = append(_s, _x)
	}
	ret := _m.ctrl.Call(_m, "SecurityGroupPolicyUpdate", _s...)
	ret0, _ := ret[0].(*SecurityGroupPolicyResponseMsg)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// SecurityGroupPolicyUpdate indicates an expected call of SecurityGroupPolicyUpdate
func (_mr *MockNwSecurityClientMockRecorder) SecurityGroupPolicyUpdate(arg0, arg1 interface{}, arg2 ...interface{}) *gomock.Call {
	_s := append([]interface{}{arg0, arg1}, arg2...)
	return _mr.mock.ctrl.RecordCallWithMethodType(_mr.mock, "SecurityGroupPolicyUpdate", reflect.TypeOf((*MockNwSecurityClient)(nil).SecurityGroupPolicyUpdate), _s...)
}

// SecurityGroupPolicyDelete mocks base method
func (_m *MockNwSecurityClient) SecurityGroupPolicyDelete(ctx context.Context, in *SecurityGroupPolicyDeleteRequestMsg, opts ...grpc.CallOption) (*SecurityGroupPolicyDeleteResponseMsg, error) {
	_s := []interface{}{ctx, in}
	for _, _x := range opts {
		_s = append(_s, _x)
	}
	ret := _m.ctrl.Call(_m, "SecurityGroupPolicyDelete", _s...)
	ret0, _ := ret[0].(*SecurityGroupPolicyDeleteResponseMsg)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// SecurityGroupPolicyDelete indicates an expected call of SecurityGroupPolicyDelete
func (_mr *MockNwSecurityClientMockRecorder) SecurityGroupPolicyDelete(arg0, arg1 interface{}, arg2 ...interface{}) *gomock.Call {
	_s := append([]interface{}{arg0, arg1}, arg2...)
	return _mr.mock.ctrl.RecordCallWithMethodType(_mr.mock, "SecurityGroupPolicyDelete", reflect.TypeOf((*MockNwSecurityClient)(nil).SecurityGroupPolicyDelete), _s...)
}

// SecurityGroupPolicyGet mocks base method
func (_m *MockNwSecurityClient) SecurityGroupPolicyGet(ctx context.Context, in *SecurityGroupPolicyGetRequestMsg, opts ...grpc.CallOption) (*SecurityGroupPolicyGetResponseMsg, error) {
	_s := []interface{}{ctx, in}
	for _, _x := range opts {
		_s = append(_s, _x)
	}
	ret := _m.ctrl.Call(_m, "SecurityGroupPolicyGet", _s...)
	ret0, _ := ret[0].(*SecurityGroupPolicyGetResponseMsg)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// SecurityGroupPolicyGet indicates an expected call of SecurityGroupPolicyGet
func (_mr *MockNwSecurityClientMockRecorder) SecurityGroupPolicyGet(arg0, arg1 interface{}, arg2 ...interface{}) *gomock.Call {
	_s := append([]interface{}{arg0, arg1}, arg2...)
	return _mr.mock.ctrl.RecordCallWithMethodType(_mr.mock, "SecurityGroupPolicyGet", reflect.TypeOf((*MockNwSecurityClient)(nil).SecurityGroupPolicyGet), _s...)
}

// SecurityGroupCreate mocks base method
func (_m *MockNwSecurityClient) SecurityGroupCreate(ctx context.Context, in *SecurityGroupRequestMsg, opts ...grpc.CallOption) (*SecurityGroupResponseMsg, error) {
	_s := []interface{}{ctx, in}
	for _, _x := range opts {
		_s = append(_s, _x)
	}
	ret := _m.ctrl.Call(_m, "SecurityGroupCreate", _s...)
	ret0, _ := ret[0].(*SecurityGroupResponseMsg)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// SecurityGroupCreate indicates an expected call of SecurityGroupCreate
func (_mr *MockNwSecurityClientMockRecorder) SecurityGroupCreate(arg0, arg1 interface{}, arg2 ...interface{}) *gomock.Call {
	_s := append([]interface{}{arg0, arg1}, arg2...)
	return _mr.mock.ctrl.RecordCallWithMethodType(_mr.mock, "SecurityGroupCreate", reflect.TypeOf((*MockNwSecurityClient)(nil).SecurityGroupCreate), _s...)
}

// SecurityGroupUpdate mocks base method
func (_m *MockNwSecurityClient) SecurityGroupUpdate(ctx context.Context, in *SecurityGroupRequestMsg, opts ...grpc.CallOption) (*SecurityGroupResponseMsg, error) {
	_s := []interface{}{ctx, in}
	for _, _x := range opts {
		_s = append(_s, _x)
	}
	ret := _m.ctrl.Call(_m, "SecurityGroupUpdate", _s...)
	ret0, _ := ret[0].(*SecurityGroupResponseMsg)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// SecurityGroupUpdate indicates an expected call of SecurityGroupUpdate
func (_mr *MockNwSecurityClientMockRecorder) SecurityGroupUpdate(arg0, arg1 interface{}, arg2 ...interface{}) *gomock.Call {
	_s := append([]interface{}{arg0, arg1}, arg2...)
	return _mr.mock.ctrl.RecordCallWithMethodType(_mr.mock, "SecurityGroupUpdate", reflect.TypeOf((*MockNwSecurityClient)(nil).SecurityGroupUpdate), _s...)
}

// SecurityGroupDelete mocks base method
func (_m *MockNwSecurityClient) SecurityGroupDelete(ctx context.Context, in *SecurityGroupDeleteRequestMsg, opts ...grpc.CallOption) (*SecurityGroupDeleteResponseMsg, error) {
	_s := []interface{}{ctx, in}
	for _, _x := range opts {
		_s = append(_s, _x)
	}
	ret := _m.ctrl.Call(_m, "SecurityGroupDelete", _s...)
	ret0, _ := ret[0].(*SecurityGroupDeleteResponseMsg)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// SecurityGroupDelete indicates an expected call of SecurityGroupDelete
func (_mr *MockNwSecurityClientMockRecorder) SecurityGroupDelete(arg0, arg1 interface{}, arg2 ...interface{}) *gomock.Call {
	_s := append([]interface{}{arg0, arg1}, arg2...)
	return _mr.mock.ctrl.RecordCallWithMethodType(_mr.mock, "SecurityGroupDelete", reflect.TypeOf((*MockNwSecurityClient)(nil).SecurityGroupDelete), _s...)
}

// SecurityGroupGet mocks base method
func (_m *MockNwSecurityClient) SecurityGroupGet(ctx context.Context, in *SecurityGroupGetRequestMsg, opts ...grpc.CallOption) (*SecurityGroupGetResponseMsg, error) {
	_s := []interface{}{ctx, in}
	for _, _x := range opts {
		_s = append(_s, _x)
	}
	ret := _m.ctrl.Call(_m, "SecurityGroupGet", _s...)
	ret0, _ := ret[0].(*SecurityGroupGetResponseMsg)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// SecurityGroupGet indicates an expected call of SecurityGroupGet
func (_mr *MockNwSecurityClientMockRecorder) SecurityGroupGet(arg0, arg1 interface{}, arg2 ...interface{}) *gomock.Call {
	_s := append([]interface{}{arg0, arg1}, arg2...)
	return _mr.mock.ctrl.RecordCallWithMethodType(_mr.mock, "SecurityGroupGet", reflect.TypeOf((*MockNwSecurityClient)(nil).SecurityGroupGet), _s...)
}

// SecurityPolicyCreate mocks base method
func (_m *MockNwSecurityClient) SecurityPolicyCreate(ctx context.Context, in *SecurityPolicyRequestMsg, opts ...grpc.CallOption) (*SecurityPolicyResponseMsg, error) {
	_s := []interface{}{ctx, in}
	for _, _x := range opts {
		_s = append(_s, _x)
	}
	ret := _m.ctrl.Call(_m, "SecurityPolicyCreate", _s...)
	ret0, _ := ret[0].(*SecurityPolicyResponseMsg)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// SecurityPolicyCreate indicates an expected call of SecurityPolicyCreate
func (_mr *MockNwSecurityClientMockRecorder) SecurityPolicyCreate(arg0, arg1 interface{}, arg2 ...interface{}) *gomock.Call {
	_s := append([]interface{}{arg0, arg1}, arg2...)
	return _mr.mock.ctrl.RecordCallWithMethodType(_mr.mock, "SecurityPolicyCreate", reflect.TypeOf((*MockNwSecurityClient)(nil).SecurityPolicyCreate), _s...)
}

// SecurityPolicyUpdate mocks base method
func (_m *MockNwSecurityClient) SecurityPolicyUpdate(ctx context.Context, in *SecurityPolicyRequestMsg, opts ...grpc.CallOption) (*SecurityPolicyResponseMsg, error) {
	_s := []interface{}{ctx, in}
	for _, _x := range opts {
		_s = append(_s, _x)
	}
	ret := _m.ctrl.Call(_m, "SecurityPolicyUpdate", _s...)
	ret0, _ := ret[0].(*SecurityPolicyResponseMsg)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// SecurityPolicyUpdate indicates an expected call of SecurityPolicyUpdate
func (_mr *MockNwSecurityClientMockRecorder) SecurityPolicyUpdate(arg0, arg1 interface{}, arg2 ...interface{}) *gomock.Call {
	_s := append([]interface{}{arg0, arg1}, arg2...)
	return _mr.mock.ctrl.RecordCallWithMethodType(_mr.mock, "SecurityPolicyUpdate", reflect.TypeOf((*MockNwSecurityClient)(nil).SecurityPolicyUpdate), _s...)
}

// SecurityPolicyDelete mocks base method
func (_m *MockNwSecurityClient) SecurityPolicyDelete(ctx context.Context, in *SecurityPolicyDeleteRequestMsg, opts ...grpc.CallOption) (*SecurityPolicyDeleteResponseMsg, error) {
	_s := []interface{}{ctx, in}
	for _, _x := range opts {
		_s = append(_s, _x)
	}
	ret := _m.ctrl.Call(_m, "SecurityPolicyDelete", _s...)
	ret0, _ := ret[0].(*SecurityPolicyDeleteResponseMsg)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// SecurityPolicyDelete indicates an expected call of SecurityPolicyDelete
func (_mr *MockNwSecurityClientMockRecorder) SecurityPolicyDelete(arg0, arg1 interface{}, arg2 ...interface{}) *gomock.Call {
	_s := append([]interface{}{arg0, arg1}, arg2...)
	return _mr.mock.ctrl.RecordCallWithMethodType(_mr.mock, "SecurityPolicyDelete", reflect.TypeOf((*MockNwSecurityClient)(nil).SecurityPolicyDelete), _s...)
}

// SecurityPolicyGet mocks base method
func (_m *MockNwSecurityClient) SecurityPolicyGet(ctx context.Context, in *SecurityPolicyGetRequestMsg, opts ...grpc.CallOption) (*SecurityPolicyGetResponseMsg, error) {
	_s := []interface{}{ctx, in}
	for _, _x := range opts {
		_s = append(_s, _x)
	}
	ret := _m.ctrl.Call(_m, "SecurityPolicyGet", _s...)
	ret0, _ := ret[0].(*SecurityPolicyGetResponseMsg)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// SecurityPolicyGet indicates an expected call of SecurityPolicyGet
func (_mr *MockNwSecurityClientMockRecorder) SecurityPolicyGet(arg0, arg1 interface{}, arg2 ...interface{}) *gomock.Call {
	_s := append([]interface{}{arg0, arg1}, arg2...)
	return _mr.mock.ctrl.RecordCallWithMethodType(_mr.mock, "SecurityPolicyGet", reflect.TypeOf((*MockNwSecurityClient)(nil).SecurityPolicyGet), _s...)
}

// MockNwSecurityServer is a mock of NwSecurityServer interface
type MockNwSecurityServer struct {
	ctrl     *gomock.Controller
	recorder *MockNwSecurityServerMockRecorder
}

// MockNwSecurityServerMockRecorder is the mock recorder for MockNwSecurityServer
type MockNwSecurityServerMockRecorder struct {
	mock *MockNwSecurityServer
}

// NewMockNwSecurityServer creates a new mock instance
func NewMockNwSecurityServer(ctrl *gomock.Controller) *MockNwSecurityServer {
	mock := &MockNwSecurityServer{ctrl: ctrl}
	mock.recorder = &MockNwSecurityServerMockRecorder{mock}
	return mock
}

// EXPECT returns an object that allows the caller to indicate expected use
func (_m *MockNwSecurityServer) EXPECT() *MockNwSecurityServerMockRecorder {
	return _m.recorder
}

// SecurityProfileCreate mocks base method
func (_m *MockNwSecurityServer) SecurityProfileCreate(_param0 context.Context, _param1 *SecurityProfileRequestMsg) (*SecurityProfileResponseMsg, error) {
	ret := _m.ctrl.Call(_m, "SecurityProfileCreate", _param0, _param1)
	ret0, _ := ret[0].(*SecurityProfileResponseMsg)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// SecurityProfileCreate indicates an expected call of SecurityProfileCreate
func (_mr *MockNwSecurityServerMockRecorder) SecurityProfileCreate(arg0, arg1 interface{}) *gomock.Call {
	return _mr.mock.ctrl.RecordCallWithMethodType(_mr.mock, "SecurityProfileCreate", reflect.TypeOf((*MockNwSecurityServer)(nil).SecurityProfileCreate), arg0, arg1)
}

// SecurityProfileUpdate mocks base method
func (_m *MockNwSecurityServer) SecurityProfileUpdate(_param0 context.Context, _param1 *SecurityProfileRequestMsg) (*SecurityProfileResponseMsg, error) {
	ret := _m.ctrl.Call(_m, "SecurityProfileUpdate", _param0, _param1)
	ret0, _ := ret[0].(*SecurityProfileResponseMsg)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// SecurityProfileUpdate indicates an expected call of SecurityProfileUpdate
func (_mr *MockNwSecurityServerMockRecorder) SecurityProfileUpdate(arg0, arg1 interface{}) *gomock.Call {
	return _mr.mock.ctrl.RecordCallWithMethodType(_mr.mock, "SecurityProfileUpdate", reflect.TypeOf((*MockNwSecurityServer)(nil).SecurityProfileUpdate), arg0, arg1)
}

// SecurityProfileDelete mocks base method
func (_m *MockNwSecurityServer) SecurityProfileDelete(_param0 context.Context, _param1 *SecurityProfileDeleteRequestMsg) (*SecurityProfileDeleteResponseMsg, error) {
	ret := _m.ctrl.Call(_m, "SecurityProfileDelete", _param0, _param1)
	ret0, _ := ret[0].(*SecurityProfileDeleteResponseMsg)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// SecurityProfileDelete indicates an expected call of SecurityProfileDelete
func (_mr *MockNwSecurityServerMockRecorder) SecurityProfileDelete(arg0, arg1 interface{}) *gomock.Call {
	return _mr.mock.ctrl.RecordCallWithMethodType(_mr.mock, "SecurityProfileDelete", reflect.TypeOf((*MockNwSecurityServer)(nil).SecurityProfileDelete), arg0, arg1)
}

// SecurityProfileGet mocks base method
func (_m *MockNwSecurityServer) SecurityProfileGet(_param0 context.Context, _param1 *SecurityProfileGetRequestMsg) (*SecurityProfileGetResponseMsg, error) {
	ret := _m.ctrl.Call(_m, "SecurityProfileGet", _param0, _param1)
	ret0, _ := ret[0].(*SecurityProfileGetResponseMsg)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// SecurityProfileGet indicates an expected call of SecurityProfileGet
func (_mr *MockNwSecurityServerMockRecorder) SecurityProfileGet(arg0, arg1 interface{}) *gomock.Call {
	return _mr.mock.ctrl.RecordCallWithMethodType(_mr.mock, "SecurityProfileGet", reflect.TypeOf((*MockNwSecurityServer)(nil).SecurityProfileGet), arg0, arg1)
}

// SecurityGroupPolicyCreate mocks base method
func (_m *MockNwSecurityServer) SecurityGroupPolicyCreate(_param0 context.Context, _param1 *SecurityGroupPolicyRequestMsg) (*SecurityGroupPolicyResponseMsg, error) {
	ret := _m.ctrl.Call(_m, "SecurityGroupPolicyCreate", _param0, _param1)
	ret0, _ := ret[0].(*SecurityGroupPolicyResponseMsg)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// SecurityGroupPolicyCreate indicates an expected call of SecurityGroupPolicyCreate
func (_mr *MockNwSecurityServerMockRecorder) SecurityGroupPolicyCreate(arg0, arg1 interface{}) *gomock.Call {
	return _mr.mock.ctrl.RecordCallWithMethodType(_mr.mock, "SecurityGroupPolicyCreate", reflect.TypeOf((*MockNwSecurityServer)(nil).SecurityGroupPolicyCreate), arg0, arg1)
}

// SecurityGroupPolicyUpdate mocks base method
func (_m *MockNwSecurityServer) SecurityGroupPolicyUpdate(_param0 context.Context, _param1 *SecurityGroupPolicyRequestMsg) (*SecurityGroupPolicyResponseMsg, error) {
	ret := _m.ctrl.Call(_m, "SecurityGroupPolicyUpdate", _param0, _param1)
	ret0, _ := ret[0].(*SecurityGroupPolicyResponseMsg)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// SecurityGroupPolicyUpdate indicates an expected call of SecurityGroupPolicyUpdate
func (_mr *MockNwSecurityServerMockRecorder) SecurityGroupPolicyUpdate(arg0, arg1 interface{}) *gomock.Call {
	return _mr.mock.ctrl.RecordCallWithMethodType(_mr.mock, "SecurityGroupPolicyUpdate", reflect.TypeOf((*MockNwSecurityServer)(nil).SecurityGroupPolicyUpdate), arg0, arg1)
}

// SecurityGroupPolicyDelete mocks base method
func (_m *MockNwSecurityServer) SecurityGroupPolicyDelete(_param0 context.Context, _param1 *SecurityGroupPolicyDeleteRequestMsg) (*SecurityGroupPolicyDeleteResponseMsg, error) {
	ret := _m.ctrl.Call(_m, "SecurityGroupPolicyDelete", _param0, _param1)
	ret0, _ := ret[0].(*SecurityGroupPolicyDeleteResponseMsg)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// SecurityGroupPolicyDelete indicates an expected call of SecurityGroupPolicyDelete
func (_mr *MockNwSecurityServerMockRecorder) SecurityGroupPolicyDelete(arg0, arg1 interface{}) *gomock.Call {
	return _mr.mock.ctrl.RecordCallWithMethodType(_mr.mock, "SecurityGroupPolicyDelete", reflect.TypeOf((*MockNwSecurityServer)(nil).SecurityGroupPolicyDelete), arg0, arg1)
}

// SecurityGroupPolicyGet mocks base method
func (_m *MockNwSecurityServer) SecurityGroupPolicyGet(_param0 context.Context, _param1 *SecurityGroupPolicyGetRequestMsg) (*SecurityGroupPolicyGetResponseMsg, error) {
	ret := _m.ctrl.Call(_m, "SecurityGroupPolicyGet", _param0, _param1)
	ret0, _ := ret[0].(*SecurityGroupPolicyGetResponseMsg)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// SecurityGroupPolicyGet indicates an expected call of SecurityGroupPolicyGet
func (_mr *MockNwSecurityServerMockRecorder) SecurityGroupPolicyGet(arg0, arg1 interface{}) *gomock.Call {
	return _mr.mock.ctrl.RecordCallWithMethodType(_mr.mock, "SecurityGroupPolicyGet", reflect.TypeOf((*MockNwSecurityServer)(nil).SecurityGroupPolicyGet), arg0, arg1)
}

// SecurityGroupCreate mocks base method
func (_m *MockNwSecurityServer) SecurityGroupCreate(_param0 context.Context, _param1 *SecurityGroupRequestMsg) (*SecurityGroupResponseMsg, error) {
	ret := _m.ctrl.Call(_m, "SecurityGroupCreate", _param0, _param1)
	ret0, _ := ret[0].(*SecurityGroupResponseMsg)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// SecurityGroupCreate indicates an expected call of SecurityGroupCreate
func (_mr *MockNwSecurityServerMockRecorder) SecurityGroupCreate(arg0, arg1 interface{}) *gomock.Call {
	return _mr.mock.ctrl.RecordCallWithMethodType(_mr.mock, "SecurityGroupCreate", reflect.TypeOf((*MockNwSecurityServer)(nil).SecurityGroupCreate), arg0, arg1)
}

// SecurityGroupUpdate mocks base method
func (_m *MockNwSecurityServer) SecurityGroupUpdate(_param0 context.Context, _param1 *SecurityGroupRequestMsg) (*SecurityGroupResponseMsg, error) {
	ret := _m.ctrl.Call(_m, "SecurityGroupUpdate", _param0, _param1)
	ret0, _ := ret[0].(*SecurityGroupResponseMsg)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// SecurityGroupUpdate indicates an expected call of SecurityGroupUpdate
func (_mr *MockNwSecurityServerMockRecorder) SecurityGroupUpdate(arg0, arg1 interface{}) *gomock.Call {
	return _mr.mock.ctrl.RecordCallWithMethodType(_mr.mock, "SecurityGroupUpdate", reflect.TypeOf((*MockNwSecurityServer)(nil).SecurityGroupUpdate), arg0, arg1)
}

// SecurityGroupDelete mocks base method
func (_m *MockNwSecurityServer) SecurityGroupDelete(_param0 context.Context, _param1 *SecurityGroupDeleteRequestMsg) (*SecurityGroupDeleteResponseMsg, error) {
	ret := _m.ctrl.Call(_m, "SecurityGroupDelete", _param0, _param1)
	ret0, _ := ret[0].(*SecurityGroupDeleteResponseMsg)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// SecurityGroupDelete indicates an expected call of SecurityGroupDelete
func (_mr *MockNwSecurityServerMockRecorder) SecurityGroupDelete(arg0, arg1 interface{}) *gomock.Call {
	return _mr.mock.ctrl.RecordCallWithMethodType(_mr.mock, "SecurityGroupDelete", reflect.TypeOf((*MockNwSecurityServer)(nil).SecurityGroupDelete), arg0, arg1)
}

// SecurityGroupGet mocks base method
func (_m *MockNwSecurityServer) SecurityGroupGet(_param0 context.Context, _param1 *SecurityGroupGetRequestMsg) (*SecurityGroupGetResponseMsg, error) {
	ret := _m.ctrl.Call(_m, "SecurityGroupGet", _param0, _param1)
	ret0, _ := ret[0].(*SecurityGroupGetResponseMsg)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// SecurityGroupGet indicates an expected call of SecurityGroupGet
func (_mr *MockNwSecurityServerMockRecorder) SecurityGroupGet(arg0, arg1 interface{}) *gomock.Call {
	return _mr.mock.ctrl.RecordCallWithMethodType(_mr.mock, "SecurityGroupGet", reflect.TypeOf((*MockNwSecurityServer)(nil).SecurityGroupGet), arg0, arg1)
}

// SecurityPolicyCreate mocks base method
func (_m *MockNwSecurityServer) SecurityPolicyCreate(_param0 context.Context, _param1 *SecurityPolicyRequestMsg) (*SecurityPolicyResponseMsg, error) {
	ret := _m.ctrl.Call(_m, "SecurityPolicyCreate", _param0, _param1)
	ret0, _ := ret[0].(*SecurityPolicyResponseMsg)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// SecurityPolicyCreate indicates an expected call of SecurityPolicyCreate
func (_mr *MockNwSecurityServerMockRecorder) SecurityPolicyCreate(arg0, arg1 interface{}) *gomock.Call {
	return _mr.mock.ctrl.RecordCallWithMethodType(_mr.mock, "SecurityPolicyCreate", reflect.TypeOf((*MockNwSecurityServer)(nil).SecurityPolicyCreate), arg0, arg1)
}

// SecurityPolicyUpdate mocks base method
func (_m *MockNwSecurityServer) SecurityPolicyUpdate(_param0 context.Context, _param1 *SecurityPolicyRequestMsg) (*SecurityPolicyResponseMsg, error) {
	ret := _m.ctrl.Call(_m, "SecurityPolicyUpdate", _param0, _param1)
	ret0, _ := ret[0].(*SecurityPolicyResponseMsg)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// SecurityPolicyUpdate indicates an expected call of SecurityPolicyUpdate
func (_mr *MockNwSecurityServerMockRecorder) SecurityPolicyUpdate(arg0, arg1 interface{}) *gomock.Call {
	return _mr.mock.ctrl.RecordCallWithMethodType(_mr.mock, "SecurityPolicyUpdate", reflect.TypeOf((*MockNwSecurityServer)(nil).SecurityPolicyUpdate), arg0, arg1)
}

// SecurityPolicyDelete mocks base method
func (_m *MockNwSecurityServer) SecurityPolicyDelete(_param0 context.Context, _param1 *SecurityPolicyDeleteRequestMsg) (*SecurityPolicyDeleteResponseMsg, error) {
	ret := _m.ctrl.Call(_m, "SecurityPolicyDelete", _param0, _param1)
	ret0, _ := ret[0].(*SecurityPolicyDeleteResponseMsg)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// SecurityPolicyDelete indicates an expected call of SecurityPolicyDelete
func (_mr *MockNwSecurityServerMockRecorder) SecurityPolicyDelete(arg0, arg1 interface{}) *gomock.Call {
	return _mr.mock.ctrl.RecordCallWithMethodType(_mr.mock, "SecurityPolicyDelete", reflect.TypeOf((*MockNwSecurityServer)(nil).SecurityPolicyDelete), arg0, arg1)
}

// SecurityPolicyGet mocks base method
func (_m *MockNwSecurityServer) SecurityPolicyGet(_param0 context.Context, _param1 *SecurityPolicyGetRequestMsg) (*SecurityPolicyGetResponseMsg, error) {
	ret := _m.ctrl.Call(_m, "SecurityPolicyGet", _param0, _param1)
	ret0, _ := ret[0].(*SecurityPolicyGetResponseMsg)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// SecurityPolicyGet indicates an expected call of SecurityPolicyGet
func (_mr *MockNwSecurityServerMockRecorder) SecurityPolicyGet(arg0, arg1 interface{}) *gomock.Call {
	return _mr.mock.ctrl.RecordCallWithMethodType(_mr.mock, "SecurityPolicyGet", reflect.TypeOf((*MockNwSecurityServer)(nil).SecurityPolicyGet), arg0, arg1)
}
