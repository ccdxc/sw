// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

/*
Package networkCliUtilsBackend is a auto generated package.
Input file: svc_network.proto
*/
package cli

import (
	"context"
	"fmt"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/network"
	loginctx "github.com/pensando/sw/api/login/context"
	"github.com/pensando/sw/venice/cli/gen"
)

func restGetNetwork(hostname, tenant, token string, obj interface{}) error {

	restcl, err := apiclient.NewRestAPIClient(hostname)
	if err != nil {
		return fmt.Errorf("cannot create REST client")
	}
	defer restcl.Close()
	loginCtx := loginctx.NewContextWithAuthzHeader(context.Background(), "Bearer "+token)

	if v, ok := obj.(*network.Network); ok {
		nv, err := restcl.NetworkV1().Network().Get(loginCtx, &v.ObjectMeta)
		if err != nil {
			return err
		}
		*v = *nv
	}

	if v, ok := obj.(*network.NetworkList); ok {
		opts := api.ListWatchOptions{ObjectMeta: api.ObjectMeta{Tenant: tenant}}
		nv, err := restcl.NetworkV1().Network().List(loginCtx, &opts)
		if err != nil {
			return err
		}
		v.Items = nv
	}
	return nil

}

func restDeleteNetwork(hostname, token string, obj interface{}) error {

	restcl, err := apiclient.NewRestAPIClient(hostname)
	if err != nil {
		return fmt.Errorf("cannot create REST client")
	}
	defer restcl.Close()
	loginCtx := loginctx.NewContextWithAuthzHeader(context.Background(), "Bearer "+token)

	if v, ok := obj.(*network.Network); ok {
		nv, err := restcl.NetworkV1().Network().Delete(loginCtx, &v.ObjectMeta)
		if err != nil {
			return err
		}
		*v = *nv
	}
	return nil

}

func restPostNetwork(hostname, token string, obj interface{}) error {

	restcl, err := apiclient.NewRestAPIClient(hostname)
	if err != nil {
		return fmt.Errorf("cannot create REST client")
	}
	defer restcl.Close()
	loginCtx := loginctx.NewContextWithAuthzHeader(context.Background(), "Bearer "+token)

	if v, ok := obj.(*network.Network); ok {
		nv, err := restcl.NetworkV1().Network().Create(loginCtx, v)
		if err != nil {
			return err
		}
		*v = *nv
	}
	return nil

}

func restPutNetwork(hostname, token string, obj interface{}) error {

	restcl, err := apiclient.NewRestAPIClient(hostname)
	if err != nil {
		return fmt.Errorf("cannot create REST client")
	}
	defer restcl.Close()
	loginCtx := loginctx.NewContextWithAuthzHeader(context.Background(), "Bearer "+token)

	if v, ok := obj.(*network.Network); ok {
		nv, err := restcl.NetworkV1().Network().Update(loginCtx, v)
		if err != nil {
			return err
		}
		*v = *nv
	}
	return nil

}

func restGetService(hostname, tenant, token string, obj interface{}) error {
	return fmt.Errorf("get operation not supported for Service object")
}

func restDeleteService(hostname, token string, obj interface{}) error {
	return fmt.Errorf("delete operation not supported for Service object")
}

func restPostService(hostname, token string, obj interface{}) error {
	return fmt.Errorf("create operation not supported for Service object")
}

func restPutService(hostname, token string, obj interface{}) error {
	return fmt.Errorf("put operation not supported for Service object")
}

func restGetLbPolicy(hostname, tenant, token string, obj interface{}) error {
	return fmt.Errorf("get operation not supported for LbPolicy object")
}

func restDeleteLbPolicy(hostname, token string, obj interface{}) error {
	return fmt.Errorf("delete operation not supported for LbPolicy object")
}

func restPostLbPolicy(hostname, token string, obj interface{}) error {
	return fmt.Errorf("create operation not supported for LbPolicy object")
}

func restPutLbPolicy(hostname, token string, obj interface{}) error {
	return fmt.Errorf("put operation not supported for LbPolicy object")
}

