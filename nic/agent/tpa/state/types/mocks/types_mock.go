// Code generated by MockGen. DO NOT EDIT.
// Source: ../types.go

// Package mocks is a generated GoMock package.
package mocks

import (
	context "context"
	reflect "reflect"

	gomock "github.com/golang/mock/gomock"

	protos "github.com/pensando/sw/venice/ctrler/tpm/rpcserver/protos"
)

// MockCtrlerIntf is a mock of CtrlerIntf interface
type MockCtrlerIntf struct {
	ctrl     *gomock.Controller
	recorder *MockCtrlerIntfMockRecorder
}

// MockCtrlerIntfMockRecorder is the mock recorder for MockCtrlerIntf
type MockCtrlerIntfMockRecorder struct {
	mock *MockCtrlerIntf
}

// NewMockCtrlerIntf creates a new mock instance
func NewMockCtrlerIntf(ctrl *gomock.Controller) *MockCtrlerIntf {
	mock := &MockCtrlerIntf{ctrl: ctrl}
	mock.recorder = &MockCtrlerIntfMockRecorder{mock}
	return mock
}

// EXPECT returns an object that allows the caller to indicate expected use
func (m *MockCtrlerIntf) EXPECT() *MockCtrlerIntfMockRecorder {
	return m.recorder
}

// CreateFlowExportPolicy mocks base method
func (m *MockCtrlerIntf) CreateFlowExportPolicy(ctx context.Context, p *protos.FlowExportPolicy) error {
	ret := m.ctrl.Call(m, "CreateFlowExportPolicy", ctx, p)
	ret0, _ := ret[0].(error)
	return ret0
}

// CreateFlowExportPolicy indicates an expected call of CreateFlowExportPolicy
func (mr *MockCtrlerIntfMockRecorder) CreateFlowExportPolicy(ctx, p interface{}) *gomock.Call {
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "CreateFlowExportPolicy", reflect.TypeOf((*MockCtrlerIntf)(nil).CreateFlowExportPolicy), ctx, p)
}

// GetFlowExportPolicy mocks base method
func (m *MockCtrlerIntf) GetFlowExportPolicy(tx context.Context, p *protos.FlowExportPolicy) (*protos.FlowExportPolicy, error) {
	ret := m.ctrl.Call(m, "GetFlowExportPolicy", tx, p)
	ret0, _ := ret[0].(*protos.FlowExportPolicy)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// GetFlowExportPolicy indicates an expected call of GetFlowExportPolicy
func (mr *MockCtrlerIntfMockRecorder) GetFlowExportPolicy(tx, p interface{}) *gomock.Call {
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "GetFlowExportPolicy", reflect.TypeOf((*MockCtrlerIntf)(nil).GetFlowExportPolicy), tx, p)
}

// ListFlowExportPolicy mocks base method
func (m *MockCtrlerIntf) ListFlowExportPolicy(tx context.Context) ([]*protos.FlowExportPolicy, error) {
	ret := m.ctrl.Call(m, "ListFlowExportPolicy", tx)
	ret0, _ := ret[0].([]*protos.FlowExportPolicy)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// ListFlowExportPolicy indicates an expected call of ListFlowExportPolicy
func (mr *MockCtrlerIntfMockRecorder) ListFlowExportPolicy(tx interface{}) *gomock.Call {
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "ListFlowExportPolicy", reflect.TypeOf((*MockCtrlerIntf)(nil).ListFlowExportPolicy), tx)
}

// UpdateFlowExportPolicy mocks base method
func (m *MockCtrlerIntf) UpdateFlowExportPolicy(ctx context.Context, p *protos.FlowExportPolicy) error {
	ret := m.ctrl.Call(m, "UpdateFlowExportPolicy", ctx, p)
	ret0, _ := ret[0].(error)
	return ret0
}

// UpdateFlowExportPolicy indicates an expected call of UpdateFlowExportPolicy
func (mr *MockCtrlerIntfMockRecorder) UpdateFlowExportPolicy(ctx, p interface{}) *gomock.Call {
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "UpdateFlowExportPolicy", reflect.TypeOf((*MockCtrlerIntf)(nil).UpdateFlowExportPolicy), ctx, p)
}

