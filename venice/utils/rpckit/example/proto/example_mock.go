// Automatically generated by MockGen. DO NOT EDIT!
// Source: example.pb.go

package proto

import (
	gomock "github.com/golang/mock/gomock"
	context "golang.org/x/net/context"
	grpc "google.golang.org/grpc"
)

// Mock of ExampleClient interface
type MockExampleClient struct {
	ctrl     *gomock.Controller
	recorder *_MockExampleClientRecorder
}

// Recorder for MockExampleClient (not exported)
type _MockExampleClientRecorder struct {
	mock *MockExampleClient
}

func NewMockExampleClient(ctrl *gomock.Controller) *MockExampleClient {
	mock := &MockExampleClient{ctrl: ctrl}
	mock.recorder = &_MockExampleClientRecorder{mock}
	return mock
}

func (_m *MockExampleClient) EXPECT() *_MockExampleClientRecorder {
	return _m.recorder
}

func (_m *MockExampleClient) ExampleRPC(ctx context.Context, in *ExampleReq, opts ...grpc.CallOption) (*ExampleResp, error) {
	_s := []interface{}{ctx, in}
	for _, _x := range opts {
		_s = append(_s, _x)
	}
	ret := _m.ctrl.Call(_m, "ExampleRPC", _s...)
	ret0, _ := ret[0].(*ExampleResp)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

func (_mr *_MockExampleClientRecorder) ExampleRPC(arg0, arg1 interface{}, arg2 ...interface{}) *gomock.Call {
	_s := append([]interface{}{arg0, arg1}, arg2...)
	return _mr.mock.ctrl.RecordCall(_mr.mock, "ExampleRPC", _s...)
}

// Mock of ExampleServer interface
type MockExampleServer struct {
	ctrl     *gomock.Controller
	recorder *_MockExampleServerRecorder
}

// Recorder for MockExampleServer (not exported)
type _MockExampleServerRecorder struct {
	mock *MockExampleServer
}

func NewMockExampleServer(ctrl *gomock.Controller) *MockExampleServer {
	mock := &MockExampleServer{ctrl: ctrl}
	mock.recorder = &_MockExampleServerRecorder{mock}
	return mock
}

func (_m *MockExampleServer) EXPECT() *_MockExampleServerRecorder {
	return _m.recorder
}

func (_m *MockExampleServer) ExampleRPC(_param0 context.Context, _param1 *ExampleReq) (*ExampleResp, error) {
	ret := _m.ctrl.Call(_m, "ExampleRPC", _param0, _param1)
	ret0, _ := ret[0].(*ExampleResp)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

func (_mr *_MockExampleServerRecorder) ExampleRPC(arg0, arg1 interface{}) *gomock.Call {
	return _mr.mock.ctrl.RecordCall(_mr.mock, "ExampleRPC", arg0, arg1)
}
