// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

/*
Package networkGwService is a auto generated package.
Input file: svc_network.proto
*/
package networkGwService

import (
	"context"
	"net/http"
	"sync"
	"time"

	"github.com/pkg/errors"
	oldcontext "golang.org/x/net/context"
	"google.golang.org/grpc"

	"github.com/pensando/grpc-gateway/runtime"

	"github.com/pensando/sw/api"
	network "github.com/pensando/sw/api/generated/network"
	grpcclient "github.com/pensando/sw/api/generated/network/grpc/client"
	"github.com/pensando/sw/venice/apigw"
	"github.com/pensando/sw/venice/apigw/pkg"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/authz"
	"github.com/pensando/sw/venice/utils/balancer"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
)

// Dummy vars to suppress import errors
var _ api.TypeMeta
var _ authz.Authorizer

type sNetworkV1GwService struct {
	logger     log.Logger
	defSvcProf apigw.ServiceProfile
	svcProf    map[string]apigw.ServiceProfile
}

type adapterNetworkV1 struct {
	conn    *rpckit.RPCClient
	service network.ServiceNetworkV1Client
	gwSvc   *sNetworkV1GwService
	gw      apigw.APIGateway
}

func (a adapterNetworkV1) AutoAddLbPolicy(oldctx oldcontext.Context, t *network.LbPolicy, options ...grpc.CallOption) (*network.LbPolicy, error) {
	// Not using options for now. Will be passed through context as needed.
	ctx := context.Context(oldctx)
	prof, err := a.gwSvc.GetServiceProfile("AutoAddLbPolicy")
	if err != nil {
		return nil, errors.New("unknown service profile")
	}
	oper, kind, tenant, namespace, group, name := apiserver.CreateOper, "LbPolicy", t.Tenant, t.Namespace, "network", t.Name

	op := authz.NewAPIServerOperation(authz.NewResource(tenant, group, kind, namespace, name), oper)
	// XXX-TODO(vishal-j): Replace with utility function from Authz
	ctx = context.WithValue(ctx, "AuthZOper", op)

	fn := func(ctx context.Context, i interface{}) (interface{}, error) {
		in := i.(*network.LbPolicy)
		return a.service.AutoAddLbPolicy(ctx, in)
	}
	ret, err := a.gw.HandleRequest(ctx, t, prof, fn)
	if ret == nil {
		return nil, err
	}
	return ret.(*network.LbPolicy), err
}

func (a adapterNetworkV1) AutoAddNetwork(oldctx oldcontext.Context, t *network.Network, options ...grpc.CallOption) (*network.Network, error) {
	// Not using options for now. Will be passed through context as needed.
	ctx := context.Context(oldctx)
	prof, err := a.gwSvc.GetServiceProfile("AutoAddNetwork")
	if err != nil {
		return nil, errors.New("unknown service profile")
	}
	oper, kind, tenant, namespace, group, name := apiserver.CreateOper, "Network", t.Tenant, t.Namespace, "network", t.Name

	op := authz.NewAPIServerOperation(authz.NewResource(tenant, group, kind, namespace, name), oper)
	// XXX-TODO(vishal-j): Replace with utility function from Authz
	ctx = context.WithValue(ctx, "AuthZOper", op)

	fn := func(ctx context.Context, i interface{}) (interface{}, error) {
		in := i.(*network.Network)
		return a.service.AutoAddNetwork(ctx, in)
	}
	ret, err := a.gw.HandleRequest(ctx, t, prof, fn)
	if ret == nil {
		return nil, err
	}
	return ret.(*network.Network), err
}

func (a adapterNetworkV1) AutoAddService(oldctx oldcontext.Context, t *network.Service, options ...grpc.CallOption) (*network.Service, error) {
	// Not using options for now. Will be passed through context as needed.
	ctx := context.Context(oldctx)
	prof, err := a.gwSvc.GetServiceProfile("AutoAddService")
	if err != nil {
		return nil, errors.New("unknown service profile")
	}
	oper, kind, tenant, namespace, group, name := apiserver.CreateOper, "Service", t.Tenant, t.Namespace, "network", t.Name

	op := authz.NewAPIServerOperation(authz.NewResource(tenant, group, kind, namespace, name), oper)
	// XXX-TODO(vishal-j): Replace with utility function from Authz
	ctx = context.WithValue(ctx, "AuthZOper", op)

	fn := func(ctx context.Context, i interface{}) (interface{}, error) {
		in := i.(*network.Service)
		return a.service.AutoAddService(ctx, in)
	}
	ret, err := a.gw.HandleRequest(ctx, t, prof, fn)
	if ret == nil {
		return nil, err
	}
	return ret.(*network.Service), err
}

