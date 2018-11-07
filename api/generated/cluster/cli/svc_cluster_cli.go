// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

/*
Package clusterCliUtilsBackend is a auto generated package.
Input file: svc_cluster.proto
*/
package cli

import (
	"context"
	"fmt"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/cluster"
	loginctx "github.com/pensando/sw/api/login/context"
	"github.com/pensando/sw/venice/cli/gen"
)

func restGetCluster(hostname, tenant, token string, obj interface{}) error {
	return fmt.Errorf("get operation not supported for Cluster object")
}

func restDeleteCluster(hostname, token string, obj interface{}) error {

	restcl, err := apiclient.NewRestAPIClient(hostname)
	if err != nil {
		return fmt.Errorf("cannot create REST client")
	}
	loginCtx := loginctx.NewContextWithAuthzHeader(context.Background(), "Bearer "+token)

	if v, ok := obj.(*cluster.Cluster); ok {
		nv, err := restcl.ClusterV1().Cluster().Delete(loginCtx, &v.ObjectMeta)
		if err != nil {
			return err
		}
		*v = *nv
	}
	return nil

}

func restPostCluster(hostname, token string, obj interface{}) error {
	return fmt.Errorf("create operation not supported for Cluster object")
}

func restPutCluster(hostname, token string, obj interface{}) error {

	restcl, err := apiclient.NewRestAPIClient(hostname)
	if err != nil {
		return fmt.Errorf("cannot create REST client")
	}
	loginCtx := loginctx.NewContextWithAuthzHeader(context.Background(), "Bearer "+token)

	if v, ok := obj.(*cluster.Cluster); ok {
		nv, err := restcl.ClusterV1().Cluster().Update(loginCtx, v)
		if err != nil {
			return err
		}
		*v = *nv
	}
	return nil

}

func restGetNode(hostname, tenant, token string, obj interface{}) error {

	restcl, err := apiclient.NewRestAPIClient(hostname)
	if err != nil {
		return fmt.Errorf("cannot create REST client")
	}
	loginCtx := loginctx.NewContextWithAuthzHeader(context.Background(), "Bearer "+token)

	if v, ok := obj.(*cluster.Node); ok {
		nv, err := restcl.ClusterV1().Node().Get(loginCtx, &v.ObjectMeta)
		if err != nil {
			return err
		}
		*v = *nv
	}

	if v, ok := obj.(*cluster.NodeList); ok {
		opts := api.ListWatchOptions{ObjectMeta: api.ObjectMeta{Tenant: tenant}}
		nv, err := restcl.ClusterV1().Node().List(loginCtx, &opts)
		if err != nil {
			return err
		}
		v.Items = nv
	}
	return nil

}

func restDeleteNode(hostname, token string, obj interface{}) error {

	restcl, err := apiclient.NewRestAPIClient(hostname)
	if err != nil {
		return fmt.Errorf("cannot create REST client")
	}
	loginCtx := loginctx.NewContextWithAuthzHeader(context.Background(), "Bearer "+token)

	if v, ok := obj.(*cluster.Node); ok {
		nv, err := restcl.ClusterV1().Node().Delete(loginCtx, &v.ObjectMeta)
		if err != nil {
			return err
		}
		*v = *nv
	}
	return nil

}

func restPostNode(hostname, token string, obj interface{}) error {

	restcl, err := apiclient.NewRestAPIClient(hostname)
	if err != nil {
		return fmt.Errorf("cannot create REST client")
	}
	loginCtx := loginctx.NewContextWithAuthzHeader(context.Background(), "Bearer "+token)

	if v, ok := obj.(*cluster.Node); ok {
		nv, err := restcl.ClusterV1().Node().Create(loginCtx, v)
		if err != nil {
			return err
		}
		*v = *nv
	}
	return nil

}

func restPutNode(hostname, token string, obj interface{}) error {

	restcl, err := apiclient.NewRestAPIClient(hostname)
	if err != nil {
		return fmt.Errorf("cannot create REST client")
	}
	loginCtx := loginctx.NewContextWithAuthzHeader(context.Background(), "Bearer "+token)

	if v, ok := obj.(*cluster.Node); ok {
		nv, err := restcl.ClusterV1().Node().Update(loginCtx, v)
		if err != nil {
			return err
		}
		*v = *nv
	}
	return nil

}

