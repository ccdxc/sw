package impl

import (
	"context"
	"errors"

	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/venice/apigw"
	"github.com/pensando/sw/venice/apigw/pkg"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/bootstrapper"
	"github.com/pensando/sw/venice/utils/log"
)

type clusterHooks struct {
	bootstrapper bootstrapper.Bootstrapper
	logger       log.Logger
}

func (a *clusterHooks) authBootstrap(ctx context.Context, i interface{}) (context.Context, interface{}, bool, error) {
	a.logger.DebugLog("msg", "APIGw authBootstrap PreAuthNHook called for cluster obj")
	switch obj := i.(type) {
	case *cluster.ClusterAuthBootstrapRequest:
	case *cluster.Tenant:
		if obj.GetName() != globals.DefaultTenant {
			return ctx, i, false, nil
		}
	default:
		return ctx, i, false, errors.New("invalid input type")
	}
	ok, err := a.bootstrapper.IsFlagSet(globals.DefaultTenant, bootstrapper.Auth)
	return ctx, i, !ok, err
}

// setAuthBootstrapFlag is a pre-call hook to check if cluster has actually been bootstrapped before setting the flag
func (a *clusterHooks) setAuthBootstrapFlag(ctx context.Context, in interface{}) (context.Context, interface{}, bool, error) {
	a.logger.DebugLog("msg", "APIGw setAuthBootstrapFlag PreCallHook called")
	ok, err := a.bootstrapper.IsBootstrapped(globals.DefaultTenant, bootstrapper.Auth)
	if err != nil {
		return ctx, in, true, err
	}
	if !ok {
		return ctx, in, true, errors.New("cluster is not bootstrapped")
	}
	return ctx, in, false, nil
}

func (a *clusterHooks) registerSetAuthBootstrapFlagHook(svc apigw.APIGatewayService) error {
	prof, err := svc.GetServiceProfile("AuthBootstrapComplete")
	if err != nil {
		return err
	}
	prof.AddPreAuthNHook(a.authBootstrap)
	prof.AddPreCallHook(a.setAuthBootstrapFlag)
	return nil
}

func registerClusterHooks(svc apigw.APIGatewayService, l log.Logger) error {
	r := clusterHooks{
		bootstrapper: bootstrapper.GetBootstrapper(),
		logger:       l,
	}
	prof, err := svc.GetCrudServiceProfile("Tenant", apiserver.CreateOper)
	if err != nil {
		return err
	}
	prof.AddPreAuthNHook(r.authBootstrap)
	return r.registerSetAuthBootstrapFlagHook(svc)
}

func init() {
	gw := apigwpkg.MustGetAPIGateway()
	gw.RegisterHooksCb("cluster.ClusterV1", registerClusterHooks)
}