func (a adapterNetworkV1) AutoDeleteLbPolicy(oldctx oldcontext.Context, t *network.LbPolicy, options ...grpc.CallOption) (*network.LbPolicy, error) {
	// Not using options for now. Will be passed through context as needed.
	ctx := context.Context(oldctx)
	prof, err := a.gwSvc.GetServiceProfile("AutoDeleteLbPolicy")
	if err != nil {
		return nil, errors.New("unknown service profile")
	}
	oper, kind, tenant, namespace, group, name := apiserver.DeleteOper, "LbPolicy", t.Tenant, t.Namespace, "network", t.Name

	op := authz.NewAPIServerOperation(authz.NewResource(tenant, group, kind, namespace, name), oper)
	// XXX-TODO(vishal-j): Replace with utility function from Authz
	ctx = context.WithValue(ctx, "AuthZOper", op)

	fn := func(ctx context.Context, i interface{}) (interface{}, error) {
		in := i.(*network.LbPolicy)
		return a.service.AutoDeleteLbPolicy(ctx, in)
	}
	ret, err := a.gw.HandleRequest(ctx, t, prof, fn)
	if ret == nil {
		return nil, err
	}
	return ret.(*network.LbPolicy), err
}

func (a adapterNetworkV1) AutoDeleteNetwork(oldctx oldcontext.Context, t *network.Network, options ...grpc.CallOption) (*network.Network, error) {
	// Not using options for now. Will be passed through context as needed.
	ctx := context.Context(oldctx)
	prof, err := a.gwSvc.GetServiceProfile("AutoDeleteNetwork")
	if err != nil {
		return nil, errors.New("unknown service profile")
	}
	oper, kind, tenant, namespace, group, name := apiserver.DeleteOper, "Network", t.Tenant, t.Namespace, "network", t.Name

	op := authz.NewAPIServerOperation(authz.NewResource(tenant, group, kind, namespace, name), oper)
	// XXX-TODO(vishal-j): Replace with utility function from Authz
	ctx = context.WithValue(ctx, "AuthZOper", op)

	fn := func(ctx context.Context, i interface{}) (interface{}, error) {
		in := i.(*network.Network)
		return a.service.AutoDeleteNetwork(ctx, in)
	}
	ret, err := a.gw.HandleRequest(ctx, t, prof, fn)
	if ret == nil {
		return nil, err
	}
	return ret.(*network.Network), err
}

func (a adapterNetworkV1) AutoDeleteService(oldctx oldcontext.Context, t *network.Service, options ...grpc.CallOption) (*network.Service, error) {
	// Not using options for now. Will be passed through context as needed.
	ctx := context.Context(oldctx)
	prof, err := a.gwSvc.GetServiceProfile("AutoDeleteService")
	if err != nil {
		return nil, errors.New("unknown service profile")
	}
	oper, kind, tenant, namespace, group, name := apiserver.DeleteOper, "Service", t.Tenant, t.Namespace, "network", t.Name

	op := authz.NewAPIServerOperation(authz.NewResource(tenant, group, kind, namespace, name), oper)
	// XXX-TODO(vishal-j): Replace with utility function from Authz
	ctx = context.WithValue(ctx, "AuthZOper", op)

	fn := func(ctx context.Context, i interface{}) (interface{}, error) {
		in := i.(*network.Service)
		return a.service.AutoDeleteService(ctx, in)
	}
	ret, err := a.gw.HandleRequest(ctx, t, prof, fn)
	if ret == nil {
		return nil, err
	}
	return ret.(*network.Service), err
}

