package impl

import (
	"context"
	"errors"
	"fmt"

	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/venice/apigw"
	"github.com/pensando/sw/venice/apigw/pkg"
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

func (a *clusterHooks) addOwner(ctx context.Context, in interface{}) (context.Context, interface{}, error) {
	user, ok := apigwpkg.UserFromContext(ctx)
	if !ok || user == nil {
		a.logger.ErrorLog("method", "addOwner", "msg", "pre-authz hook failed, failed to get user info from context")
		return ctx, in, errors.New("internal error")
	}

	nctx, err := addOwnerToContext(ctx, user, a.logger)
	if err != nil {
		a.logger.ErrorLog("method", "addOwner", "msg", "pre-authz hook failed, failed add owner to context")
		return ctx, in, err
	}
	return nctx, in, nil
}

// checkFFBootstrap checks if the FeatureFlags can be updated
func (a *clusterHooks) checkFFBootstrap(ctx context.Context, in interface{}) (context.Context, interface{}, bool, error) {
	bs, err := a.bootstrapper.IsFlagSet(globals.DefaultTenant, bootstrapper.Auth)
	if err != nil {
		return ctx, in, false, err
	}
	if bs {
		return ctx, in, false, fmt.Errorf("updating feature flags allowed only during cluster bootstrapping")
	}
	return ctx, in, true, nil
}

func registerClusterHooks(svc apigw.APIGatewayService, l log.Logger) error {
	r := clusterHooks{
		bootstrapper: bootstrapper.GetBootstrapper(),
		logger:       l,
	}
	prof, err := svc.GetCrudServiceProfile("Tenant", apiintf.CreateOper)
	if err != nil {
		return err
	}
	prof.AddPreAuthNHook(r.authBootstrap)

	// user should get and watch version without limitation
	prof, err = svc.GetCrudServiceProfile("Version", apiintf.GetOper)
	prof.AddPreAuthZHook(r.addOwner)
	prof, err = svc.GetCrudServiceProfile("Version", apiintf.WatchOper)
	prof.AddPreAuthZHook(r.addOwner)

	prof, err = svc.GetCrudServiceProfile("License", apiintf.CreateOper)
	prof.AddPreAuthNHook(r.checkFFBootstrap)
	prof, err = svc.GetCrudServiceProfile("License", apiintf.UpdateOper)
	prof.AddPreAuthNHook(r.checkFFBootstrap)

	return r.registerSetAuthBootstrapFlagHook(svc)
}

func init() {
	gw := apigwpkg.MustGetAPIGateway()
	gw.RegisterHooksCb("cluster.ClusterV1", registerClusterHooks)
}