// DeleteFlowExportPolicy mocks base method
func (m *MockCtrlerIntf) DeleteFlowExportPolicy(ctx context.Context, p *protos.FlowExportPolicy) error {
	ret := m.ctrl.Call(m, "DeleteFlowExportPolicy", ctx, p)
	ret0, _ := ret[0].(error)
	return ret0
}

// DeleteFlowExportPolicy indicates an expected call of DeleteFlowExportPolicy
func (mr *MockCtrlerIntfMockRecorder) DeleteFlowExportPolicy(ctx, p interface{}) *gomock.Call {
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "DeleteFlowExportPolicy", reflect.TypeOf((*MockCtrlerIntf)(nil).DeleteFlowExportPolicy), ctx, p)
}

// CreateFwLogPolicy mocks base method
func (m *MockCtrlerIntf) CreateFwLogPolicy(ctx context.Context, p *protos.FwlogPolicy) error {
	ret := m.ctrl.Call(m, "CreateFwLogPolicy", ctx, p)
	ret0, _ := ret[0].(error)
	return ret0
}

// CreateFwLogPolicy indicates an expected call of CreateFwLogPolicy
func (mr *MockCtrlerIntfMockRecorder) CreateFwLogPolicy(ctx, p interface{}) *gomock.Call {
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "CreateFwLogPolicy", reflect.TypeOf((*MockCtrlerIntf)(nil).CreateFwLogPolicy), ctx, p)
}

// GetFwLogPolicy mocks base method
func (m *MockCtrlerIntf) GetFwLogPolicy(tx context.Context, p *protos.FwlogPolicy) (*protos.FwlogPolicy, error) {
	ret := m.ctrl.Call(m, "GetFwLogPolicy", tx, p)
	ret0, _ := ret[0].(*protos.FwlogPolicy)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// GetFwLogPolicy indicates an expected call of GetFwLogPolicy
func (mr *MockCtrlerIntfMockRecorder) GetFwLogPolicy(tx, p interface{}) *gomock.Call {
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "GetFwLogPolicy", reflect.TypeOf((*MockCtrlerIntf)(nil).GetFwLogPolicy), tx, p)
}

// ListFwLogPolicy mocks base method
func (m *MockCtrlerIntf) ListFwLogPolicy(tx context.Context) ([]*protos.FwlogPolicy, error) {
	ret := m.ctrl.Call(m, "ListFwLogPolicy", tx)
	ret0, _ := ret[0].([]*protos.FwlogPolicy)
	ret1, _ := ret[1].(error)
	return ret0, ret1
}

// ListFwLogPolicy indicates an expected call of ListFwLogPolicy
func (mr *MockCtrlerIntfMockRecorder) ListFwLogPolicy(tx interface{}) *gomock.Call {
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "ListFwLogPolicy", reflect.TypeOf((*MockCtrlerIntf)(nil).ListFwLogPolicy), tx)
}

// UpdateFwLogPolicy mocks base method
func (m *MockCtrlerIntf) UpdateFwLogPolicy(ctx context.Context, p *protos.FwlogPolicy) error {
	ret := m.ctrl.Call(m, "UpdateFwLogPolicy", ctx, p)
	ret0, _ := ret[0].(error)
	return ret0
}

// UpdateFwLogPolicy indicates an expected call of UpdateFwLogPolicy
func (mr *MockCtrlerIntfMockRecorder) UpdateFwLogPolicy(ctx, p interface{}) *gomock.Call {
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "UpdateFwLogPolicy", reflect.TypeOf((*MockCtrlerIntf)(nil).UpdateFwLogPolicy), ctx, p)
}

// DeleteFwLogPolicy mocks base method
func (m *MockCtrlerIntf) DeleteFwLogPolicy(ctx context.Context, p *protos.FwlogPolicy) error {
	ret := m.ctrl.Call(m, "DeleteFwLogPolicy", ctx, p)
	ret0, _ := ret[0].(error)
	return ret0
}

// DeleteFwLogPolicy indicates an expected call of DeleteFwLogPolicy
func (mr *MockCtrlerIntfMockRecorder) DeleteFwLogPolicy(ctx, p interface{}) *gomock.Call {
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "DeleteFwLogPolicy", reflect.TypeOf((*MockCtrlerIntf)(nil).DeleteFwLogPolicy), ctx, p)
}