func (a adapterNetworkV1) AutoGetLbPolicy(oldctx oldcontext.Context, t *network.LbPolicy, options ...grpc.CallOption) (*network.LbPolicy, error) {
	// Not using options for now. Will be passed through context as needed.
	ctx := context.Context(oldctx)
	prof, err := a.gwSvc.GetServiceProfile("AutoGetLbPolicy")
	if err != nil {
		return nil, errors.New("unknown service profile")
	}
	oper, kind, tenant, namespace, group, name := apiserver.GetOper, "LbPolicy", t.Tenant, t.Namespace, "network", t.Name

	op := authz.NewAPIServerOperation(authz.NewResource(tenant, group, kind, namespace, name), oper)
	// XXX-TODO(vishal-j): Replace with utility function from Authz
	ctx = context.WithValue(ctx, "AuthZOper", op)

	fn := func(ctx context.Context, i interface{}) (interface{}, error) {
		in := i.(*network.LbPolicy)
		return a.service.AutoGetLbPolicy(ctx, in)
	}
	ret, err := a.gw.HandleRequest(ctx, t, prof, fn)
	if ret == nil {
		return nil, err
	}
	return ret.(*network.LbPolicy), err
}

func (a adapterNetworkV1) AutoGetNetwork(oldctx oldcontext.Context, t *network.Network, options ...grpc.CallOption) (*network.Network, error) {
	// Not using options for now. Will be passed through context as needed.
	ctx := context.Context(oldctx)
	prof, err := a.gwSvc.GetServiceProfile("AutoGetNetwork")
	if err != nil {
		return nil, errors.New("unknown service profile")
	}
	oper, kind, tenant, namespace, group, name := apiserver.GetOper, "Network", t.Tenant, t.Namespace, "network", t.Name

	op := authz.NewAPIServerOperation(authz.NewResource(tenant, group, kind, namespace, name), oper)
	// XXX-TODO(vishal-j): Replace with utility function from Authz
	ctx = context.WithValue(ctx, "AuthZOper", op)

	fn := func(ctx context.Context, i interface{}) (interface{}, error) {
		in := i.(*network.Network)
		return a.service.AutoGetNetwork(ctx, in)
	}
	ret, err := a.gw.HandleRequest(ctx, t, prof, fn)
	if ret == nil {
		return nil, err
	}
	return ret.(*network.Network), err
}

func (a adapterNetworkV1) AutoGetService(oldctx oldcontext.Context, t *network.Service, options ...grpc.CallOption) (*network.Service, error) {
	// Not using options for now. Will be passed through context as needed.
	ctx := context.Context(oldctx)
	prof, err := a.gwSvc.GetServiceProfile("AutoGetService")
	if err != nil {
		return nil, errors.New("unknown service profile")
	}
	oper, kind, tenant, namespace, group, name := apiserver.GetOper, "Service", t.Tenant, t.Namespace, "network", t.Name

	op := authz.NewAPIServerOperation(authz.NewResource(tenant, group, kind, namespace, name), oper)
	// XXX-TODO(vishal-j): Replace with utility function from Authz
	ctx = context.WithValue(ctx, "AuthZOper", op)

	fn := func(ctx context.Context, i interface{}) (interface{}, error) {
		in := i.(*network.Service)
		return a.service.AutoGetService(ctx, in)
	}
	ret, err := a.gw.HandleRequest(ctx, t, prof, fn)
	if ret == nil {
		return nil, err
	}
	return ret.(*network.Service), err
}

func (a adapterNetworkV1) AutoListLbPolicy(oldctx oldcontext.Context, t *api.ListWatchOptions, options ...grpc.CallOption) (*network.LbPolicyList, error) {
	// Not using options for now. Will be passed through context as needed.
	ctx := context.Context(oldctx)
	prof, err := a.gwSvc.GetServiceProfile("AutoListLbPolicy")
	if err != nil {
		return nil, errors.New("unknown service profile")
	}
	oper, kind, tenant, namespace, group, name := apiserver.ListOper, "LbPolicyList", t.Tenant, t.Namespace, "network", ""

	op := authz.NewAPIServerOperation(authz.NewResource(tenant, group, kind, namespace, name), oper)
	// XXX-TODO(vishal-j): Replace with utility function from Authz
	ctx = context.WithValue(ctx, "AuthZOper", op)

	fn := func(ctx context.Context, i interface{}) (interface{}, error) {
		in := i.(*api.ListWatchOptions)
		return a.service.AutoListLbPolicy(ctx, in)
	}
	ret, err := a.gw.HandleRequest(ctx, t, prof, fn)
	if ret == nil {
		return nil, err
	}
	return ret.(*network.LbPolicyList), err
}

