/*
Package network is a auto generated package.
Input file: protos/network.proto
*/
package networkApiServer

import (
	"context"

	"github.com/pensando/sw/apiserver"
	"github.com/pensando/sw/apiserver/pkg"

	network "github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/utils/kvstore"
	"github.com/pensando/sw/utils/log"
	"github.com/pensando/sw/utils/runtime"
	"github.com/pkg/errors"
	"google.golang.org/grpc"
)

var apisrv apiserver.Server

type s_networkBackend struct {
	Services map[string]apiserver.Service
	Messages map[string]apiserver.Message

	endpoints_TenantV1        *e_TenantV1Endpoints
	endpoints_NetworkV1       *e_NetworkV1Endpoints
	endpoints_SecurityGroupV1 *e_SecurityGroupV1Endpoints
	endpoints_SgpolicyV1      *e_SgpolicyV1Endpoints
	endpoints_ServiceV1       *e_ServiceV1Endpoints
	endpoints_LbPolicyV1      *e_LbPolicyV1Endpoints
	endpoints_EndpointV1      *e_EndpointV1Endpoints
}

type e_TenantV1Endpoints struct {
	Svc s_networkBackend

	fn_GetTenantList func(ctx context.Context, t interface{}) (interface{}, error)
	fn_TenantOper    func(ctx context.Context, t interface{}) (interface{}, error)
}
type e_NetworkV1Endpoints struct {
	Svc s_networkBackend

	fn_GetNetworkList func(ctx context.Context, t interface{}) (interface{}, error)
	fn_NetworkOper    func(ctx context.Context, t interface{}) (interface{}, error)
}
type e_SecurityGroupV1Endpoints struct {
	Svc s_networkBackend

	fn_GetSecurityGroupList func(ctx context.Context, t interface{}) (interface{}, error)
	fn_SecurityGroupOper    func(ctx context.Context, t interface{}) (interface{}, error)
}
type e_SgpolicyV1Endpoints struct {
	Svc s_networkBackend

	fn_GetSgpolicyList func(ctx context.Context, t interface{}) (interface{}, error)
	fn_SgpolicyOper    func(ctx context.Context, t interface{}) (interface{}, error)
}
type e_ServiceV1Endpoints struct {
	Svc s_networkBackend

	fn_GetServiceList func(ctx context.Context, t interface{}) (interface{}, error)
	fn_ServiceOper    func(ctx context.Context, t interface{}) (interface{}, error)
}
type e_LbPolicyV1Endpoints struct {
	Svc s_networkBackend

	fn_GetLbPolicyList func(ctx context.Context, t interface{}) (interface{}, error)
	fn_LbPolicyOper    func(ctx context.Context, t interface{}) (interface{}, error)
}
type e_EndpointV1Endpoints struct {
	Svc s_networkBackend

	fn_GetEndpointList func(ctx context.Context, t interface{}) (interface{}, error)
	fn_EndpointOper    func(ctx context.Context, t interface{}) (interface{}, error)
}

