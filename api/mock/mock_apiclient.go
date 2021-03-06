// Code generated by MockGen. DO NOT EDIT.
// Source: ../generated/apiclient/client.go

// Package mock is a generated GoMock package.
package mock

import (
	reflect "reflect"

	gomock "github.com/golang/mock/gomock"

	aggwatch "github.com/pensando/sw/api/generated/aggwatch"
	auth "github.com/pensando/sw/api/generated/auth"
	bookstore "github.com/pensando/sw/api/generated/bookstore"
	browser "github.com/pensando/sw/api/generated/browser"
	cluster "github.com/pensando/sw/api/generated/cluster"
	diagnostics "github.com/pensando/sw/api/generated/diagnostics"
	monitoring "github.com/pensando/sw/api/generated/monitoring"
	network "github.com/pensando/sw/api/generated/network"
	objstore "github.com/pensando/sw/api/generated/objstore"
	orchestration "github.com/pensando/sw/api/generated/orchestration"
	rollout "github.com/pensando/sw/api/generated/rollout"
	routing "github.com/pensando/sw/api/generated/routing"
	security "github.com/pensando/sw/api/generated/security"
	staging "github.com/pensando/sw/api/generated/staging"
	workload "github.com/pensando/sw/api/generated/workload"
)

// MockServices is a mock of Services interface
type MockServices struct {
	ctrl     *gomock.Controller
	recorder *MockServicesMockRecorder
}

// MockServicesMockRecorder is the mock recorder for MockServices
type MockServicesMockRecorder struct {
	mock *MockServices
}

// NewMockServices creates a new mock instance
func NewMockServices(ctrl *gomock.Controller) *MockServices {
	mock := &MockServices{ctrl: ctrl}
	mock.recorder = &MockServicesMockRecorder{mock}
	return mock
}

// EXPECT returns an object that allows the caller to indicate expected use
func (m *MockServices) EXPECT() *MockServicesMockRecorder {
	return m.recorder
}

// Close mocks base method
func (m *MockServices) Close() error {
	ret := m.ctrl.Call(m, "Close")
	ret0, _ := ret[0].(error)
	return ret0
}

// Close indicates an expected call of Close
func (mr *MockServicesMockRecorder) Close() *gomock.Call {
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "Close", reflect.TypeOf((*MockServices)(nil).Close))
}

// AggWatchV1 mocks base method
func (m *MockServices) AggWatchV1() aggwatch.AggWatchV1Interface {
	ret := m.ctrl.Call(m, "AggWatchV1")
	ret0, _ := ret[0].(aggwatch.AggWatchV1Interface)
	return ret0
}

// AggWatchV1 indicates an expected call of AggWatchV1
func (mr *MockServicesMockRecorder) AggWatchV1() *gomock.Call {
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "AggWatchV1", reflect.TypeOf((*MockServices)(nil).AggWatchV1))
}

// AuthV1 mocks base method
func (m *MockServices) AuthV1() auth.AuthV1Interface {
	ret := m.ctrl.Call(m, "AuthV1")
	ret0, _ := ret[0].(auth.AuthV1Interface)
	return ret0
}

// AuthV1 indicates an expected call of AuthV1
func (mr *MockServicesMockRecorder) AuthV1() *gomock.Call {
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "AuthV1", reflect.TypeOf((*MockServices)(nil).AuthV1))
}

// BookstoreV1 mocks base method
func (m *MockServices) BookstoreV1() bookstore.BookstoreV1Interface {
	ret := m.ctrl.Call(m, "BookstoreV1")
	ret0, _ := ret[0].(bookstore.BookstoreV1Interface)
	return ret0
}

// BookstoreV1 indicates an expected call of BookstoreV1
func (mr *MockServicesMockRecorder) BookstoreV1() *gomock.Call {
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "BookstoreV1", reflect.TypeOf((*MockServices)(nil).BookstoreV1))
}

// BrowserV1 mocks base method
func (m *MockServices) BrowserV1() browser.BrowserV1Interface {
	ret := m.ctrl.Call(m, "BrowserV1")
	ret0, _ := ret[0].(browser.BrowserV1Interface)
	return ret0
}

// BrowserV1 indicates an expected call of BrowserV1
func (mr *MockServicesMockRecorder) BrowserV1() *gomock.Call {
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "BrowserV1", reflect.TypeOf((*MockServices)(nil).BrowserV1))
}

// ClusterV1 mocks base method
func (m *MockServices) ClusterV1() cluster.ClusterV1Interface {
	ret := m.ctrl.Call(m, "ClusterV1")
	ret0, _ := ret[0].(cluster.ClusterV1Interface)
	return ret0
}

// ClusterV1 indicates an expected call of ClusterV1
func (mr *MockServicesMockRecorder) ClusterV1() *gomock.Call {
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "ClusterV1", reflect.TypeOf((*MockServices)(nil).ClusterV1))
}

// DiagnosticsV1 mocks base method
func (m *MockServices) DiagnosticsV1() diagnostics.DiagnosticsV1Interface {
	ret := m.ctrl.Call(m, "DiagnosticsV1")
	ret0, _ := ret[0].(diagnostics.DiagnosticsV1Interface)
	return ret0
}