func restGetHost(hostname, tenant, token string, obj interface{}) error {

	restcl, err := apiclient.NewRestAPIClient(hostname)
	if err != nil {
		return fmt.Errorf("cannot create REST client")
	}
	loginCtx := loginctx.NewContextWithAuthzHeader(context.Background(), "Bearer "+token)

	if v, ok := obj.(*cluster.Host); ok {
		nv, err := restcl.ClusterV1().Host().Get(loginCtx, &v.ObjectMeta)
		if err != nil {
			return err
		}
		*v = *nv
	}

	if v, ok := obj.(*cluster.HostList); ok {
		opts := api.ListWatchOptions{ObjectMeta: api.ObjectMeta{Tenant: tenant}}
		nv, err := restcl.ClusterV1().Host().List(loginCtx, &opts)
		if err != nil {
			return err
		}
		v.Items = nv
	}
	return nil

}

func restDeleteHost(hostname, token string, obj interface{}) error {

	restcl, err := apiclient.NewRestAPIClient(hostname)
	if err != nil {
		return fmt.Errorf("cannot create REST client")
	}
	loginCtx := loginctx.NewContextWithAuthzHeader(context.Background(), "Bearer "+token)

	if v, ok := obj.(*cluster.Host); ok {
		nv, err := restcl.ClusterV1().Host().Delete(loginCtx, &v.ObjectMeta)
		if err != nil {
			return err
		}
		*v = *nv
	}
	return nil

}

func restPostHost(hostname, token string, obj interface{}) error {

	restcl, err := apiclient.NewRestAPIClient(hostname)
	if err != nil {
		return fmt.Errorf("cannot create REST client")
	}
	loginCtx := loginctx.NewContextWithAuthzHeader(context.Background(), "Bearer "+token)

	if v, ok := obj.(*cluster.Host); ok {
		nv, err := restcl.ClusterV1().Host().Create(loginCtx, v)
		if err != nil {
			return err
		}
		*v = *nv
	}
	return nil

}

func restPutHost(hostname, token string, obj interface{}) error {

	restcl, err := apiclient.NewRestAPIClient(hostname)
	if err != nil {
		return fmt.Errorf("cannot create REST client")
	}
	loginCtx := loginctx.NewContextWithAuthzHeader(context.Background(), "Bearer "+token)

	if v, ok := obj.(*cluster.Host); ok {
		nv, err := restcl.ClusterV1().Host().Update(loginCtx, v)
		if err != nil {
			return err
		}
		*v = *nv
	}
	return nil

}

func restGetSmartNIC(hostname, tenant, token string, obj interface{}) error {

	restcl, err := apiclient.NewRestAPIClient(hostname)
	if err != nil {
		return fmt.Errorf("cannot create REST client")
	}
	loginCtx := loginctx.NewContextWithAuthzHeader(context.Background(), "Bearer "+token)

	if v, ok := obj.(*cluster.SmartNIC); ok {
		nv, err := restcl.ClusterV1().SmartNIC().Get(loginCtx, &v.ObjectMeta)
		if err != nil {
			return err
		}
		*v = *nv
	}

	if v, ok := obj.(*cluster.SmartNICList); ok {
		opts := api.ListWatchOptions{ObjectMeta: api.ObjectMeta{Tenant: tenant}}
		nv, err := restcl.ClusterV1().SmartNIC().List(loginCtx, &opts)
		if err != nil {
			return err
		}
		v.Items = nv
	}
	return nil

}

func restDeleteSmartNIC(hostname, token string, obj interface{}) error {

	restcl, err := apiclient.NewRestAPIClient(hostname)
	if err != nil {
		return fmt.Errorf("cannot create REST client")
	}
	loginCtx := loginctx.NewContextWithAuthzHeader(context.Background(), "Bearer "+token)

	if v, ok := obj.(*cluster.SmartNIC); ok {
		nv, err := restcl.ClusterV1().SmartNIC().Delete(loginCtx, &v.ObjectMeta)
		if err != nil {
			return err
		}
		*v = *nv
	}
	return nil

}

func restPostSmartNIC(hostname, token string, obj interface{}) error {
	return fmt.Errorf("create operation not supported for SmartNIC object")
}

func restPutSmartNIC(hostname, token string, obj interface{}) error {

	restcl, err := apiclient.NewRestAPIClient(hostname)
	if err != nil {
		return fmt.Errorf("cannot create REST client")
	}
	loginCtx := loginctx.NewContextWithAuthzHeader(context.Background(), "Bearer "+token)

	if v, ok := obj.(*cluster.SmartNIC); ok {
		nv, err := restcl.ClusterV1().SmartNIC().Update(loginCtx, v)
		if err != nil {
			return err
		}
		*v = *nv
	}
	return nil

}

