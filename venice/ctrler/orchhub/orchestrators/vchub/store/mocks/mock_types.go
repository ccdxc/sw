package mocks

import (
	"context"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/bookstore"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/diagnostics"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/api/generated/objstore"
	"github.com/pensando/sw/api/generated/orchestration"
	"github.com/pensando/sw/api/generated/rollout"
	"github.com/pensando/sw/api/generated/security"
	"github.com/pensando/sw/api/generated/staging"
	"github.com/pensando/sw/api/generated/workload"
	"github.com/pensando/sw/venice/utils/kvstore"
)

// MockAPIClient implements the Service interface
type MockAPIClient struct {
	AAuthV1         auth.AuthV1Interface
	ABookstoreV1    bookstore.BookstoreV1Interface
	AClusterV1      cluster.ClusterV1Interface
	ADiagnosticsV1  diagnostics.DiagnosticsV1Interface
	AMonitoringV1   monitoring.MonitoringV1Interface
	ANetworkV1      network.NetworkV1Interface
	AObjstoreV1     objstore.ObjstoreV1Interface
	AOrchestratorV1 orchestration.OrchestratorV1Interface
	ARolloutV1      rollout.RolloutV1Interface
	ASecurityV1     security.SecurityV1Interface
	AStagingV1      staging.StagingV1Interface
	AWorkloadV1     workload.WorkloadV1Interface
}

// Close method
func (a *MockAPIClient) Close() error {
	return nil
}

// AuthV1 method
func (a *MockAPIClient) AuthV1() auth.AuthV1Interface {
	return a.AAuthV1
}

// BookstoreV1 method
func (a *MockAPIClient) BookstoreV1() bookstore.BookstoreV1Interface {
	return a.ABookstoreV1
}

// ClusterV1 method
func (a *MockAPIClient) ClusterV1() cluster.ClusterV1Interface {
	return a.AClusterV1
}

// DiagnosticsV1 method
func (a *MockAPIClient) DiagnosticsV1() diagnostics.DiagnosticsV1Interface {
	return a.ADiagnosticsV1
}

// MonitoringV1 method
func (a *MockAPIClient) MonitoringV1() monitoring.MonitoringV1Interface {
	return a.AMonitoringV1
}

// NetworkV1 method
func (a *MockAPIClient) NetworkV1() network.NetworkV1Interface {
	return a.ANetworkV1
}

// ObjstoreV1 method
func (a *MockAPIClient) ObjstoreV1() objstore.ObjstoreV1Interface {
	return a.AObjstoreV1
}

// OrchestratorV1 method
func (a *MockAPIClient) OrchestratorV1() orchestration.OrchestratorV1Interface {
	return a.AOrchestratorV1
}

// RolloutV1 method
func (a *MockAPIClient) RolloutV1() rollout.RolloutV1Interface {
	return a.ARolloutV1
}

// SecurityV1 method
func (a *MockAPIClient) SecurityV1() security.SecurityV1Interface {
	return a.ASecurityV1
}

// StagingV1 method
func (a *MockAPIClient) StagingV1() staging.StagingV1Interface {
	return a.AStagingV1
}

// WorkloadV1 method
func (a *MockAPIClient) WorkloadV1() workload.WorkloadV1Interface {
	return a.AWorkloadV1
}

// MockClusterV1 instance
type MockClusterV1 struct {
	MHost cluster.ClusterV1HostInterface
}

// Tenant method
func (m MockClusterV1) Tenant() cluster.ClusterV1TenantInterface {
	return nil
}

// Cluster method
func (m MockClusterV1) Cluster() cluster.ClusterV1ClusterInterface {
	return nil
}

// Node method
func (m MockClusterV1) Node() cluster.ClusterV1NodeInterface {
	return nil
}

// Host method
func (m MockClusterV1) Host() cluster.ClusterV1HostInterface {
	return m.MHost
}

// Version method
func (m MockClusterV1) Version() cluster.ClusterV1VersionInterface {
	return nil
}

// DistributedServiceCard method
func (m MockClusterV1) DistributedServiceCard() cluster.ClusterV1DistributedServiceCardInterface {
	return nil
}

// License method
func (m MockClusterV1) License() cluster.ClusterV1LicenseInterface {
	return nil
}

// Watch method
func (m MockClusterV1) Watch(ctx context.Context, options *api.ListWatchOptions) (kvstore.Watcher, error) {
	return nil, nil
}

// MockWorkloadV1 instance
type MockWorkloadV1 struct {
	MWorkload workload.WorkloadV1WorkloadInterface
	MEndpoint workload.WorkloadV1EndpointInterface
}

// Workload method
func (m MockWorkloadV1) Workload() workload.WorkloadV1WorkloadInterface {
	return m.MWorkload
}

// Endpoint method
func (m MockWorkloadV1) Endpoint() workload.WorkloadV1EndpointInterface {
	return m.MEndpoint
}

// Watch method
func (m MockWorkloadV1) Watch(ctx context.Context, options *api.ListWatchOptions) (kvstore.Watcher, error) {
	return nil, nil
}
