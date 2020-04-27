package impl

import (
	"context"
	"errors"
	"fmt"
	"strings"

	"github.com/pensando/sw/api/generated/audit"
	"github.com/pensando/sw/api/generated/cluster"
	apiintf "github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/venice/apigw"
	apigwpkg "github.com/pensando/sw/venice/apigw/pkg"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/authz"
	"github.com/pensando/sw/venice/utils/authz/rbac"
	"github.com/pensando/sw/venice/utils/bootstrapper"
	"github.com/pensando/sw/venice/utils/log"
)

type clusterHooks struct {
	permissionGetter rbac.PermissionGetter
	bootstrapper     bootstrapper.Bootstrapper
	logger           log.Logger
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
func (a *clusterHooks) setAuthBootstrapFlag(ctx context.Context, in, out interface{}) (context.Context, interface{}, interface{}, bool, error) {
	a.logger.DebugLog("msg", "APIGw setAuthBootstrapFlag PreCallHook called")
	ok, err := a.bootstrapper.IsBootstrapped(globals.DefaultTenant, bootstrapper.Auth)
	if err != nil {
		return ctx, in, out, true, err
	}
	if !ok {
		return ctx, in, out, true, errors.New("cluster is not bootstrapped")
	}
	return ctx, in, out, false, nil
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

// userContext is a pre-call hook to set user and permissions in grpc metadata in outgoing context
func (a *clusterHooks) userContext(ctx context.Context, in, out interface{}) (context.Context, interface{}, interface{}, bool, error) {
	a.logger.DebugLog("msg", "APIGw userContext pre-call hook called")
	switch in.(type) {
	// check read authorization for sg policy included in policy search request
	case *cluster.Host:
	default:
		return ctx, in, out, true, errors.New("invalid input type")
	}
	nctx, err := newContextWithUserPerms(ctx, a.permissionGetter, a.logger)
	if err != nil {
		return ctx, in, out, true, err
	}
	return nctx, in, out, false, nil
}

func (a *clusterHooks) snapshotPostCallHook(ctx context.Context, out interface{}) (retCtx context.Context, retOut interface{}, err error) {
	obj, ok := out.(*cluster.ConfigurationSnapshot)
	if !ok {
		return ctx, out, errors.New("invalid input type")
	}
	if obj.Status.LastSnapshot == nil || obj.Status.LastSnapshot.URI == "" {
		a.logger.InfoLog("method", "snapshotPostCallHook", "msg", fmt.Sprintf("no snapshot information in ConfigurationSnapshot: %#v", *obj))
		return ctx, out, nil
	}
	nctx := ctx
	a.logger.DebugLog("method", "snapshotPostCallHook", "msg", fmt.Sprintf("ConfigurationSnapshot status: %#v", *obj.Status.LastSnapshot))
	s := strings.Split(obj.Status.LastSnapshot.URI, "/")
	filename := s[len(s)-1]
	a.logger.DebugLog("method", "snapshotPostCallHook", "msg", fmt.Sprintf("filename [%s] extracted from URI: %s", filename, obj.Status.LastSnapshot.URI))
	var nOps []authz.Operation
	operations, _ := apigwpkg.OperationsFromContext(ctx)
	for _, op := range operations {
		if op.GetResource().GetKind() == string(cluster.KindConfigurationSnapshot) {
			resource := op.GetResource()
			nOp := authz.NewOperationWithID(authz.NewResource(resource.GetTenant(), resource.GetGroup(), resource.GetKind(), resource.GetNamespace(), filename),
				op.GetAction(), op.GetAuditAction(), op.GetID())
			nOps = append(nOps, nOp)
		} else {
			nOps = append(nOps, op)
		}
	}
	nctx = apigwpkg.NewContextWithOperations(ctx, nOps...)
	obj.Name = filename
	return nctx, out, nil
}

func (a *clusterHooks) registerClusterHooks(svc apigw.APIGatewayService) error {
	prof, err := svc.GetCrudServiceProfile("Tenant", apiintf.CreateOper)
	if err != nil {
		return err
	}
	prof.AddPreAuthNHook(a.authBootstrap)

	// Host object - add user context to request for apiserver hook
	prof, err = svc.GetCrudServiceProfile("Host", apiintf.DeleteOper)
	prof.AddPreCallHook(a.userContext)

	// user should get and watch version without limitation
	prof, err = svc.GetCrudServiceProfile("Version", apiintf.GetOper)
	prof.AddPreAuthZHook(a.addOwner)
	prof, err = svc.GetCrudServiceProfile("Version", apiintf.WatchOper)
	prof.AddPreAuthZHook(a.addOwner)

	prof, err = svc.GetCrudServiceProfile("License", apiintf.CreateOper)
	prof.AddPreAuthNHook(a.checkFFBootstrap)
	prof, err = svc.GetCrudServiceProfile("License", apiintf.UpdateOper)
	prof.AddPreAuthNHook(a.checkFFBootstrap)

	prof, err = svc.GetServiceProfile("Save")
	if err != nil {
		panic("unknown service specified")
	}
	prof.SetAuditLevel(audit.Level_Response.String())
	prof.AddPostCallHook(a.snapshotPostCallHook)

	return a.registerSetAuthBootstrapFlagHook(svc)
}

func registerClusterHooks(svc apigw.APIGatewayService, l log.Logger) error {
	gw := apigwpkg.MustGetAPIGateway()
	grpcaddr := gw.GetAPIServerAddr(globals.APIServer)
	a := clusterHooks{
		bootstrapper:     bootstrapper.GetBootstrapper(),
		permissionGetter: rbac.GetPermissionGetter(globals.APIGw, grpcaddr, gw.GetResolver()),
		logger:           l,
	}
	return a.registerClusterHooks(svc)
}

func init() {
	gw := apigwpkg.MustGetAPIGateway()
	gw.RegisterHooksCb("cluster.ClusterV1", registerClusterHooks)
}
