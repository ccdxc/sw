// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

/*
Package networkGwService is a auto generated package.
Input file: svc_network.proto
*/
package networkGwService

import (
	"context"
	"net/http"
	"strings"
	"sync"
	"time"

	"github.com/pkg/errors"
	oldcontext "golang.org/x/net/context"
	"google.golang.org/grpc"

	"github.com/pensando/grpc-gateway/runtime"

	"github.com/pensando/sw/api"
	network "github.com/pensando/sw/api/generated/network"
	grpcclient "github.com/pensando/sw/api/generated/network/grpc/client"
	"github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/api/utils"
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
	oper, kind, tenant, namespace, group, name, auditAction := apiintf.CreateOper, "LbPolicy", t.Tenant, t.Namespace, "network", t.Name, strings.Title(string(apiintf.CreateOper))

	op := authz.NewAPIServerOperation(authz.NewResource(tenant, group, kind, namespace, name), oper, auditAction)
	ctx = apigwpkg.NewContextWithOperations(ctx, op)

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
	oper, kind, tenant, namespace, group, name, auditAction := apiintf.CreateOper, "Network", t.Tenant, t.Namespace, "network", t.Name, strings.Title(string(apiintf.CreateOper))

	op := authz.NewAPIServerOperation(authz.NewResource(tenant, group, kind, namespace, name), oper, auditAction)
	ctx = apigwpkg.NewContextWithOperations(ctx, op)

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
	oper, kind, tenant, namespace, group, name, auditAction := apiintf.CreateOper, "Service", t.Tenant, t.Namespace, "network", t.Name, strings.Title(string(apiintf.CreateOper))

	op := authz.NewAPIServerOperation(authz.NewResource(tenant, group, kind, namespace, name), oper, auditAction)
	ctx = apigwpkg.NewContextWithOperations(ctx, op)

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

func (a adapterNetworkV1) AutoAddVirtualRouter(oldctx oldcontext.Context, t *network.VirtualRouter, options ...grpc.CallOption) (*network.VirtualRouter, error) {
	// Not using options for now. Will be passed through context as needed.
	ctx := context.Context(oldctx)
	prof, err := a.gwSvc.GetServiceProfile("AutoAddVirtualRouter")
	if err != nil {
		return nil, errors.New("unknown service profile")
	}
	oper, kind, tenant, namespace, group, name, auditAction := apiintf.CreateOper, "VirtualRouter", t.Tenant, t.Namespace, "network", t.Name, strings.Title(string(apiintf.CreateOper))

	op := authz.NewAPIServerOperation(authz.NewResource(tenant, group, kind, namespace, name), oper, auditAction)
	ctx = apigwpkg.NewContextWithOperations(ctx, op)

	fn := func(ctx context.Context, i interface{}) (interface{}, error) {
		in := i.(*network.VirtualRouter)
		return a.service.AutoAddVirtualRouter(ctx, in)
	}
	ret, err := a.gw.HandleRequest(ctx, t, prof, fn)
	if ret == nil {
		return nil, err
	}
	return ret.(*network.VirtualRouter), err
}

func (a adapterNetworkV1) AutoDeleteLbPolicy(oldctx oldcontext.Context, t *network.LbPolicy, options ...grpc.CallOption) (*network.LbPolicy, error) {
	// Not using options for now. Will be passed through context as needed.
	ctx := context.Context(oldctx)
	prof, err := a.gwSvc.GetServiceProfile("AutoDeleteLbPolicy")
	if err != nil {
		return nil, errors.New("unknown service profile")
	}
	oper, kind, tenant, namespace, group, name, auditAction := apiintf.DeleteOper, "LbPolicy", t.Tenant, t.Namespace, "network", t.Name, strings.Title(string(apiintf.DeleteOper))

	op := authz.NewAPIServerOperation(authz.NewResource(tenant, group, kind, namespace, name), oper, auditAction)
	ctx = apigwpkg.NewContextWithOperations(ctx, op)

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
	oper, kind, tenant, namespace, group, name, auditAction := apiintf.DeleteOper, "Network", t.Tenant, t.Namespace, "network", t.Name, strings.Title(string(apiintf.DeleteOper))

	op := authz.NewAPIServerOperation(authz.NewResource(tenant, group, kind, namespace, name), oper, auditAction)
	ctx = apigwpkg.NewContextWithOperations(ctx, op)

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
	oper, kind, tenant, namespace, group, name, auditAction := apiintf.DeleteOper, "Service", t.Tenant, t.Namespace, "network", t.Name, strings.Title(string(apiintf.DeleteOper))

	op := authz.NewAPIServerOperation(authz.NewResource(tenant, group, kind, namespace, name), oper, auditAction)
	ctx = apigwpkg.NewContextWithOperations(ctx, op)

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

func (a adapterNetworkV1) AutoDeleteVirtualRouter(oldctx oldcontext.Context, t *network.VirtualRouter, options ...grpc.CallOption) (*network.VirtualRouter, error) {
	// Not using options for now. Will be passed through context as needed.
	ctx := context.Context(oldctx)
	prof, err := a.gwSvc.GetServiceProfile("AutoDeleteVirtualRouter")
	if err != nil {
		return nil, errors.New("unknown service profile")
	}
	oper, kind, tenant, namespace, group, name, auditAction := apiintf.DeleteOper, "VirtualRouter", t.Tenant, t.Namespace, "network", t.Name, strings.Title(string(apiintf.DeleteOper))

	op := authz.NewAPIServerOperation(authz.NewResource(tenant, group, kind, namespace, name), oper, auditAction)
	ctx = apigwpkg.NewContextWithOperations(ctx, op)

	fn := func(ctx context.Context, i interface{}) (interface{}, error) {
		in := i.(*network.VirtualRouter)
		return a.service.AutoDeleteVirtualRouter(ctx, in)
	}
	ret, err := a.gw.HandleRequest(ctx, t, prof, fn)
	if ret == nil {
		return nil, err
	}
	return ret.(*network.VirtualRouter), err
}

func (a adapterNetworkV1) AutoGetLbPolicy(oldctx oldcontext.Context, t *network.LbPolicy, options ...grpc.CallOption) (*network.LbPolicy, error) {
	// Not using options for now. Will be passed through context as needed.
	ctx := context.Context(oldctx)
	prof, err := a.gwSvc.GetServiceProfile("AutoGetLbPolicy")
	if err != nil {
		return nil, errors.New("unknown service profile")
	}
	oper, kind, tenant, namespace, group, name, auditAction := apiintf.GetOper, "LbPolicy", t.Tenant, t.Namespace, "network", t.Name, strings.Title(string(apiintf.GetOper))

	op := authz.NewAPIServerOperation(authz.NewResource(tenant, group, kind, namespace, name), oper, auditAction)
	ctx = apigwpkg.NewContextWithOperations(ctx, op)

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
	oper, kind, tenant, namespace, group, name, auditAction := apiintf.GetOper, "Network", t.Tenant, t.Namespace, "network", t.Name, strings.Title(string(apiintf.GetOper))

	op := authz.NewAPIServerOperation(authz.NewResource(tenant, group, kind, namespace, name), oper, auditAction)
	ctx = apigwpkg.NewContextWithOperations(ctx, op)

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
	oper, kind, tenant, namespace, group, name, auditAction := apiintf.GetOper, "Service", t.Tenant, t.Namespace, "network", t.Name, strings.Title(string(apiintf.GetOper))

	op := authz.NewAPIServerOperation(authz.NewResource(tenant, group, kind, namespace, name), oper, auditAction)
	ctx = apigwpkg.NewContextWithOperations(ctx, op)

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

func (a adapterNetworkV1) AutoGetVirtualRouter(oldctx oldcontext.Context, t *network.VirtualRouter, options ...grpc.CallOption) (*network.VirtualRouter, error) {
	// Not using options for now. Will be passed through context as needed.
	ctx := context.Context(oldctx)
	prof, err := a.gwSvc.GetServiceProfile("AutoGetVirtualRouter")
	if err != nil {
		return nil, errors.New("unknown service profile")
	}
	oper, kind, tenant, namespace, group, name, auditAction := apiintf.GetOper, "VirtualRouter", t.Tenant, t.Namespace, "network", t.Name, strings.Title(string(apiintf.GetOper))

	op := authz.NewAPIServerOperation(authz.NewResource(tenant, group, kind, namespace, name), oper, auditAction)
	ctx = apigwpkg.NewContextWithOperations(ctx, op)

	fn := func(ctx context.Context, i interface{}) (interface{}, error) {
		in := i.(*network.VirtualRouter)
		return a.service.AutoGetVirtualRouter(ctx, in)
	}
	ret, err := a.gw.HandleRequest(ctx, t, prof, fn)
	if ret == nil {
		return nil, err
	}
	return ret.(*network.VirtualRouter), err
}

func (a adapterNetworkV1) AutoListLbPolicy(oldctx oldcontext.Context, t *api.ListWatchOptions, options ...grpc.CallOption) (*network.LbPolicyList, error) {
	// Not using options for now. Will be passed through context as needed.
	ctx := context.Context(oldctx)
	prof, err := a.gwSvc.GetServiceProfile("AutoListLbPolicy")
	if err != nil {
		return nil, errors.New("unknown service profile")
	}

	if t.Tenant == "" {
		t.Tenant = globals.DefaultTenant
	}
	t.Namespace = ""
	oper, kind, tenant, namespace, group, name, auditAction := apiintf.ListOper, "LbPolicy", t.Tenant, t.Namespace, "network", "", strings.Title(string(apiintf.ListOper))

	op := authz.NewAPIServerOperation(authz.NewResource(tenant, group, kind, namespace, name), oper, auditAction)
	ctx = apigwpkg.NewContextWithOperations(ctx, op)

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

	if t.Tenant == "" {
		t.Tenant = globals.DefaultTenant
	}
	t.Namespace = ""
	oper, kind, tenant, namespace, group, name, auditAction := apiintf.ListOper, "Network", t.Tenant, t.Namespace, "network", "", strings.Title(string(apiintf.ListOper))

	op := authz.NewAPIServerOperation(authz.NewResource(tenant, group, kind, namespace, name), oper, auditAction)
	ctx = apigwpkg.NewContextWithOperations(ctx, op)

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

	if t.Tenant == "" {
		t.Tenant = globals.DefaultTenant
	}
	t.Namespace = ""
	oper, kind, tenant, namespace, group, name, auditAction := apiintf.ListOper, "Service", t.Tenant, t.Namespace, "network", "", strings.Title(string(apiintf.ListOper))

	op := authz.NewAPIServerOperation(authz.NewResource(tenant, group, kind, namespace, name), oper, auditAction)
	ctx = apigwpkg.NewContextWithOperations(ctx, op)

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

func (a adapterNetworkV1) AutoListVirtualRouter(oldctx oldcontext.Context, t *api.ListWatchOptions, options ...grpc.CallOption) (*network.VirtualRouterList, error) {
	// Not using options for now. Will be passed through context as needed.
	ctx := context.Context(oldctx)
	prof, err := a.gwSvc.GetServiceProfile("AutoListVirtualRouter")
	if err != nil {
		return nil, errors.New("unknown service profile")
	}

	if t.Tenant == "" {
		t.Tenant = globals.DefaultTenant
	}
	t.Namespace = ""
	oper, kind, tenant, namespace, group, name, auditAction := apiintf.ListOper, "VirtualRouter", t.Tenant, t.Namespace, "network", "", strings.Title(string(apiintf.ListOper))

	op := authz.NewAPIServerOperation(authz.NewResource(tenant, group, kind, namespace, name), oper, auditAction)
	ctx = apigwpkg.NewContextWithOperations(ctx, op)

	fn := func(ctx context.Context, i interface{}) (interface{}, error) {
		in := i.(*api.ListWatchOptions)
		return a.service.AutoListVirtualRouter(ctx, in)
	}
	ret, err := a.gw.HandleRequest(ctx, t, prof, fn)
	if ret == nil {
		return nil, err
	}
	return ret.(*network.VirtualRouterList), err
}

func (a adapterNetworkV1) AutoUpdateLbPolicy(oldctx oldcontext.Context, t *network.LbPolicy, options ...grpc.CallOption) (*network.LbPolicy, error) {
	// Not using options for now. Will be passed through context as needed.
	ctx := context.Context(oldctx)
	prof, err := a.gwSvc.GetServiceProfile("AutoUpdateLbPolicy")
	if err != nil {
		return nil, errors.New("unknown service profile")
	}
	oper, kind, tenant, namespace, group, name, auditAction := apiintf.UpdateOper, "LbPolicy", t.Tenant, t.Namespace, "network", t.Name, strings.Title(string(apiintf.UpdateOper))

	op := authz.NewAPIServerOperation(authz.NewResource(tenant, group, kind, namespace, name), oper, auditAction)
	ctx = apigwpkg.NewContextWithOperations(ctx, op)

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
	oper, kind, tenant, namespace, group, name, auditAction := apiintf.UpdateOper, "Network", t.Tenant, t.Namespace, "network", t.Name, strings.Title(string(apiintf.UpdateOper))

	op := authz.NewAPIServerOperation(authz.NewResource(tenant, group, kind, namespace, name), oper, auditAction)
	ctx = apigwpkg.NewContextWithOperations(ctx, op)

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
	oper, kind, tenant, namespace, group, name, auditAction := apiintf.UpdateOper, "Service", t.Tenant, t.Namespace, "network", t.Name, strings.Title(string(apiintf.UpdateOper))

	op := authz.NewAPIServerOperation(authz.NewResource(tenant, group, kind, namespace, name), oper, auditAction)
	ctx = apigwpkg.NewContextWithOperations(ctx, op)

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

func (a adapterNetworkV1) AutoUpdateVirtualRouter(oldctx oldcontext.Context, t *network.VirtualRouter, options ...grpc.CallOption) (*network.VirtualRouter, error) {
	// Not using options for now. Will be passed through context as needed.
	ctx := context.Context(oldctx)
	prof, err := a.gwSvc.GetServiceProfile("AutoUpdateVirtualRouter")
	if err != nil {
		return nil, errors.New("unknown service profile")
	}
	oper, kind, tenant, namespace, group, name, auditAction := apiintf.UpdateOper, "VirtualRouter", t.Tenant, t.Namespace, "network", t.Name, strings.Title(string(apiintf.UpdateOper))

	op := authz.NewAPIServerOperation(authz.NewResource(tenant, group, kind, namespace, name), oper, auditAction)
	ctx = apigwpkg.NewContextWithOperations(ctx, op)

	fn := func(ctx context.Context, i interface{}) (interface{}, error) {
		in := i.(*network.VirtualRouter)
		return a.service.AutoUpdateVirtualRouter(ctx, in)
	}
	ret, err := a.gw.HandleRequest(ctx, t, prof, fn)
	if ret == nil {
		return nil, err
	}
	return ret.(*network.VirtualRouter), err
}

func (a adapterNetworkV1) AutoWatchSvcNetworkV1(oldctx oldcontext.Context, in *api.ListWatchOptions, options ...grpc.CallOption) (network.NetworkV1_AutoWatchSvcNetworkV1Client, error) {
	ctx := context.Context(oldctx)
	prof, err := a.gwSvc.GetServiceProfile("AutoWatchSvcNetworkV1")
	if err != nil {
		return nil, errors.New("unknown service profile")
	}
	oper, kind, tenant, namespace, group := apiintf.WatchOper, "", in.Tenant, in.Namespace, "network"
	op := authz.NewAPIServerOperation(authz.NewResource(tenant, group, kind, namespace, ""), oper, strings.Title(string(oper)))
	ctx = apigwpkg.NewContextWithOperations(ctx, op)
	fn := func(ctx context.Context, i interface{}) (interface{}, error) {
		in := i.(*api.ListWatchOptions)
		iws, ok := apiutils.GetVar(ctx, apiutils.CtxKeyAPIGwWebSocketWatch)
		if ok && iws.(bool) {
			nctx, cancel := context.WithCancel(ctx)
			ir, ok := apiutils.GetVar(ctx, apiutils.CtxKeyAPIGwHTTPReq)
			if !ok {
				return nil, errors.New("unable to retrieve request")
			}
			iw, ok := apiutils.GetVar(ctx, apiutils.CtxKeyAPIGwHTTPWriter)
			if !ok {
				return nil, errors.New("unable to retrieve writer")
			}
			conn, err := wsUpgrader.Upgrade(iw.(http.ResponseWriter), ir.(*http.Request), nil)
			if err != nil {
				log.Errorf("WebSocket Upgrade failed (%s)", err)
				return nil, err
			}
			ctx = apiutils.SetVar(nctx, apiutils.CtxKeyAPIGwWebSocketConn, conn)
			conn.SetCloseHandler(func(code int, text string) error {
				cancel()
				log.Infof("received close notification on websocket [AutoWatchNetworkV1] (%v/%v)", code, text)
				return nil
			})
			// start a dummy reciever
			go func() {
				for {
					_, _, err := conn.ReadMessage()
					if err != nil {
						log.Errorf("received error on websocket receive (%s)", err)
						cancel()
						return
					}
				}
			}()
		}
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

	if in.Tenant == "" {
		in.Tenant = globals.DefaultTenant
	}
	in.Namespace = ""
	oper, kind, tenant, namespace, group := apiintf.WatchOper, "Network", in.Tenant, in.Namespace, "network"
	op := authz.NewAPIServerOperation(authz.NewResource(tenant, group, kind, namespace, ""), oper, strings.Title(string(oper)))
	ctx = apigwpkg.NewContextWithOperations(ctx, op)
	fn := func(ctx context.Context, i interface{}) (interface{}, error) {
		in := i.(*api.ListWatchOptions)
		iws, ok := apiutils.GetVar(ctx, apiutils.CtxKeyAPIGwWebSocketWatch)
		if ok && iws.(bool) {
			nctx, cancel := context.WithCancel(ctx)
			ir, ok := apiutils.GetVar(ctx, apiutils.CtxKeyAPIGwHTTPReq)
			if !ok {
				return nil, errors.New("unable to retrieve request")
			}
			iw, ok := apiutils.GetVar(ctx, apiutils.CtxKeyAPIGwHTTPWriter)
			if !ok {
				return nil, errors.New("unable to retrieve writer")
			}
			conn, err := wsUpgrader.Upgrade(iw.(http.ResponseWriter), ir.(*http.Request), nil)
			if err != nil {
				log.Errorf("WebSocket Upgrade failed (%s)", err)
				return nil, err
			}
			ctx = apiutils.SetVar(nctx, apiutils.CtxKeyAPIGwWebSocketConn, conn)
			conn.SetCloseHandler(func(code int, text string) error {
				cancel()
				log.Infof("received close notification on websocket [AutoWatchNetwork] (%v/%v)", code, text)
				return nil
			})
			// start a dummy reciever
			go func() {
				for {
					_, _, err := conn.ReadMessage()
					if err != nil {
						log.Errorf("received error on websocket receive (%s)", err)
						cancel()
						return
					}
				}
			}()
		}
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

	if in.Tenant == "" {
		in.Tenant = globals.DefaultTenant
	}
	in.Namespace = ""
	oper, kind, tenant, namespace, group := apiintf.WatchOper, "Service", in.Tenant, in.Namespace, "network"
	op := authz.NewAPIServerOperation(authz.NewResource(tenant, group, kind, namespace, ""), oper, strings.Title(string(oper)))
	ctx = apigwpkg.NewContextWithOperations(ctx, op)
	fn := func(ctx context.Context, i interface{}) (interface{}, error) {
		in := i.(*api.ListWatchOptions)
		iws, ok := apiutils.GetVar(ctx, apiutils.CtxKeyAPIGwWebSocketWatch)
		if ok && iws.(bool) {
			nctx, cancel := context.WithCancel(ctx)
			ir, ok := apiutils.GetVar(ctx, apiutils.CtxKeyAPIGwHTTPReq)
			if !ok {
				return nil, errors.New("unable to retrieve request")
			}
			iw, ok := apiutils.GetVar(ctx, apiutils.CtxKeyAPIGwHTTPWriter)
			if !ok {
				return nil, errors.New("unable to retrieve writer")
			}
			conn, err := wsUpgrader.Upgrade(iw.(http.ResponseWriter), ir.(*http.Request), nil)
			if err != nil {
				log.Errorf("WebSocket Upgrade failed (%s)", err)
				return nil, err
			}
			ctx = apiutils.SetVar(nctx, apiutils.CtxKeyAPIGwWebSocketConn, conn)
			conn.SetCloseHandler(func(code int, text string) error {
				cancel()
				log.Infof("received close notification on websocket [AutoWatchService] (%v/%v)", code, text)
				return nil
			})
			// start a dummy reciever
			go func() {
				for {
					_, _, err := conn.ReadMessage()
					if err != nil {
						log.Errorf("received error on websocket receive (%s)", err)
						cancel()
						return
					}
				}
			}()
		}
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

	if in.Tenant == "" {
		in.Tenant = globals.DefaultTenant
	}
	in.Namespace = ""
	oper, kind, tenant, namespace, group := apiintf.WatchOper, "LbPolicy", in.Tenant, in.Namespace, "network"
	op := authz.NewAPIServerOperation(authz.NewResource(tenant, group, kind, namespace, ""), oper, strings.Title(string(oper)))
	ctx = apigwpkg.NewContextWithOperations(ctx, op)
	fn := func(ctx context.Context, i interface{}) (interface{}, error) {
		in := i.(*api.ListWatchOptions)
		iws, ok := apiutils.GetVar(ctx, apiutils.CtxKeyAPIGwWebSocketWatch)
		if ok && iws.(bool) {
			nctx, cancel := context.WithCancel(ctx)
			ir, ok := apiutils.GetVar(ctx, apiutils.CtxKeyAPIGwHTTPReq)
			if !ok {
				return nil, errors.New("unable to retrieve request")
			}
			iw, ok := apiutils.GetVar(ctx, apiutils.CtxKeyAPIGwHTTPWriter)
			if !ok {
				return nil, errors.New("unable to retrieve writer")
			}
			conn, err := wsUpgrader.Upgrade(iw.(http.ResponseWriter), ir.(*http.Request), nil)
			if err != nil {
				log.Errorf("WebSocket Upgrade failed (%s)", err)
				return nil, err
			}
			ctx = apiutils.SetVar(nctx, apiutils.CtxKeyAPIGwWebSocketConn, conn)
			conn.SetCloseHandler(func(code int, text string) error {
				cancel()
				log.Infof("received close notification on websocket [AutoWatchLbPolicy] (%v/%v)", code, text)
				return nil
			})
			// start a dummy reciever
			go func() {
				for {
					_, _, err := conn.ReadMessage()
					if err != nil {
						log.Errorf("received error on websocket receive (%s)", err)
						cancel()
						return
					}
				}
			}()
		}
		return a.service.AutoWatchLbPolicy(ctx, in)
	}
	ret, err := a.gw.HandleRequest(ctx, in, prof, fn)
	if ret == nil {
		return nil, err
	}
	return ret.(network.NetworkV1_AutoWatchLbPolicyClient), err
}

func (a adapterNetworkV1) AutoWatchVirtualRouter(oldctx oldcontext.Context, in *api.ListWatchOptions, options ...grpc.CallOption) (network.NetworkV1_AutoWatchVirtualRouterClient, error) {
	ctx := context.Context(oldctx)
	prof, err := a.gwSvc.GetServiceProfile("AutoWatchVirtualRouter")
	if err != nil {
		return nil, errors.New("unknown service profile")
	}

	if in.Tenant == "" {
		in.Tenant = globals.DefaultTenant
	}
	in.Namespace = ""
	oper, kind, tenant, namespace, group := apiintf.WatchOper, "VirtualRouter", in.Tenant, in.Namespace, "network"
	op := authz.NewAPIServerOperation(authz.NewResource(tenant, group, kind, namespace, ""), oper, strings.Title(string(oper)))
	ctx = apigwpkg.NewContextWithOperations(ctx, op)
	fn := func(ctx context.Context, i interface{}) (interface{}, error) {
		in := i.(*api.ListWatchOptions)
		iws, ok := apiutils.GetVar(ctx, apiutils.CtxKeyAPIGwWebSocketWatch)
		if ok && iws.(bool) {
			nctx, cancel := context.WithCancel(ctx)
			ir, ok := apiutils.GetVar(ctx, apiutils.CtxKeyAPIGwHTTPReq)
			if !ok {
				return nil, errors.New("unable to retrieve request")
			}
			iw, ok := apiutils.GetVar(ctx, apiutils.CtxKeyAPIGwHTTPWriter)
			if !ok {
				return nil, errors.New("unable to retrieve writer")
			}
			conn, err := wsUpgrader.Upgrade(iw.(http.ResponseWriter), ir.(*http.Request), nil)
			if err != nil {
				log.Errorf("WebSocket Upgrade failed (%s)", err)
				return nil, err
			}
			ctx = apiutils.SetVar(nctx, apiutils.CtxKeyAPIGwWebSocketConn, conn)
			conn.SetCloseHandler(func(code int, text string) error {
				cancel()
				log.Infof("received close notification on websocket [AutoWatchVirtualRouter] (%v/%v)", code, text)
				return nil
			})
			// start a dummy reciever
			go func() {
				for {
					_, _, err := conn.ReadMessage()
					if err != nil {
						log.Errorf("received error on websocket receive (%s)", err)
						cancel()
						return
					}
				}
			}()
		}
		return a.service.AutoWatchVirtualRouter(ctx, in)
	}
	ret, err := a.gw.HandleRequest(ctx, in, prof, fn)
	if ret == nil {
		return nil, err
	}
	return ret.(network.NetworkV1_AutoWatchVirtualRouterClient), err
}

func (e *sNetworkV1GwService) setupSvcProfile() {
	e.defSvcProf = apigwpkg.NewServiceProfile(nil, "", "network", apiintf.UnknownOper)
	e.defSvcProf.SetDefaults()
	e.svcProf = make(map[string]apigw.ServiceProfile)

	e.svcProf["AutoGetNetwork"] = apigwpkg.NewServiceProfile(e.defSvcProf, "Network", "network", apiintf.GetOper)

	e.svcProf["AutoListNetwork"] = apigwpkg.NewServiceProfile(e.defSvcProf, "NetworkList", "network", apiintf.ListOper)

	e.svcProf["AutoWatchNetwork"] = apigwpkg.NewServiceProfile(e.defSvcProf, "AutoMsgNetworkWatchHelper", "network", apiintf.WatchOper)
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
func (e *sNetworkV1GwService) GetCrudServiceProfile(obj string, oper apiintf.APIOperType) (apigw.ServiceProfile, error) {
	name := apiserver.GetCrudServiceName(obj, oper)
	if name != "" {
		return e.GetServiceProfile(name)
	}
	return nil, errors.New("not found")
}

// GetProxyServiceProfile returns the service Profile for a reverse proxy path
func (e *sNetworkV1GwService) GetProxyServiceProfile(path string) (apigw.ServiceProfile, error) {
	name := "_RProxy_" + path
	return e.GetServiceProfile(name)
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
		logger.ErrorLog("msg", "failed to register swagger spec", "service", "network.NetworkV1", "err", err)
	}
	wg.Add(1)
	go func() {
		defer func() {
			muxMutex.Lock()
			mux = nil
			muxMutex.Unlock()
		}()
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
			logger.ErrorLog("msg", "failed to register", "service", "network.NetworkV1", "err", err)
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
	opts = append(opts, rpckit.WithTLSClientIdentity(globals.APIGw))
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
				e.logger.ErrorLog("msg", "Failed to close conn on Done()", "addr", grpcAddr, "err", cerr)
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