func (a adapterNetworkV1) AutoListNetwork(oldctx oldcontext.Context, t *api.ListWatchOptions, options ...grpc.CallOption) (*network.NetworkList, error) {
	// Not using options for now. Will be passed through context as needed.
	ctx := context.Context(oldctx)
	prof, err := a.gwSvc.GetServiceProfile("AutoListNetwork")
	if err != nil {
		return nil, errors.New("unknown service profile")
	}
	oper, kind, tenant, namespace, group, name := apiserver.ListOper, "NetworkList", t.Tenant, t.Namespace, "network", ""

	op := authz.NewAPIServerOperation(authz.NewResource(tenant, group, kind, namespace, name), oper)
	// XXX-TODO(vishal-j): Replace with utility function from Authz
	ctx = context.WithValue(ctx, "AuthZOper", op)

	fn := func(ctx context.Context, i interface{}) (interface{}, error) {
		in := i.(*api.ListWatchOptions)
		return a.service.AutoListNetwork(ctx, in)
	}
	ret, err := a.gw.HandleRequest(ctx, t, prof, fn)
	if ret == nil {
		return nil, err
	}
	return ret.(*network.NetworkList), err
}

func (a adapterNetworkV1) AutoListService(oldctx oldcontext.Context, t *api.ListWatchOptions, options ...grpc.CallOption) (*network.ServiceList, error) {
	// Not using options for now. Will be passed through context as needed.
	ctx := context.Context(oldctx)
	prof, err := a.gwSvc.GetServiceProfile("AutoListService")
	if err != nil {
		return nil, errors.New("unknown service profile")
	}
	oper, kind, tenant, namespace, group, name := apiserver.ListOper, "ServiceList", t.Tenant, t.Namespace, "network", ""

	op := authz.NewAPIServerOperation(authz.NewResource(tenant, group, kind, namespace, name), oper)
	// XXX-TODO(vishal-j): Replace with utility function from Authz
	ctx = context.WithValue(ctx, "AuthZOper", op)

	fn := func(ctx context.Context, i interface{}) (interface{}, error) {
		in := i.(*api.ListWatchOptions)
		return a.service.AutoListService(ctx, in)
	}
	ret, err := a.gw.HandleRequest(ctx, t, prof, fn)
	if ret == nil {
		return nil, err
	}
	return ret.(*network.ServiceList), err
}

func (a adapterNetworkV1) AutoUpdateLbPolicy(oldctx oldcontext.Context, t *network.LbPolicy, options ...grpc.CallOption) (*network.LbPolicy, error) {
	// Not using options for now. Will be passed through context as needed.
	ctx := context.Context(oldctx)
	prof, err := a.gwSvc.GetServiceProfile("AutoUpdateLbPolicy")
	if err != nil {
		return nil, errors.New("unknown service profile")
	}
	oper, kind, tenant, namespace, group, name := apiserver.UpdateOper, "LbPolicy", t.Tenant, t.Namespace, "network", t.Name

	op := authz.NewAPIServerOperation(authz.NewResource(tenant, group, kind, namespace, name), oper)
	// XXX-TODO(vishal-j): Replace with utility function from Authz
	ctx = context.WithValue(ctx, "AuthZOper", op)

	fn := func(ctx context.Context, i interface{}) (interface{}, error) {
		in := i.(*network.LbPolicy)
		return a.service.AutoUpdateLbPolicy(ctx, in)
	}
	ret, err := a.gw.HandleRequest(ctx, t, prof, fn)
	if ret == nil {
		return nil, err
	}
	return ret.(*network.LbPolicy), err
}

func (a adapterNetworkV1) AutoUpdateNetwork(oldctx oldcontext.Context, t *network.Network, options ...grpc.CallOption) (*network.Network, error) {
	// Not using options for now. Will be passed through context as needed.
	ctx := context.Context(oldctx)
	prof, err := a.gwSvc.GetServiceProfile("AutoUpdateNetwork")
	if err != nil {
		return nil, errors.New("unknown service profile")
	}
	oper, kind, tenant, namespace, group, name := apiserver.UpdateOper, "Network", t.Tenant, t.Namespace, "network", t.Name

	op := authz.NewAPIServerOperation(authz.NewResource(tenant, group, kind, namespace, name), oper)
	// XXX-TODO(vishal-j): Replace with utility function from Authz
	ctx = context.WithValue(ctx, "AuthZOper", op)

	fn := func(ctx context.Context, i interface{}) (interface{}, error) {
		in := i.(*network.Network)
		return a.service.AutoUpdateNetwork(ctx, in)
	}
	ret, err := a.gw.HandleRequest(ctx, t, prof, fn)
	if ret == nil {
		return nil, err
	}
	return ret.(*network.Network), err
}