func (s *s_networkBackend) CompleteRegistration(ctx context.Context, logger log.Logger,
	grpcserver *grpc.Server, scheme *runtime.Scheme) error {
	s.Messages = map[string]apiserver.Message{

		"network.TenantSpec":   apisrvpkg.NewMessage("network.TenantSpec"),
		"network.TenantStatus": apisrvpkg.NewMessage("network.TenantStatus"),
		"network.Tenant": apisrvpkg.NewMessage("network.Tenant").WithKeyGenerator(func(i interface{}, prefix string) string {
			r := i.(network.Tenant)
			return r.MakeKey(prefix)
		}).WithKvUpdater(func(ctx context.Context, kvs kvstore.Interface, i interface{}, prefix string, create bool) (interface{}, error) {
			r := i.(network.Tenant)
			key := r.MakeKey(prefix)
			var err error
			if create {
				err = kvs.Create(ctx, key, &r)
				err = errors.Wrap(err, "KV create failed")
			} else {
				err = kvs.Update(ctx, key, &r)
				err = errors.Wrap(err, "KV update failed")
			}
			return r, err
		}).WithKvTxnUpdater(func(ctx context.Context, txn kvstore.Txn, i interface{}, prefix string, create bool) error {
			r := i.(network.Tenant)
			key := r.MakeKey(prefix)
			var err error
			if create {
				err = txn.Create(key, &r)
				err = errors.Wrap(err, "KV transaction create failed")
			} else {
				err = txn.Update(key, &r)
				err = errors.Wrap(err, "KV transaction update failed")
			}
			return err
		}).WithKvGetter(func(ctx context.Context, kvs kvstore.Interface, key string) (interface{}, error) {
			r := network.Tenant{}
			err := kvs.Get(ctx, key, &r)
			err = errors.Wrap(err, "KV get failed")
			return r, err
		}).WithKvDelFunc(func(ctx context.Context, kvs kvstore.Interface, key string) (interface{}, error) {
			r := network.Tenant{}
			err := kvs.Delete(ctx, key, &r)
			return r, err
		}).WithKvTxnDelFunc(func(ctx context.Context, txn kvstore.Txn, key string) error {
			return txn.Delete(key)
		}),
		"network.TenantList":    apisrvpkg.NewMessage("network.TenantList"),
		"network.NetworkSpec":   apisrvpkg.NewMessage("network.NetworkSpec"),
		"network.NetworkStatus": apisrvpkg.NewMessage("network.NetworkStatus"),
		"network.Network": apisrvpkg.NewMessage("network.Network").WithKeyGenerator(func(i interface{}, prefix string) string {
			r := i.(network.Network)
			return r.MakeKey(prefix)
		}).WithKvUpdater(func(ctx context.Context, kvs kvstore.Interface, i interface{}, prefix string, create bool) (interface{}, error) {
			r := i.(network.Network)
			key := r.MakeKey(prefix)
			var err error
			if create {
				err = kvs.Create(ctx, key, &r)
				err = errors.Wrap(err, "KV create failed")
			} else {
				err = kvs.Update(ctx, key, &r)
				err = errors.Wrap(err, "KV update failed")
			}
			return r, err
		}).WithKvTxnUpdater(func(ctx context.Context, txn kvstore.Txn, i interface{}, prefix string, create bool) error {
			r := i.(network.Network)
			key := r.MakeKey(prefix)
			var err error
			if create {
				err = txn.Create(key, &r)
				err = errors.Wrap(err, "KV transaction create failed")
			} else {
				err = txn.Update(key, &r)
				err = errors.Wrap(err, "KV transaction update failed")
			}
			return err
		}).WithKvGetter(func(ctx context.Context, kvs kvstore.Interface, key string) (interface{}, error) {
			r := network.Network{}
			err := kvs.Get(ctx, key, &r)
			err = errors.Wrap(err, "KV get failed")
			return r, err
		}).WithKvDelFunc(func(ctx context.Context, kvs kvstore.Interface, key string) (interface{}, error) {
			r := network.Network{}
			err := kvs.Delete(ctx, key, &r)
			return r, err
		}).WithKvTxnDelFunc(func(ctx context.Context, txn kvstore.Txn, key string) error {
			return txn.Delete(key)
		}),
		"network.NetworkList":         apisrvpkg.NewMessage("network.NetworkList"),
		"network.SecurityGroupSpec":   apisrvpkg.NewMessage("network.SecurityGroupSpec"),
		"network.SecurityGroupStatus": apisrvpkg.NewMessage("network.SecurityGroupStatus"),
		"network.SecurityGroup": apisrvpkg.NewMessage("network.SecurityGroup").WithKeyGenerator(func(i interface{}, prefix string) string {
			r := i.(network.SecurityGroup)
			return r.MakeKey(prefix)
		}).WithKvUpdater(func(ctx context.Context, kvs kvstore.Interface, i interface{}, prefix string, create bool) (interface{}, error) {
			r := i.(network.SecurityGroup)
			key := r.MakeKey(prefix)
			var err error
			if create {
				err = kvs.Create(ctx, key, &r)
				err = errors.Wrap(err, "KV create failed")
			} else {
				err = kvs.Update(ctx, key, &r)
				err = errors.Wrap(err, "KV update failed")
			}
			return r, err
		}).WithKvTxnUpdater(func(ctx context.Context, txn kvstore.Txn, i interface{}, prefix string, create bool) error {
			r := i.(network.SecurityGroup)
			key := r.MakeKey(prefix)
			var err error
			if create {
				err = txn.Create(key, &r)
				err = errors.Wrap(err, "KV transaction create failed")
			} else {
				err = txn.Update(key, &r)
				err = errors.Wrap(err, "KV transaction update failed")
			}
			return err
		}).WithKvGetter(func(ctx context.Context, kvs kvstore.Interface, key string) (interface{}, error) {
			r := network.SecurityGroup{}
			err := kvs.Get(ctx, key, &r)
			err = errors.Wrap(err, "KV get failed")
			return r, err
		}).WithKvDelFunc(func(ctx context.Context, kvs kvstore.Interface, key string) (interface{}, error) {
			r := network.SecurityGroup{}
			err := kvs.Delete(ctx, key, &r)
			return r, err
		}).WithKvTxnDelFunc(func(ctx context.Context, txn kvstore.Txn, key string) error {
			return txn.Delete(key)
		}),
		"network.SecurityGroupList": apisrvpkg.NewMessage("network.SecurityGroupList"),
		"network.SGRule":            apisrvpkg.NewMessage("network.SGRule"),
		"network.SgpolicySpec":      apisrvpkg.NewMessage("network.SgpolicySpec"),
		"network.SgpolicyStatus":    apisrvpkg.NewMessage("network.SgpolicyStatus"),
		"network.Sgpolicy": apisrvpkg.NewMessage("network.Sgpolicy").WithKeyGenerator(func(i interface{}, prefix string) string {
			r := i.(network.Sgpolicy)
			return r.MakeKey(prefix)
		}).WithKvUpdater(func(ctx context.Context, kvs kvstore.Interface, i interface{}, prefix string, create bool) (interface{}, error) {
			r := i.(network.Sgpolicy)
			key := r.MakeKey(prefix)
			var err error
			if create {
				err = kvs.Create(ctx, key, &r)
				err = errors.Wrap(err, "KV create failed")
			} else {
				err = kvs.Update(ctx, key, &r)
				err = errors.Wrap(err, "KV update failed")
			}
			return r, err
		}).WithKvTxnUpdater(func(ctx context.Context, txn kvstore.Txn, i interface{}, prefix string, create bool) error {
			r := i.(network.Sgpolicy)
			key := r.MakeKey(prefix)
			var err error
			if create {
				err = txn.Create(key, &r)
				err = errors.Wrap(err, "KV transaction create failed")
			} else {
				err = txn.Update(key, &r)
				err = errors.Wrap(err, "KV transaction update failed")
			}
			return err
		}).WithKvGetter(func(ctx context.Context, kvs kvstore.Interface, key string) (interface{}, error) {
			r := network.Sgpolicy{}
			err := kvs.Get(ctx, key, &r)
			err = errors.Wrap(err, "KV get failed")
			return r, err
		}).WithKvDelFunc(func(ctx context.Context, kvs kvstore.Interface, key string) (interface{}, error) {
			r := network.Sgpolicy{}
			err := kvs.Delete(ctx, key, &r)
			return r, err
		}).WithKvTxnDelFunc(func(ctx context.Context, txn kvstore.Txn, key string) error {
			return txn.Delete(key)
		}),
		"network.SgpolicyList":  apisrvpkg.NewMessage("network.SgpolicyList"),
		"network.ServiceSpec":   apisrvpkg.NewMessage("network.ServiceSpec"),
		"network.ServiceStatus": apisrvpkg.NewMessage("network.ServiceStatus"),
		"network.Service": apisrvpkg.NewMessage("network.Service").WithKeyGenerator(func(i interface{}, prefix string) string {
			r := i.(network.Service)
			return r.MakeKey(prefix)
		}).WithKvUpdater(func(ctx context.Context, kvs kvstore.Interface, i interface{}, prefix string, create bool) (interface{}, error) {
			r := i.(network.Service)
			key := r.MakeKey(prefix)
			var err error
			if create {
				err = kvs.Create(ctx, key, &r)
				err = errors.Wrap(err, "KV create failed")
			} else {
				err = kvs.Update(ctx, key, &r)
				err = errors.Wrap(err, "KV update failed")
			}
			return r, err
		}).WithKvTxnUpdater(func(ctx context.Context, txn kvstore.Txn, i interface{}, prefix string, create bool) error {
			r := i.(network.Service)
			key := r.MakeKey(prefix)
			var err error
			if create {
				err = txn.Create(key, &r)
				err = errors.Wrap(err, "KV transaction create failed")
			} else {
				err = txn.Update(key, &r)
				err = errors.Wrap(err, "KV transaction update failed")
			}
			return err
		}).WithKvGetter(func(ctx context.Context, kvs kvstore.Interface, key string) (interface{}, error) {
			r := network.Service{}
			err := kvs.Get(ctx, key, &r)
			err = errors.Wrap(err, "KV get failed")
			return r, err
		}).WithKvDelFunc(func(ctx context.Context, kvs kvstore.Interface, key string) (interface{}, error) {
			r := network.Service{}
			err := kvs.Delete(ctx, key, &r)
			return r, err
		}).WithKvTxnDelFunc(func(ctx context.Context, txn kvstore.Txn, key string) error {
			return txn.Delete(key)
		}),
		"network.ServiceList":     apisrvpkg.NewMessage("network.ServiceList"),
		"network.HealthCheckSpec": apisrvpkg.NewMessage("network.HealthCheckSpec"),
		"network.LbPolicySpec":    apisrvpkg.NewMessage("network.LbPolicySpec"),
		"network.LbPolicyStatus":  apisrvpkg.NewMessage("network.LbPolicyStatus"),
		"network.LbPolicy": apisrvpkg.NewMessage("network.LbPolicy").WithKeyGenerator(func(i interface{}, prefix string) string {
			r := i.(network.LbPolicy)
			return r.MakeKey(prefix)
		}).WithKvUpdater(func(ctx context.Context, kvs kvstore.Interface, i interface{}, prefix string, create bool) (interface{}, error) {
			r := i.(network.LbPolicy)
			key := r.MakeKey(prefix)
			var err error
			if create {
				err = kvs.Create(ctx, key, &r)
				err = errors.Wrap(err, "KV create failed")
			} else {
				err = kvs.Update(ctx, key, &r)
				err = errors.Wrap(err, "KV update failed")
			}
			return r, err
		}).WithKvTxnUpdater(func(ctx context.Context, txn kvstore.Txn, i interface{}, prefix string, create bool) error {
			r := i.(network.LbPolicy)
			key := r.MakeKey(prefix)
			var err error
			if create {
				err = txn.Create(key, &r)
				err = errors.Wrap(err, "KV transaction create failed")
			} else {
				err = txn.Update(key, &r)
				err = errors.Wrap(err, "KV transaction update failed")
			}
			return err
		}).WithKvGetter(func(ctx context.Context, kvs kvstore.Interface, key string) (interface{}, error) {
			r := network.LbPolicy{}
			err := kvs.Get(ctx, key, &r)
			err = errors.Wrap(err, "KV get failed")
			return r, err
		}).WithKvDelFunc(func(ctx context.Context, kvs kvstore.Interface, key string) (interface{}, error) {
			r := network.LbPolicy{}
			err := kvs.Delete(ctx, key, &r)
			return r, err
		}).WithKvTxnDelFunc(func(ctx context.Context, txn kvstore.Txn, key string) error {
			return txn.Delete(key)
		}),
		"network.LbPolicyList":   apisrvpkg.NewMessage("network.LbPolicyList"),
		"network.EndpointSpec":   apisrvpkg.NewMessage("network.EndpointSpec"),
		"network.EndpointStatus": apisrvpkg.NewMessage("network.EndpointStatus"),
		"network.Endpoint": apisrvpkg.NewMessage("network.Endpoint").WithKeyGenerator(func(i interface{}, prefix string) string {
			r := i.(network.Endpoint)
			return r.MakeKey(prefix)
		}).WithKvUpdater(func(ctx context.Context, kvs kvstore.Interface, i interface{}, prefix string, create bool) (interface{}, error) {
			r := i.(network.Endpoint)
			key := r.MakeKey(prefix)
			var err error
			if create {
				err = kvs.Create(ctx, key, &r)
				err = errors.Wrap(err, "KV create failed")
			} else {
				err = kvs.Update(ctx, key, &r)
				err = errors.Wrap(err, "KV update failed")
			}
			return r, err
		}).WithKvTxnUpdater(func(ctx context.Context, txn kvstore.Txn, i interface{}, prefix string, create bool) error {
			r := i.(network.Endpoint)
			key := r.MakeKey(prefix)
			var err error
			if create {
				err = txn.Create(key, &r)
				err = errors.Wrap(err, "KV transaction create failed")
			} else {
				err = txn.Update(key, &r)
				err = errors.Wrap(err, "KV transaction update failed")
			}
			return err
		}).WithKvGetter(func(ctx context.Context, kvs kvstore.Interface, key string) (interface{}, error) {
			r := network.Endpoint{}
			err := kvs.Get(ctx, key, &r)
			err = errors.Wrap(err, "KV get failed")
			return r, err
		}).WithKvDelFunc(func(ctx context.Context, kvs kvstore.Interface, key string) (interface{}, error) {
			r := network.Endpoint{}
			err := kvs.Delete(ctx, key, &r)
			return r, err
		}).WithKvTxnDelFunc(func(ctx context.Context, txn kvstore.Txn, key string) error {
			return txn.Delete(key)
		}),
		"network.EndpointList": apisrvpkg.NewMessage("network.EndpointList"),
	}

	scheme.AddKnownTypes(
		&network.Tenant{},
		&network.Network{},
		&network.SecurityGroup{},
		&network.Sgpolicy{},
		&network.Service{},
		&network.LbPolicy{},
		&network.Endpoint{},
	)

	apisrv.RegisterMessages("network", s.Messages)

	{
		srv := apisrvpkg.NewService("TenantV1")

		s.endpoints_TenantV1.fn_GetTenantList = srv.AddMethod("GetTenantList",
			apisrvpkg.NewMethod(s.Messages["network.TenantList"], s.Messages["network.TenantList"], "tenant", "GetTenantList")).WithPreCommitHook(func(ctx context.Context, kvs kvstore.Interface, txn kvstore.Txn, key, oper string, i interface{}) (interface{}, bool, error) {
			return i, false, nil
		}).WithVersion("v1").HandleInvocation

		s.endpoints_TenantV1.fn_TenantOper = srv.AddMethod("TenantOper",
			apisrvpkg.NewMethod(s.Messages["network.Tenant"], s.Messages["network.Tenant"], "tenant", "TenantOper")).WithVersion("v1").HandleInvocation

		s.Services = map[string]apiserver.Service{
			"network.TenantV1": srv,
		}
		apisrv.RegisterService("network.TenantV1", srv)
		endpoints := network.MakeTenantV1ServerEndpoints(s.endpoints_TenantV1, logger)
		server := network.MakeGRPCServerTenantV1(ctx, endpoints, logger)
		network.RegisterTenantV1Server(grpcserver, server)
	}
	{
		srv := apisrvpkg.NewService("NetworkV1")

		s.endpoints_NetworkV1.fn_GetNetworkList = srv.AddMethod("GetNetworkList",
			apisrvpkg.NewMethod(s.Messages["network.NetworkList"], s.Messages["network.NetworkList"], "network", "GetNetworkList")).WithPreCommitHook(func(ctx context.Context, kvs kvstore.Interface, txn kvstore.Txn, key, oper string, i interface{}) (interface{}, bool, error) {
			return i, false, nil
		}).WithVersion("v1").HandleInvocation

		s.endpoints_NetworkV1.fn_NetworkOper = srv.AddMethod("NetworkOper",
			apisrvpkg.NewMethod(s.Messages["network.Network"], s.Messages["network.Network"], "network", "NetworkOper")).WithVersion("v1").HandleInvocation

		s.Services = map[string]apiserver.Service{
			"network.NetworkV1": srv,
		}
		apisrv.RegisterService("network.NetworkV1", srv)
		endpoints := network.MakeNetworkV1ServerEndpoints(s.endpoints_NetworkV1, logger)
		server := network.MakeGRPCServerNetworkV1(ctx, endpoints, logger)
		network.RegisterNetworkV1Server(grpcserver, server)
	}
	{
		srv := apisrvpkg.NewService("SecurityGroupV1")

		s.endpoints_SecurityGroupV1.fn_GetSecurityGroupList = srv.AddMethod("GetSecurityGroupList",
			apisrvpkg.NewMethod(s.Messages["network.SecurityGroupList"], s.Messages["network.SecurityGroupList"], "security-group", "GetSecurityGroupList")).WithPreCommitHook(func(ctx context.Context, kvs kvstore.Interface, txn kvstore.Txn, key, oper string, i interface{}) (interface{}, bool, error) {
			return i, false, nil
		}).WithVersion("v1").HandleInvocation

		s.endpoints_SecurityGroupV1.fn_SecurityGroupOper = srv.AddMethod("SecurityGroupOper",
			apisrvpkg.NewMethod(s.Messages["network.SecurityGroup"], s.Messages["network.SecurityGroup"], "security-group", "SecurityGroupOper")).WithVersion("v1").HandleInvocation

		s.Services = map[string]apiserver.Service{
			"network.SecurityGroupV1": srv,
		}
		apisrv.RegisterService("network.SecurityGroupV1", srv)
		endpoints := network.MakeSecurityGroupV1ServerEndpoints(s.endpoints_SecurityGroupV1, logger)
		server := network.MakeGRPCServerSecurityGroupV1(ctx, endpoints, logger)
		network.RegisterSecurityGroupV1Server(grpcserver, server)
	}
	{
		srv := apisrvpkg.NewService("SgpolicyV1")

		s.endpoints_SgpolicyV1.fn_GetSgpolicyList = srv.AddMethod("GetSgpolicyList",
			apisrvpkg.NewMethod(s.Messages["network.SgpolicyList"], s.Messages["network.SgpolicyList"], "sgpolicy", "GetSgpolicyList")).WithPreCommitHook(func(ctx context.Context, kvs kvstore.Interface, txn kvstore.Txn, key, oper string, i interface{}) (interface{}, bool, error) {
			return i, false, nil
		}).WithVersion("v1").HandleInvocation

		s.endpoints_SgpolicyV1.fn_SgpolicyOper = srv.AddMethod("SgpolicyOper",
			apisrvpkg.NewMethod(s.Messages["network.Sgpolicy"], s.Messages["network.Sgpolicy"], "sgpolicy", "SgpolicyOper")).WithVersion("v1").HandleInvocation

		s.Services = map[string]apiserver.Service{
			"network.SgpolicyV1": srv,
		}
		apisrv.RegisterService("network.SgpolicyV1", srv)
		endpoints := network.MakeSgpolicyV1ServerEndpoints(s.endpoints_SgpolicyV1, logger)
		server := network.MakeGRPCServerSgpolicyV1(ctx, endpoints, logger)
		network.RegisterSgpolicyV1Server(grpcserver, server)
	}
	{
		srv := apisrvpkg.NewService("ServiceV1")

		s.endpoints_ServiceV1.fn_GetServiceList = srv.AddMethod("GetServiceList",
			apisrvpkg.NewMethod(s.Messages["network.ServiceList"], s.Messages["network.ServiceList"], "service", "GetServiceList")).WithPreCommitHook(func(ctx context.Context, kvs kvstore.Interface, txn kvstore.Txn, key, oper string, i interface{}) (interface{}, bool, error) {
			return i, false, nil
		}).WithVersion("v1").HandleInvocation

		s.endpoints_ServiceV1.fn_ServiceOper = srv.AddMethod("ServiceOper",
			apisrvpkg.NewMethod(s.Messages["network.Service"], s.Messages["network.Service"], "service", "ServiceOper")).WithVersion("v1").HandleInvocation

		s.Services = map[string]apiserver.Service{
			"network.ServiceV1": srv,
		}
		apisrv.RegisterService("network.ServiceV1", srv)
		endpoints := network.MakeServiceV1ServerEndpoints(s.endpoints_ServiceV1, logger)
		server := network.MakeGRPCServerServiceV1(ctx, endpoints, logger)
		network.RegisterServiceV1Server(grpcserver, server)
	}
	{
		srv := apisrvpkg.NewService("LbPolicyV1")

		s.endpoints_LbPolicyV1.fn_GetLbPolicyList = srv.AddMethod("GetLbPolicyList",
			apisrvpkg.NewMethod(s.Messages["network.LbPolicyList"], s.Messages["network.LbPolicyList"], "lb-policy", "GetLbPolicyList")).WithPreCommitHook(func(ctx context.Context, kvs kvstore.Interface, txn kvstore.Txn, key, oper string, i interface{}) (interface{}, bool, error) {
			return i, false, nil
		}).WithVersion("v1").HandleInvocation

		s.endpoints_LbPolicyV1.fn_LbPolicyOper = srv.AddMethod("LbPolicyOper",
			apisrvpkg.NewMethod(s.Messages["network.LbPolicy"], s.Messages["network.LbPolicy"], "lb-policy", "LbPolicyOper")).WithVersion("v1").HandleInvocation

		s.Services = map[string]apiserver.Service{
			"network.LbPolicyV1": srv,
		}
		apisrv.RegisterService("network.LbPolicyV1", srv)
		endpoints := network.MakeLbPolicyV1ServerEndpoints(s.endpoints_LbPolicyV1, logger)
		server := network.MakeGRPCServerLbPolicyV1(ctx, endpoints, logger)
		network.RegisterLbPolicyV1Server(grpcserver, server)
	}
	{
		srv := apisrvpkg.NewService("EndpointV1")

		s.endpoints_EndpointV1.fn_GetEndpointList = srv.AddMethod("GetEndpointList",
			apisrvpkg.NewMethod(s.Messages["network.EndpointList"], s.Messages["network.EndpointList"], "endpoint", "GetEndpointList")).WithPreCommitHook(func(ctx context.Context, kvs kvstore.Interface, txn kvstore.Txn, key, oper string, i interface{}) (interface{}, bool, error) {
			return i, false, nil
		}).WithVersion("v1").HandleInvocation

		s.endpoints_EndpointV1.fn_EndpointOper = srv.AddMethod("EndpointOper",
			apisrvpkg.NewMethod(s.Messages["network.Endpoint"], s.Messages["network.Endpoint"], "endpoint", "EndpointOper")).WithVersion("v1").HandleInvocation

		s.Services = map[string]apiserver.Service{
			"network.EndpointV1": srv,
		}
		apisrv.RegisterService("network.EndpointV1", srv)
		endpoints := network.MakeEndpointV1ServerEndpoints(s.endpoints_EndpointV1, logger)
		server := network.MakeGRPCServerEndpointV1(ctx, endpoints, logger)
		network.RegisterEndpointV1Server(grpcserver, server)
	}
	return nil
}

