// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

package security

import (
	"context"

	api "github.com/pensando/sw/api"
	apiserver "github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/utils/kvstore"
)

// Dummy vars to suppress unused imports message
var _ context.Context
var _ api.ObjectMeta
var _ kvstore.Interface

// SecurityV1SecurityGroupInterface exposes the CRUD methods for SecurityGroup
type SecurityV1SecurityGroupInterface interface {
	Create(ctx context.Context, in *SecurityGroup) (*SecurityGroup, error)
	Update(ctx context.Context, in *SecurityGroup) (*SecurityGroup, error)
	Get(ctx context.Context, objMeta *api.ObjectMeta) (*SecurityGroup, error)
	Delete(ctx context.Context, objMeta *api.ObjectMeta) (*SecurityGroup, error)
	List(ctx context.Context, options *api.ListWatchOptions) ([]*SecurityGroup, error)
	Watch(ctx context.Context, options *api.ListWatchOptions) (kvstore.Watcher, error)
	Allowed(oper apiserver.APIOperType) bool
}

// SecurityV1SgpolicyInterface exposes the CRUD methods for Sgpolicy
type SecurityV1SgpolicyInterface interface {
	Create(ctx context.Context, in *Sgpolicy) (*Sgpolicy, error)
	Update(ctx context.Context, in *Sgpolicy) (*Sgpolicy, error)
	Get(ctx context.Context, objMeta *api.ObjectMeta) (*Sgpolicy, error)
	Delete(ctx context.Context, objMeta *api.ObjectMeta) (*Sgpolicy, error)
	List(ctx context.Context, options *api.ListWatchOptions) ([]*Sgpolicy, error)
	Watch(ctx context.Context, options *api.ListWatchOptions) (kvstore.Watcher, error)
	Allowed(oper apiserver.APIOperType) bool
}

// SecurityV1AppInterface exposes the CRUD methods for App
type SecurityV1AppInterface interface {
	Create(ctx context.Context, in *App) (*App, error)
	Update(ctx context.Context, in *App) (*App, error)
	Get(ctx context.Context, objMeta *api.ObjectMeta) (*App, error)
	Delete(ctx context.Context, objMeta *api.ObjectMeta) (*App, error)
	List(ctx context.Context, options *api.ListWatchOptions) ([]*App, error)
	Watch(ctx context.Context, options *api.ListWatchOptions) (kvstore.Watcher, error)
	Allowed(oper apiserver.APIOperType) bool
}

// SecurityV1AppUserInterface exposes the CRUD methods for AppUser
type SecurityV1AppUserInterface interface {
	Create(ctx context.Context, in *AppUser) (*AppUser, error)
	Update(ctx context.Context, in *AppUser) (*AppUser, error)
	Get(ctx context.Context, objMeta *api.ObjectMeta) (*AppUser, error)
	Delete(ctx context.Context, objMeta *api.ObjectMeta) (*AppUser, error)
	List(ctx context.Context, options *api.ListWatchOptions) ([]*AppUser, error)
	Watch(ctx context.Context, options *api.ListWatchOptions) (kvstore.Watcher, error)
	Allowed(oper apiserver.APIOperType) bool
}

// SecurityV1AppUserGrpInterface exposes the CRUD methods for AppUserGrp
type SecurityV1AppUserGrpInterface interface {
	Create(ctx context.Context, in *AppUserGrp) (*AppUserGrp, error)
	Update(ctx context.Context, in *AppUserGrp) (*AppUserGrp, error)
	Get(ctx context.Context, objMeta *api.ObjectMeta) (*AppUserGrp, error)
	Delete(ctx context.Context, objMeta *api.ObjectMeta) (*AppUserGrp, error)
	List(ctx context.Context, options *api.ListWatchOptions) ([]*AppUserGrp, error)
	Watch(ctx context.Context, options *api.ListWatchOptions) (kvstore.Watcher, error)
	Allowed(oper apiserver.APIOperType) bool
}

// SecurityV1CertificateInterface exposes the CRUD methods for Certificate
type SecurityV1CertificateInterface interface {
	Create(ctx context.Context, in *Certificate) (*Certificate, error)
	Update(ctx context.Context, in *Certificate) (*Certificate, error)
	Get(ctx context.Context, objMeta *api.ObjectMeta) (*Certificate, error)
	Delete(ctx context.Context, objMeta *api.ObjectMeta) (*Certificate, error)
	List(ctx context.Context, options *api.ListWatchOptions) ([]*Certificate, error)
	Watch(ctx context.Context, options *api.ListWatchOptions) (kvstore.Watcher, error)
	Allowed(oper apiserver.APIOperType) bool
}

// SecurityV1TrafficEncryptionPolicyInterface exposes the CRUD methods for TrafficEncryptionPolicy
type SecurityV1TrafficEncryptionPolicyInterface interface {
	Create(ctx context.Context, in *TrafficEncryptionPolicy) (*TrafficEncryptionPolicy, error)
	Update(ctx context.Context, in *TrafficEncryptionPolicy) (*TrafficEncryptionPolicy, error)
	Get(ctx context.Context, objMeta *api.ObjectMeta) (*TrafficEncryptionPolicy, error)
	Delete(ctx context.Context, objMeta *api.ObjectMeta) (*TrafficEncryptionPolicy, error)
	List(ctx context.Context, options *api.ListWatchOptions) ([]*TrafficEncryptionPolicy, error)
	Watch(ctx context.Context, options *api.ListWatchOptions) (kvstore.Watcher, error)
	Allowed(oper apiserver.APIOperType) bool
}

// SecurityV1Interface exposes objects with CRUD operations allowed by the service
type SecurityV1Interface interface {
	SecurityGroup() SecurityV1SecurityGroupInterface
	Sgpolicy() SecurityV1SgpolicyInterface
	App() SecurityV1AppInterface
	AppUser() SecurityV1AppUserInterface
	AppUserGrp() SecurityV1AppUserGrpInterface
	Certificate() SecurityV1CertificateInterface
	TrafficEncryptionPolicy() SecurityV1TrafficEncryptionPolicyInterface
	Watch(ctx context.Context, options *api.ListWatchOptions) (kvstore.Watcher, error)
}