func restGetTenant(hostname, tenant, token string, obj interface{}) error {

	restcl, err := apiclient.NewRestAPIClient(hostname)
	if err != nil {
		return fmt.Errorf("cannot create REST client")
	}
	loginCtx := loginctx.NewContextWithAuthzHeader(context.Background(), "Bearer "+token)

	if v, ok := obj.(*cluster.Tenant); ok {
		nv, err := restcl.ClusterV1().Tenant().Get(loginCtx, &v.ObjectMeta)
		if err != nil {
			return err
		}
		*v = *nv
	}

	if v, ok := obj.(*cluster.TenantList); ok {
		opts := api.ListWatchOptions{ObjectMeta: api.ObjectMeta{Tenant: tenant}}
		nv, err := restcl.ClusterV1().Tenant().List(loginCtx, &opts)
		if err != nil {
			return err
		}
		v.Items = nv
	}
	return nil

}

func restDeleteTenant(hostname, token string, obj interface{}) error {

	restcl, err := apiclient.NewRestAPIClient(hostname)
	if err != nil {
		return fmt.Errorf("cannot create REST client")
	}
	loginCtx := loginctx.NewContextWithAuthzHeader(context.Background(), "Bearer "+token)

	if v, ok := obj.(*cluster.Tenant); ok {
		nv, err := restcl.ClusterV1().Tenant().Delete(loginCtx, &v.ObjectMeta)
		if err != nil {
			return err
		}
		*v = *nv
	}
	return nil

}

func restPostTenant(hostname, token string, obj interface{}) error {

	restcl, err := apiclient.NewRestAPIClient(hostname)
	if err != nil {
		return fmt.Errorf("cannot create REST client")
	}
	loginCtx := loginctx.NewContextWithAuthzHeader(context.Background(), "Bearer "+token)

	if v, ok := obj.(*cluster.Tenant); ok {
		nv, err := restcl.ClusterV1().Tenant().Create(loginCtx, v)
		if err != nil {
			return err
		}
		*v = *nv
	}
	return nil

}

func restPutTenant(hostname, token string, obj interface{}) error {

	restcl, err := apiclient.NewRestAPIClient(hostname)
	if err != nil {
		return fmt.Errorf("cannot create REST client")
	}
	loginCtx := loginctx.NewContextWithAuthzHeader(context.Background(), "Bearer "+token)

	if v, ok := obj.(*cluster.Tenant); ok {
		nv, err := restcl.ClusterV1().Tenant().Update(loginCtx, v)
		if err != nil {
			return err
		}
		*v = *nv
	}
	return nil

}

func init() {
	cl := gen.GetInfo()
	if cl == nil {
		return
	}

	cl.AddRestDeleteFunc("cluster.Cluster", "v1", restDeleteCluster)
	cl.AddRestPutFunc("cluster.Cluster", "v1", restPutCluster)
	cl.AddRestGetFunc("cluster.Cluster", "v1", restGetCluster)

	cl.AddRestPostFunc("cluster.Node", "v1", restPostNode)
	cl.AddRestDeleteFunc("cluster.Node", "v1", restDeleteNode)
	cl.AddRestPutFunc("cluster.Node", "v1", restPutNode)
	cl.AddRestGetFunc("cluster.Node", "v1", restGetNode)

	cl.AddRestPostFunc("cluster.Host", "v1", restPostHost)
	cl.AddRestDeleteFunc("cluster.Host", "v1", restDeleteHost)
	cl.AddRestPutFunc("cluster.Host", "v1", restPutHost)
	cl.AddRestGetFunc("cluster.Host", "v1", restGetHost)

	cl.AddRestDeleteFunc("cluster.SmartNIC", "v1", restDeleteSmartNIC)
	cl.AddRestPutFunc("cluster.SmartNIC", "v1", restPutSmartNIC)
	cl.AddRestGetFunc("cluster.SmartNIC", "v1", restGetSmartNIC)

	cl.AddRestPostFunc("cluster.Tenant", "v1", restPostTenant)
	cl.AddRestDeleteFunc("cluster.Tenant", "v1", restDeleteTenant)
	cl.AddRestPutFunc("cluster.Tenant", "v1", restPutTenant)
	cl.AddRestGetFunc("cluster.Tenant", "v1", restGetTenant)

}