func (e *e_TenantV1Endpoints) GetTenantList(ctx context.Context, t network.TenantList) (network.TenantList, error) {
	r, err := e.fn_GetTenantList(ctx, t)
	if err == nil {
		return r.(network.TenantList), err
	} else {
		return network.TenantList{}, err
	}
}
func (e *e_TenantV1Endpoints) TenantOper(ctx context.Context, t network.Tenant) (network.Tenant, error) {
	r, err := e.fn_TenantOper(ctx, t)
	if err == nil {
		return r.(network.Tenant), err
	} else {
		return network.Tenant{}, err
	}
}
func (e *e_NetworkV1Endpoints) GetNetworkList(ctx context.Context, t network.NetworkList) (network.NetworkList, error) {
	r, err := e.fn_GetNetworkList(ctx, t)
	if err == nil {
		return r.(network.NetworkList), err
	} else {
		return network.NetworkList{}, err
	}
}
func (e *e_NetworkV1Endpoints) NetworkOper(ctx context.Context, t network.Network) (network.Network, error) {
	r, err := e.fn_NetworkOper(ctx, t)
	if err == nil {
		return r.(network.Network), err
	} else {
		return network.Network{}, err
	}
}
func (e *e_SecurityGroupV1Endpoints) GetSecurityGroupList(ctx context.Context, t network.SecurityGroupList) (network.SecurityGroupList, error) {
	r, err := e.fn_GetSecurityGroupList(ctx, t)
	if err == nil {
		return r.(network.SecurityGroupList), err
	} else {
		return network.SecurityGroupList{}, err
	}
}
func (e *e_SecurityGroupV1Endpoints) SecurityGroupOper(ctx context.Context, t network.SecurityGroup) (network.SecurityGroup, error) {
	r, err := e.fn_SecurityGroupOper(ctx, t)
	if err == nil {
		return r.(network.SecurityGroup), err
	} else {
		return network.SecurityGroup{}, err
	}
}
func (e *e_SgpolicyV1Endpoints) GetSgpolicyList(ctx context.Context, t network.SgpolicyList) (network.SgpolicyList, error) {
	r, err := e.fn_GetSgpolicyList(ctx, t)
	if err == nil {
		return r.(network.SgpolicyList), err
	} else {
		return network.SgpolicyList{}, err
	}
}
func (e *e_SgpolicyV1Endpoints) SgpolicyOper(ctx context.Context, t network.Sgpolicy) (network.Sgpolicy, error) {
	r, err := e.fn_SgpolicyOper(ctx, t)
	if err == nil {
		return r.(network.Sgpolicy), err
	} else {
		return network.Sgpolicy{}, err
	}
}
func (e *e_ServiceV1Endpoints) GetServiceList(ctx context.Context, t network.ServiceList) (network.ServiceList, error) {
	r, err := e.fn_GetServiceList(ctx, t)
	if err == nil {
		return r.(network.ServiceList), err
	} else {
		return network.ServiceList{}, err
	}
}
func (e *e_ServiceV1Endpoints) ServiceOper(ctx context.Context, t network.Service) (network.Service, error) {
	r, err := e.fn_ServiceOper(ctx, t)
	if err == nil {
		return r.(network.Service), err
	} else {
		return network.Service{}, err
	}
}
func (e *e_LbPolicyV1Endpoints) GetLbPolicyList(ctx context.Context, t network.LbPolicyList) (network.LbPolicyList, error) {
	r, err := e.fn_GetLbPolicyList(ctx, t)
	if err == nil {
		return r.(network.LbPolicyList), err
	} else {
		return network.LbPolicyList{}, err
	}
}
func (e *e_LbPolicyV1Endpoints) LbPolicyOper(ctx context.Context, t network.LbPolicy) (network.LbPolicy, error) {
	r, err := e.fn_LbPolicyOper(ctx, t)
	if err == nil {
		return r.(network.LbPolicy), err
	} else {
		return network.LbPolicy{}, err
	}
}
func (e *e_EndpointV1Endpoints) GetEndpointList(ctx context.Context, t network.EndpointList) (network.EndpointList, error) {
	r, err := e.fn_GetEndpointList(ctx, t)
	if err == nil {
		return r.(network.EndpointList), err
	} else {
		return network.EndpointList{}, err
	}
}
func (e *e_EndpointV1Endpoints) EndpointOper(ctx context.Context, t network.Endpoint) (network.Endpoint, error) {
	r, err := e.fn_EndpointOper(ctx, t)
	if err == nil {
		return r.(network.Endpoint), err
	} else {
		return network.Endpoint{}, err
	}
}

func init() {
	apisrv = apisrvpkg.MustGetAPIServer()

	svc := s_networkBackend{}

	{
		e := e_TenantV1Endpoints{Svc: svc}
		svc.endpoints_TenantV1 = &e
	}
	{
		e := e_NetworkV1Endpoints{Svc: svc}
		svc.endpoints_NetworkV1 = &e
	}
	{
		e := e_SecurityGroupV1Endpoints{Svc: svc}
		svc.endpoints_SecurityGroupV1 = &e
	}
	{
		e := e_SgpolicyV1Endpoints{Svc: svc}
		svc.endpoints_SgpolicyV1 = &e
	}
	{
		e := e_ServiceV1Endpoints{Svc: svc}
		svc.endpoints_ServiceV1 = &e
	}
	{
		e := e_LbPolicyV1Endpoints{Svc: svc}
		svc.endpoints_LbPolicyV1 = &e
	}
	{
		e := e_EndpointV1Endpoints{Svc: svc}
		svc.endpoints_EndpointV1 = &e
	}
	apisrv.Register("network.protos/network.proto", &svc)
}