func restGetVirtualRouter(hostname, tenant, token string, obj interface{}) error {

	restcl, err := apiclient.NewRestAPIClient(hostname)
	if err != nil {
		return fmt.Errorf("cannot create REST client")
	}
	defer restcl.Close()
	loginCtx := loginctx.NewContextWithAuthzHeader(context.Background(), "Bearer "+token)

	if v, ok := obj.(*network.VirtualRouter); ok {
		nv, err := restcl.NetworkV1().VirtualRouter().Get(loginCtx, &v.ObjectMeta)
		if err != nil {
			return err
		}
		*v = *nv
	}

	if v, ok := obj.(*network.VirtualRouterList); ok {
		opts := api.ListWatchOptions{ObjectMeta: api.ObjectMeta{Tenant: tenant}}
		nv, err := restcl.NetworkV1().VirtualRouter().List(loginCtx, &opts)
		if err != nil {
			return err
		}
		v.Items = nv
	}
	return nil

}

func restDeleteVirtualRouter(hostname, token string, obj interface{}) error {

	restcl, err := apiclient.NewRestAPIClient(hostname)
	if err != nil {
		return fmt.Errorf("cannot create REST client")
	}
	defer restcl.Close()
	loginCtx := loginctx.NewContextWithAuthzHeader(context.Background(), "Bearer "+token)

	if v, ok := obj.(*network.VirtualRouter); ok {
		nv, err := restcl.NetworkV1().VirtualRouter().Delete(loginCtx, &v.ObjectMeta)
		if err != nil {
			return err
		}
		*v = *nv
	}
	return nil

}

func restPostVirtualRouter(hostname, token string, obj interface{}) error {

	restcl, err := apiclient.NewRestAPIClient(hostname)
	if err != nil {
		return fmt.Errorf("cannot create REST client")
	}
	defer restcl.Close()
	loginCtx := loginctx.NewContextWithAuthzHeader(context.Background(), "Bearer "+token)

	if v, ok := obj.(*network.VirtualRouter); ok {
		nv, err := restcl.NetworkV1().VirtualRouter().Create(loginCtx, v)
		if err != nil {
			return err
		}
		*v = *nv
	}
	return nil

}

func restPutVirtualRouter(hostname, token string, obj interface{}) error {

	restcl, err := apiclient.NewRestAPIClient(hostname)
	if err != nil {
		return fmt.Errorf("cannot create REST client")
	}
	defer restcl.Close()
	loginCtx := loginctx.NewContextWithAuthzHeader(context.Background(), "Bearer "+token)

	if v, ok := obj.(*network.VirtualRouter); ok {
		nv, err := restcl.NetworkV1().VirtualRouter().Update(loginCtx, v)
		if err != nil {
			return err
		}
		*v = *nv
	}
	return nil

}

func restGetNetworkInterface(hostname, tenant, token string, obj interface{}) error {

	restcl, err := apiclient.NewRestAPIClient(hostname)
	if err != nil {
		return fmt.Errorf("cannot create REST client")
	}
	defer restcl.Close()
	loginCtx := loginctx.NewContextWithAuthzHeader(context.Background(), "Bearer "+token)

	if v, ok := obj.(*network.NetworkInterface); ok {
		nv, err := restcl.NetworkV1().NetworkInterface().Get(loginCtx, &v.ObjectMeta)
		if err != nil {
			return err
		}
		*v = *nv
	}

	if v, ok := obj.(*network.NetworkInterfaceList); ok {
		opts := api.ListWatchOptions{ObjectMeta: api.ObjectMeta{Tenant: tenant}}
		nv, err := restcl.NetworkV1().NetworkInterface().List(loginCtx, &opts)
		if err != nil {
			return err
		}
		v.Items = nv
	}
	return nil

}

func restDeleteNetworkInterface(hostname, token string, obj interface{}) error {
	return fmt.Errorf("delete operation not supported for NetworkInterface object")
}

func restPostNetworkInterface(hostname, token string, obj interface{}) error {
	return fmt.Errorf("create operation not supported for NetworkInterface object")
}

func restPutNetworkInterface(hostname, token string, obj interface{}) error {

	restcl, err := apiclient.NewRestAPIClient(hostname)
	if err != nil {
		return fmt.Errorf("cannot create REST client")
	}
	defer restcl.Close()
	loginCtx := loginctx.NewContextWithAuthzHeader(context.Background(), "Bearer "+token)

	if v, ok := obj.(*network.NetworkInterface); ok {
		nv, err := restcl.NetworkV1().NetworkInterface().Update(loginCtx, v)
		if err != nil {
			return err
		}
		*v = *nv
	}
	return nil

}

