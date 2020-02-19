// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

package cluster

import (
	"context"

	api "github.com/pensando/sw/api"
	"github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/venice/utils/kvstore"
)

// Dummy vars to suppress unused imports message
var _ context.Context
var _ api.ObjectMeta
var _ kvstore.Interface

// ClusterV1ClusterInterface exposes the CRUD methods for Cluster
type ClusterV1ClusterInterface interface {
	Create(ctx context.Context, in *Cluster) (*Cluster, error)
	Update(ctx context.Context, in *Cluster) (*Cluster, error)
	UpdateStatus(ctx context.Context, in *Cluster) (*Cluster, error)
	Label(ctx context.Context, in *api.Label) (*Cluster, error)
	Get(ctx context.Context, objMeta *api.ObjectMeta) (*Cluster, error)
	Delete(ctx context.Context, objMeta *api.ObjectMeta) (*Cluster, error)
	List(ctx context.Context, options *api.ListWatchOptions) ([]*Cluster, error)
	Watch(ctx context.Context, options *api.ListWatchOptions) (kvstore.Watcher, error)
	Allowed(oper apiintf.APIOperType) bool
	AuthBootstrapComplete(ctx context.Context, in *ClusterAuthBootstrapRequest) (*Cluster, error)
	UpdateTLSConfig(ctx context.Context, in *UpdateTLSConfigRequest) (*Cluster, error)
}

// ClusterV1NodeInterface exposes the CRUD methods for Node
type ClusterV1NodeInterface interface {
	Create(ctx context.Context, in *Node) (*Node, error)
	Update(ctx context.Context, in *Node) (*Node, error)
	UpdateStatus(ctx context.Context, in *Node) (*Node, error)
	Label(ctx context.Context, in *api.Label) (*Node, error)
	Get(ctx context.Context, objMeta *api.ObjectMeta) (*Node, error)
	Delete(ctx context.Context, objMeta *api.ObjectMeta) (*Node, error)
	List(ctx context.Context, options *api.ListWatchOptions) ([]*Node, error)
	Watch(ctx context.Context, options *api.ListWatchOptions) (kvstore.Watcher, error)
	Allowed(oper apiintf.APIOperType) bool
}

// ClusterV1HostInterface exposes the CRUD methods for Host
type ClusterV1HostInterface interface {
	Create(ctx context.Context, in *Host) (*Host, error)
	Update(ctx context.Context, in *Host) (*Host, error)
	UpdateStatus(ctx context.Context, in *Host) (*Host, error)
	Label(ctx context.Context, in *api.Label) (*Host, error)
	Get(ctx context.Context, objMeta *api.ObjectMeta) (*Host, error)
	Delete(ctx context.Context, objMeta *api.ObjectMeta) (*Host, error)
	List(ctx context.Context, options *api.ListWatchOptions) ([]*Host, error)
	Watch(ctx context.Context, options *api.ListWatchOptions) (kvstore.Watcher, error)
	Allowed(oper apiintf.APIOperType) bool
}

// ClusterV1DistributedServiceCardInterface exposes the CRUD methods for DistributedServiceCard
type ClusterV1DistributedServiceCardInterface interface {
	Create(ctx context.Context, in *DistributedServiceCard) (*DistributedServiceCard, error)
	Update(ctx context.Context, in *DistributedServiceCard) (*DistributedServiceCard, error)
	UpdateStatus(ctx context.Context, in *DistributedServiceCard) (*DistributedServiceCard, error)
	Label(ctx context.Context, in *api.Label) (*DistributedServiceCard, error)
	Get(ctx context.Context, objMeta *api.ObjectMeta) (*DistributedServiceCard, error)
	Delete(ctx context.Context, objMeta *api.ObjectMeta) (*DistributedServiceCard, error)
	List(ctx context.Context, options *api.ListWatchOptions) ([]*DistributedServiceCard, error)
	Watch(ctx context.Context, options *api.ListWatchOptions) (kvstore.Watcher, error)
	Allowed(oper apiintf.APIOperType) bool
}

// ClusterV1TenantInterface exposes the CRUD methods for Tenant
type ClusterV1TenantInterface interface {
	Create(ctx context.Context, in *Tenant) (*Tenant, error)
	Update(ctx context.Context, in *Tenant) (*Tenant, error)
	UpdateStatus(ctx context.Context, in *Tenant) (*Tenant, error)
	Label(ctx context.Context, in *api.Label) (*Tenant, error)
	Get(ctx context.Context, objMeta *api.ObjectMeta) (*Tenant, error)
	Delete(ctx context.Context, objMeta *api.ObjectMeta) (*Tenant, error)
	List(ctx context.Context, options *api.ListWatchOptions) ([]*Tenant, error)
	Watch(ctx context.Context, options *api.ListWatchOptions) (kvstore.Watcher, error)
	Allowed(oper apiintf.APIOperType) bool
}