// DiagnosticsV1 indicates an expected call of DiagnosticsV1
func (mr *MockServicesMockRecorder) DiagnosticsV1() *gomock.Call {
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "DiagnosticsV1", reflect.TypeOf((*MockServices)(nil).DiagnosticsV1))
}

// MonitoringV1 mocks base method
func (m *MockServices) MonitoringV1() monitoring.MonitoringV1Interface {
	ret := m.ctrl.Call(m, "MonitoringV1")
	ret0, _ := ret[0].(monitoring.MonitoringV1Interface)
	return ret0
}

// MonitoringV1 indicates an expected call of MonitoringV1
func (mr *MockServicesMockRecorder) MonitoringV1() *gomock.Call {
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "MonitoringV1", reflect.TypeOf((*MockServices)(nil).MonitoringV1))
}

// NetworkV1 mocks base method
func (m *MockServices) NetworkV1() network.NetworkV1Interface {
	ret := m.ctrl.Call(m, "NetworkV1")
	ret0, _ := ret[0].(network.NetworkV1Interface)
	return ret0
}

// NetworkV1 indicates an expected call of NetworkV1
func (mr *MockServicesMockRecorder) NetworkV1() *gomock.Call {
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "NetworkV1", reflect.TypeOf((*MockServices)(nil).NetworkV1))
}

// ObjstoreV1 mocks base method
func (m *MockServices) ObjstoreV1() objstore.ObjstoreV1Interface {
	ret := m.ctrl.Call(m, "ObjstoreV1")
	ret0, _ := ret[0].(objstore.ObjstoreV1Interface)
	return ret0
}

// ObjstoreV1 indicates an expected call of ObjstoreV1
func (mr *MockServicesMockRecorder) ObjstoreV1() *gomock.Call {
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "ObjstoreV1", reflect.TypeOf((*MockServices)(nil).ObjstoreV1))
}

// OrchestratorV1 mocks base method
func (m *MockServices) OrchestratorV1() orchestration.OrchestratorV1Interface {
	ret := m.ctrl.Call(m, "OrchestratorV1")
	ret0, _ := ret[0].(orchestration.OrchestratorV1Interface)
	return ret0
}

// OrchestratorV1 indicates an expected call of OrchestratorV1
func (mr *MockServicesMockRecorder) OrchestratorV1() *gomock.Call {
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "OrchestratorV1", reflect.TypeOf((*MockServices)(nil).OrchestratorV1))
}

// RolloutV1 mocks base method
func (m *MockServices) RolloutV1() rollout.RolloutV1Interface {
	ret := m.ctrl.Call(m, "RolloutV1")
	ret0, _ := ret[0].(rollout.RolloutV1Interface)
	return ret0
}

// RolloutV1 indicates an expected call of RolloutV1
func (mr *MockServicesMockRecorder) RolloutV1() *gomock.Call {
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "RolloutV1", reflect.TypeOf((*MockServices)(nil).RolloutV1))
}

// RoutingV1 mocks base method
func (m *MockServices) RoutingV1() routing.RoutingV1Interface {
	ret := m.ctrl.Call(m, "RoutingV1")
	ret0, _ := ret[0].(routing.RoutingV1Interface)
	return ret0
}

// RoutingV1 indicates an expected call of RoutingV1
func (mr *MockServicesMockRecorder) RoutingV1() *gomock.Call {
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "RoutingV1", reflect.TypeOf((*MockServices)(nil).RoutingV1))
}

// SecurityV1 mocks base method
func (m *MockServices) SecurityV1() security.SecurityV1Interface {
	ret := m.ctrl.Call(m, "SecurityV1")
	ret0, _ := ret[0].(security.SecurityV1Interface)
	return ret0
}

// SecurityV1 indicates an expected call of SecurityV1
func (mr *MockServicesMockRecorder) SecurityV1() *gomock.Call {
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "SecurityV1", reflect.TypeOf((*MockServices)(nil).SecurityV1))
}

// StagingV1 mocks base method
func (m *MockServices) StagingV1() staging.StagingV1Interface {
	ret := m.ctrl.Call(m, "StagingV1")
	ret0, _ := ret[0].(staging.StagingV1Interface)
	return ret0
}

// StagingV1 indicates an expected call of StagingV1
func (mr *MockServicesMockRecorder) StagingV1() *gomock.Call {
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "StagingV1", reflect.TypeOf((*MockServices)(nil).StagingV1))
}

// WorkloadV1 mocks base method
func (m *MockServices) WorkloadV1() workload.WorkloadV1Interface {
	ret := m.ctrl.Call(m, "WorkloadV1")
	ret0, _ := ret[0].(workload.WorkloadV1Interface)
	return ret0
}

// WorkloadV1 indicates an expected call of WorkloadV1
func (mr *MockServicesMockRecorder) WorkloadV1() *gomock.Call {
	return mr.mock.ctrl.RecordCallWithMethodType(mr.mock, "WorkloadV1", reflect.TypeOf((*MockServices)(nil).WorkloadV1))
}