func (a adapterNetworkV1) AutoUpdateService(oldctx oldcontext.Context, t *network.Service, options ...grpc.CallOption) (*network.Service, error) {
	// Not using options for now. Will be passed through context as needed.
	ctx := context.Context(oldctx)
	prof, err := a.gwSvc.GetServiceProfile("AutoUpdateService")
	if err != nil {
		return nil, errors.New("unknown service profile")
	}
	oper, kind, tenant, namespace, group, name := apiserver.UpdateOper, "Service", t.Tenant, t.Namespace, "network", t.Name

	op := authz.NewAPIServerOperation(authz.NewResource(tenant, group, kind, namespace, name), oper)
	// XXX-TODO(vishal-j): Replace with utility function from Authz
	ctx = context.WithValue(ctx, "AuthZOper", op)

	fn := func(ctx context.Context, i interface{}) (interface{}, error) {
		in := i.(*network.Service)
		return a.service.AutoUpdateService(ctx, in)
	}
	ret, err := a.gw.HandleRequest(ctx, t, prof, fn)
	if ret == nil {
		return nil, err
	}
	return ret.(*network.Service), err
}

func (a adapterNetworkV1) AutoWatchSvcNetworkV1(oldctx oldcontext.Context, in *api.ListWatchOptions, options ...grpc.CallOption) (network.NetworkV1_AutoWatchSvcNetworkV1Client, error) {
	ctx := context.Context(oldctx)
	prof, err := a.gwSvc.GetServiceProfile("AutoWatchSvcNetworkV1")
	if err != nil {
		return nil, errors.New("unknown service profile")
	}
	oper, kind, tenant, namespace, group := apiserver.WatchOper, "", in.Tenant, in.Namespace, "network"
	op := authz.NewAPIServerOperation(authz.NewResource(tenant, group, kind, namespace, ""), oper)
	// XXX-TODO(vishal-j): Replace with utility function from Authz
	ctx = context.WithValue(ctx, "AuthZOper", op)
	fn := func(ctx context.Context, i interface{}) (interface{}, error) {
		in := i.(*api.ListWatchOptions)
		return a.service.AutoWatchSvcNetworkV1(ctx, in)
	}
	ret, err := a.gw.HandleRequest(ctx, in, prof, fn)
	if ret == nil {
		return nil, err
	}
	return ret.(network.NetworkV1_AutoWatchSvcNetworkV1Client), err
}

func (a adapterNetworkV1) AutoWatchNetwork(oldctx oldcontext.Context, in *api.ListWatchOptions, options ...grpc.CallOption) (network.NetworkV1_AutoWatchNetworkClient, error) {
	ctx := context.Context(oldctx)
	prof, err := a.gwSvc.GetServiceProfile("AutoWatchNetwork")
	if err != nil {
		return nil, errors.New("unknown service profile")
	}
	oper, kind, tenant, namespace, group := apiserver.WatchOper, "Network", in.Tenant, in.Namespace, "network"
	op := authz.NewAPIServerOperation(authz.NewResource(tenant, group, kind, namespace, ""), oper)
	// XXX-TODO(vishal-j): Replace with utility function from Authz
	ctx = context.WithValue(ctx, "AuthZOper", op)
	fn := func(ctx context.Context, i interface{}) (interface{}, error) {
		in := i.(*api.ListWatchOptions)
		return a.service.AutoWatchNetwork(ctx, in)
	}
	ret, err := a.gw.HandleRequest(ctx, in, prof, fn)
	if ret == nil {
		return nil, err
	}
	return ret.(network.NetworkV1_AutoWatchNetworkClient), err
}

func (a adapterNetworkV1) AutoWatchService(oldctx oldcontext.Context, in *api.ListWatchOptions, options ...grpc.CallOption) (network.NetworkV1_AutoWatchServiceClient, error) {
	ctx := context.Context(oldctx)
	prof, err := a.gwSvc.GetServiceProfile("AutoWatchService")
	if err != nil {
		return nil, errors.New("unknown service profile")
	}
	oper, kind, tenant, namespace, group := apiserver.WatchOper, "Service", in.Tenant, in.Namespace, "network"
	op := authz.NewAPIServerOperation(authz.NewResource(tenant, group, kind, namespace, ""), oper)
	// XXX-TODO(vishal-j): Replace with utility function from Authz
	ctx = context.WithValue(ctx, "AuthZOper", op)
	fn := func(ctx context.Context, i interface{}) (interface{}, error) {
		in := i.(*api.ListWatchOptions)
		return a.service.AutoWatchService(ctx, in)
	}
	ret, err := a.gw.HandleRequest(ctx, in, prof, fn)
	if ret == nil {
		return nil, err
	}
	return ret.(network.NetworkV1_AutoWatchServiceClient), err
}