// ClusterV1VersionInterface exposes the CRUD methods for Version
type ClusterV1VersionInterface interface {
	Create(ctx context.Context, in *Version) (*Version, error)
	Update(ctx context.Context, in *Version) (*Version, error)
	UpdateStatus(ctx context.Context, in *Version) (*Version, error)
	Label(ctx context.Context, in *api.Label) (*Version, error)
	Get(ctx context.Context, objMeta *api.ObjectMeta) (*Version, error)
	Delete(ctx context.Context, objMeta *api.ObjectMeta) (*Version, error)
	List(ctx context.Context, options *api.ListWatchOptions) ([]*Version, error)
	Watch(ctx context.Context, options *api.ListWatchOptions) (kvstore.Watcher, error)
	Allowed(oper apiintf.APIOperType) bool
}

// ClusterV1ConfigurationSnapshotInterface exposes the CRUD methods for ConfigurationSnapshot
type ClusterV1ConfigurationSnapshotInterface interface {
	Create(ctx context.Context, in *ConfigurationSnapshot) (*ConfigurationSnapshot, error)
	Update(ctx context.Context, in *ConfigurationSnapshot) (*ConfigurationSnapshot, error)
	UpdateStatus(ctx context.Context, in *ConfigurationSnapshot) (*ConfigurationSnapshot, error)
	Label(ctx context.Context, in *api.Label) (*ConfigurationSnapshot, error)
	Get(ctx context.Context, objMeta *api.ObjectMeta) (*ConfigurationSnapshot, error)
	Delete(ctx context.Context, objMeta *api.ObjectMeta) (*ConfigurationSnapshot, error)
	List(ctx context.Context, options *api.ListWatchOptions) ([]*ConfigurationSnapshot, error)
	Watch(ctx context.Context, options *api.ListWatchOptions) (kvstore.Watcher, error)
	Allowed(oper apiintf.APIOperType) bool
	Save(ctx context.Context, in *ConfigurationSnapshotRequest) (*ConfigurationSnapshot, error)
}

// ClusterV1SnapshotRestoreInterface exposes the CRUD methods for SnapshotRestore
type ClusterV1SnapshotRestoreInterface interface {
	Create(ctx context.Context, in *SnapshotRestore) (*SnapshotRestore, error)
	Update(ctx context.Context, in *SnapshotRestore) (*SnapshotRestore, error)
	UpdateStatus(ctx context.Context, in *SnapshotRestore) (*SnapshotRestore, error)
	Label(ctx context.Context, in *api.Label) (*SnapshotRestore, error)
	Get(ctx context.Context, objMeta *api.ObjectMeta) (*SnapshotRestore, error)
	Delete(ctx context.Context, objMeta *api.ObjectMeta) (*SnapshotRestore, error)
	List(ctx context.Context, options *api.ListWatchOptions) ([]*SnapshotRestore, error)
	Watch(ctx context.Context, options *api.ListWatchOptions) (kvstore.Watcher, error)
	Allowed(oper apiintf.APIOperType) bool
	Restore(ctx context.Context, in *SnapshotRestore) (*SnapshotRestore, error)
}

// ClusterV1LicenseInterface exposes the CRUD methods for License
type ClusterV1LicenseInterface interface {
	Create(ctx context.Context, in *License) (*License, error)
	Update(ctx context.Context, in *License) (*License, error)
	UpdateStatus(ctx context.Context, in *License) (*License, error)
	Label(ctx context.Context, in *api.Label) (*License, error)
	Get(ctx context.Context, objMeta *api.ObjectMeta) (*License, error)
	Delete(ctx context.Context, objMeta *api.ObjectMeta) (*License, error)
	List(ctx context.Context, options *api.ListWatchOptions) ([]*License, error)
	Watch(ctx context.Context, options *api.ListWatchOptions) (kvstore.Watcher, error)
	Allowed(oper apiintf.APIOperType) bool
}

// ClusterV1Interface exposes objects with CRUD operations allowed by the service
type ClusterV1Interface interface {
	Cluster() ClusterV1ClusterInterface
	Node() ClusterV1NodeInterface
	Host() ClusterV1HostInterface
	DistributedServiceCard() ClusterV1DistributedServiceCardInterface
	Tenant() ClusterV1TenantInterface
	Version() ClusterV1VersionInterface
	ConfigurationSnapshot() ClusterV1ConfigurationSnapshotInterface
	SnapshotRestore() ClusterV1SnapshotRestoreInterface
	License() ClusterV1LicenseInterface
	Watch(ctx context.Context, options *api.ListWatchOptions) (kvstore.Watcher, error)
}