func restGetIPAMPolicy(hostname, tenant, token string, obj interface{}) error {

	restcl, err := apiclient.NewRestAPIClient(hostname)
	if err != nil {
		return fmt.Errorf("cannot create REST client")
	}
	defer restcl.Close()
	loginCtx := loginctx.NewContextWithAuthzHeader(context.Background(), "Bearer "+token)

	if v, ok := obj.(*network.IPAMPolicy); ok {
		nv, err := restcl.NetworkV1().IPAMPolicy().Get(loginCtx, &v.ObjectMeta)
		if err != nil {
			return err
		}
		*v = *nv
	}

	if v, ok := obj.(*network.IPAMPolicyList); ok {
		opts := api.ListWatchOptions{ObjectMeta: api.ObjectMeta{Tenant: tenant}}
		nv, err := restcl.NetworkV1().IPAMPolicy().List(loginCtx, &opts)
		if err != nil {
			return err
		}
		v.Items = nv
	}
	return nil

}

func restDeleteIPAMPolicy(hostname, token string, obj interface{}) error {

	restcl, err := apiclient.NewRestAPIClient(hostname)
	if err != nil {
		return fmt.Errorf("cannot create REST client")
	}
	defer restcl.Close()
	loginCtx := loginctx.NewContextWithAuthzHeader(context.Background(), "Bearer "+token)

	if v, ok := obj.(*network.IPAMPolicy); ok {
		nv, err := restcl.NetworkV1().IPAMPolicy().Delete(loginCtx, &v.ObjectMeta)
		if err != nil {
			return err
		}
		*v = *nv
	}
	return nil

}

func restPostIPAMPolicy(hostname, token string, obj interface{}) error {

	restcl, err := apiclient.NewRestAPIClient(hostname)
	if err != nil {
		return fmt.Errorf("cannot create REST client")
	}
	defer restcl.Close()
	loginCtx := loginctx.NewContextWithAuthzHeader(context.Background(), "Bearer "+token)

	if v, ok := obj.(*network.IPAMPolicy); ok {
		nv, err := restcl.NetworkV1().IPAMPolicy().Create(loginCtx, v)
		if err != nil {
			return err
		}
		*v = *nv
	}
	return nil

}

func restPutIPAMPolicy(hostname, token string, obj interface{}) error {

	restcl, err := apiclient.NewRestAPIClient(hostname)
	if err != nil {
		return fmt.Errorf("cannot create REST client")
	}
	defer restcl.Close()
	loginCtx := loginctx.NewContextWithAuthzHeader(context.Background(), "Bearer "+token)

	if v, ok := obj.(*network.IPAMPolicy); ok {
		nv, err := restcl.NetworkV1().IPAMPolicy().Update(loginCtx, v)
		if err != nil {
			return err
		}
		*v = *nv
	}
	return nil

}

func restGetRoutingConfig(hostname, tenant, token string, obj interface{}) error {

	restcl, err := apiclient.NewRestAPIClient(hostname)
	if err != nil {
		return fmt.Errorf("cannot create REST client")
	}
	defer restcl.Close()
	loginCtx := loginctx.NewContextWithAuthzHeader(context.Background(), "Bearer "+token)

	if v, ok := obj.(*network.RoutingConfig); ok {
		nv, err := restcl.NetworkV1().RoutingConfig().Get(loginCtx, &v.ObjectMeta)
		if err != nil {
			return err
		}
		*v = *nv
	}

	if v, ok := obj.(*network.RoutingConfigList); ok {
		opts := api.ListWatchOptions{ObjectMeta: api.ObjectMeta{Tenant: tenant}}
		nv, err := restcl.NetworkV1().RoutingConfig().List(loginCtx, &opts)
		if err != nil {
			return err
		}
		v.Items = nv
	}
	return nil

}

func restDeleteRoutingConfig(hostname, token string, obj interface{}) error {

	restcl, err := apiclient.NewRestAPIClient(hostname)
	if err != nil {
		return fmt.Errorf("cannot create REST client")
	}
	defer restcl.Close()
	loginCtx := loginctx.NewContextWithAuthzHeader(context.Background(), "Bearer "+token)

	if v, ok := obj.(*network.RoutingConfig); ok {
		nv, err := restcl.NetworkV1().RoutingConfig().Delete(loginCtx, &v.ObjectMeta)
		if err != nil {
			return err
		}
		*v = *nv
	}
	return nil

}