func (a adapterNetworkV1) AutoWatchLbPolicy(oldctx oldcontext.Context, in *api.ListWatchOptions, options ...grpc.CallOption) (network.NetworkV1_AutoWatchLbPolicyClient, error) {
	ctx := context.Context(oldctx)
	prof, err := a.gwSvc.GetServiceProfile("AutoWatchLbPolicy")
	if err != nil {
		return nil, errors.New("unknown service profile")
	}
	oper, kind, tenant, namespace, group := apiserver.WatchOper, "LbPolicy", in.Tenant, in.Namespace, "network"
	op := authz.NewAPIServerOperation(authz.NewResource(tenant, group, kind, namespace, ""), oper)
	// XXX-TODO(vishal-j): Replace with utility function from Authz
	ctx = context.WithValue(ctx, "AuthZOper", op)
	fn := func(ctx context.Context, i interface{}) (interface{}, error) {
		in := i.(*api.ListWatchOptions)
		return a.service.AutoWatchLbPolicy(ctx, in)
	}
	ret, err := a.gw.HandleRequest(ctx, in, prof, fn)
	if ret == nil {
		return nil, err
	}
	return ret.(network.NetworkV1_AutoWatchLbPolicyClient), err
}

func (e *sNetworkV1GwService) setupSvcProfile() {
	e.defSvcProf = apigwpkg.NewServiceProfile(nil)
	e.defSvcProf.SetDefaults()
	e.svcProf = make(map[string]apigw.ServiceProfile)

	e.svcProf["AutoAddLbPolicy"] = apigwpkg.NewServiceProfile(e.defSvcProf)
	e.svcProf["AutoAddNetwork"] = apigwpkg.NewServiceProfile(e.defSvcProf)
	e.svcProf["AutoAddService"] = apigwpkg.NewServiceProfile(e.defSvcProf)
	e.svcProf["AutoDeleteLbPolicy"] = apigwpkg.NewServiceProfile(e.defSvcProf)
	e.svcProf["AutoDeleteNetwork"] = apigwpkg.NewServiceProfile(e.defSvcProf)
	e.svcProf["AutoDeleteService"] = apigwpkg.NewServiceProfile(e.defSvcProf)
	e.svcProf["AutoGetLbPolicy"] = apigwpkg.NewServiceProfile(e.defSvcProf)
	e.svcProf["AutoGetNetwork"] = apigwpkg.NewServiceProfile(e.defSvcProf)
	e.svcProf["AutoGetService"] = apigwpkg.NewServiceProfile(e.defSvcProf)
	e.svcProf["AutoListLbPolicy"] = apigwpkg.NewServiceProfile(e.defSvcProf)
	e.svcProf["AutoListNetwork"] = apigwpkg.NewServiceProfile(e.defSvcProf)
	e.svcProf["AutoListService"] = apigwpkg.NewServiceProfile(e.defSvcProf)
	e.svcProf["AutoUpdateLbPolicy"] = apigwpkg.NewServiceProfile(e.defSvcProf)
	e.svcProf["AutoUpdateNetwork"] = apigwpkg.NewServiceProfile(e.defSvcProf)
	e.svcProf["AutoUpdateService"] = apigwpkg.NewServiceProfile(e.defSvcProf)
	e.svcProf["AutoWatchLbPolicy"] = apigwpkg.NewServiceProfile(e.defSvcProf)
	e.svcProf["AutoWatchNetwork"] = apigwpkg.NewServiceProfile(e.defSvcProf)
	e.svcProf["AutoWatchService"] = apigwpkg.NewServiceProfile(e.defSvcProf)
}

// GetDefaultServiceProfile returns the default fallback service profile for this service
func (e *sNetworkV1GwService) GetDefaultServiceProfile() (apigw.ServiceProfile, error) {
	if e.defSvcProf == nil {
		return nil, errors.New("not found")
	}
	return e.defSvcProf, nil
}