func restPostRoutingConfig(hostname, token string, obj interface{}) error {

	restcl, err := apiclient.NewRestAPIClient(hostname)
	if err != nil {
		return fmt.Errorf("cannot create REST client")
	}
	defer restcl.Close()
	loginCtx := loginctx.NewContextWithAuthzHeader(context.Background(), "Bearer "+token)

	if v, ok := obj.(*network.RoutingConfig); ok {
		nv, err := restcl.NetworkV1().RoutingConfig().Create(loginCtx, v)
		if err != nil {
			return err
		}
		*v = *nv
	}
	return nil

}

func restPutRoutingConfig(hostname, token string, obj interface{}) error {

	restcl, err := apiclient.NewRestAPIClient(hostname)
	if err != nil {
		return fmt.Errorf("cannot create REST client")
	}
	defer restcl.Close()
	loginCtx := loginctx.NewContextWithAuthzHeader(context.Background(), "Bearer "+token)

	if v, ok := obj.(*network.RoutingConfig); ok {
		nv, err := restcl.NetworkV1().RoutingConfig().Update(loginCtx, v)
		if err != nil {
			return err
		}
		*v = *nv
	}
	return nil

}

func restGetRouteTable(hostname, tenant, token string, obj interface{}) error {

	restcl, err := apiclient.NewRestAPIClient(hostname)
	if err != nil {
		return fmt.Errorf("cannot create REST client")
	}
	defer restcl.Close()
	loginCtx := loginctx.NewContextWithAuthzHeader(context.Background(), "Bearer "+token)

	if v, ok := obj.(*network.RouteTable); ok {
		nv, err := restcl.NetworkV1().RouteTable().Get(loginCtx, &v.ObjectMeta)
		if err != nil {
			return err
		}
		*v = *nv
	}

	if v, ok := obj.(*network.RouteTableList); ok {
		opts := api.ListWatchOptions{ObjectMeta: api.ObjectMeta{Tenant: tenant}}
		nv, err := restcl.NetworkV1().RouteTable().List(loginCtx, &opts)
		if err != nil {
			return err
		}
		v.Items = nv
	}
	return nil

}

func restDeleteRouteTable(hostname, token string, obj interface{}) error {
	return fmt.Errorf("delete operation not supported for RouteTable object")
}

func restPostRouteTable(hostname, token string, obj interface{}) error {
	return fmt.Errorf("create operation not supported for RouteTable object")
}

func restPutRouteTable(hostname, token string, obj interface{}) error {
	return fmt.Errorf("put operation not supported for RouteTable object")
}

func init() {
	cl := gen.GetInfo()
	if cl == nil {
		return
	}

	cl.AddRestPostFunc("network.Network", "v1", restPostNetwork)
	cl.AddRestDeleteFunc("network.Network", "v1", restDeleteNetwork)
	cl.AddRestPutFunc("network.Network", "v1", restPutNetwork)
	cl.AddRestGetFunc("network.Network", "v1", restGetNetwork)

	cl.AddRestPostFunc("network.VirtualRouter", "v1", restPostVirtualRouter)
	cl.AddRestDeleteFunc("network.VirtualRouter", "v1", restDeleteVirtualRouter)
	cl.AddRestPutFunc("network.VirtualRouter", "v1", restPutVirtualRouter)
	cl.AddRestGetFunc("network.VirtualRouter", "v1", restGetVirtualRouter)

	cl.AddRestPutFunc("network.NetworkInterface", "v1", restPutNetworkInterface)
	cl.AddRestGetFunc("network.NetworkInterface", "v1", restGetNetworkInterface)

	cl.AddRestPostFunc("network.IPAMPolicy", "v1", restPostIPAMPolicy)
	cl.AddRestDeleteFunc("network.IPAMPolicy", "v1", restDeleteIPAMPolicy)
	cl.AddRestPutFunc("network.IPAMPolicy", "v1", restPutIPAMPolicy)
	cl.AddRestGetFunc("network.IPAMPolicy", "v1", restGetIPAMPolicy)

	cl.AddRestPostFunc("network.RoutingConfig", "v1", restPostRoutingConfig)
	cl.AddRestDeleteFunc("network.RoutingConfig", "v1", restDeleteRoutingConfig)
	cl.AddRestPutFunc("network.RoutingConfig", "v1", restPutRoutingConfig)
	cl.AddRestGetFunc("network.RoutingConfig", "v1", restGetRoutingConfig)

	cl.AddRestGetFunc("network.RouteTable", "v1", restGetRouteTable)

}