// GetServiceProfile returns the service profile for a given method in this service
func (e *sNetworkV1GwService) GetServiceProfile(method string) (apigw.ServiceProfile, error) {
	if ret, ok := e.svcProf[method]; ok {
		return ret, nil
	}
	return nil, errors.New("not found")
}

// GetCrudServiceProfile returns the service profile for a auto generated crud operation
func (e *sNetworkV1GwService) GetCrudServiceProfile(obj string, oper apiserver.APIOperType) (apigw.ServiceProfile, error) {
	name := apiserver.GetCrudServiceName(obj, oper)
	if name != "" {
		return e.GetServiceProfile(name)
	}
	return nil, errors.New("not found")
}

func (e *sNetworkV1GwService) CompleteRegistration(ctx context.Context,
	logger log.Logger,
	grpcserver *grpc.Server,
	m *http.ServeMux,
	rslvr resolver.Interface,
	wg *sync.WaitGroup) error {
	apigw := apigwpkg.MustGetAPIGateway()
	// IP:port destination or service discovery key.
	grpcaddr := "pen-apiserver"
	grpcaddr = apigw.GetAPIServerAddr(grpcaddr)
	e.logger = logger

	marshaller := runtime.JSONBuiltin{}
	opts := runtime.WithMarshalerOption("*", &marshaller)
	muxMutex.Lock()
	if mux == nil {
		mux = runtime.NewServeMux(opts)
	}
	muxMutex.Unlock()
	e.setupSvcProfile()

	err := registerSwaggerDef(m, logger)
	if err != nil {
		logger.ErrorLog("msg", "failed to register swagger spec", "service", "network.NetworkV1", "error", err)
	}
	wg.Add(1)
	go func() {
		defer wg.Done()
		for {
			nctx, cancel := context.WithCancel(ctx)
			cl, err := e.newClient(nctx, grpcaddr, rslvr, apigw.GetDevMode())
			if err == nil {
				muxMutex.Lock()
				err = network.RegisterNetworkV1HandlerWithClient(ctx, mux, cl)
				muxMutex.Unlock()
				if err == nil {
					logger.InfoLog("msg", "registered service network.NetworkV1")
					m.Handle("/configs/network/v1/", http.StripPrefix("/configs/network/v1", mux))
					return
				} else {
					err = errors.Wrap(err, "failed to register")
				}
			} else {
				err = errors.Wrap(err, "failed to create client")
			}
			cancel()
			logger.ErrorLog("msg", "failed to register", "service", "network.NetworkV1", "error", err)
			select {
			case <-ctx.Done():
				return
			case <-time.After(5 * time.Second):
			}
		}
	}()
	return nil
}

func (e *sNetworkV1GwService) newClient(ctx context.Context, grpcAddr string, rslvr resolver.Interface, devmode bool) (*adapterNetworkV1, error) {
	var opts []rpckit.Option
	if rslvr != nil {
		opts = append(opts, rpckit.WithBalancer(balancer.New(rslvr)))
	} else {
		opts = append(opts, rpckit.WithRemoteServerName("pen-apiserver"))
	}

	if !devmode {
		opts = append(opts, rpckit.WithTracerEnabled(false))
		opts = append(opts, rpckit.WithLoggerEnabled(false))
		opts = append(opts, rpckit.WithStatsEnabled(false))
	}

	client, err := rpckit.NewRPCClient(globals.APIGw, grpcAddr, opts...)
	if err != nil {
		return nil, errors.Wrap(err, "create rpc client")
	}

	e.logger.Infof("Connected to GRPC Server %s", grpcAddr)
	defer func() {
		go func() {
			<-ctx.Done()
			if cerr := client.Close(); cerr != nil {
				e.logger.ErrorLog("msg", "Failed to close conn on Done()", "addr", grpcAddr, "error", cerr)
			}
		}()
	}()

	cl := &adapterNetworkV1{conn: client, gw: apigwpkg.MustGetAPIGateway(), gwSvc: e, service: grpcclient.NewNetworkV1Backend(client.ClientConn, e.logger)}
	return cl, nil
}

func init() {

	apigw := apigwpkg.MustGetAPIGateway()

	svcNetworkV1 := sNetworkV1GwService{}
	apigw.Register("network.NetworkV1", "network/", &svcNetworkV1)
}
