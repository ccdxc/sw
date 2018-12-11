package impl

import (
	"context"
	"errors"

	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/search"
	"github.com/pensando/sw/api/generated/security"
	"github.com/pensando/sw/venice/apigw"
	"github.com/pensando/sw/venice/apigw/pkg"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/authz"
	authzgrpcctx "github.com/pensando/sw/venice/utils/authz/grpc/context"
	"github.com/pensando/sw/venice/utils/authz/rbac"
	"github.com/pensando/sw/venice/utils/log"
)

type searchHooks struct {
	permissionGetter rbac.PermissionGetter
	logger           log.Logger
}

// operations is a pre authz hook to determine authz.Operation
func (e *searchHooks) operations(ctx context.Context, in interface{}) (context.Context, interface{}, error) {
	e.logger.Debugf("APIGw search operations authz hook called for obj [%#v]", in)
	user, ok := apigwpkg.UserFromContext(ctx)
	if !ok || user == nil {
		e.logger.Errorf("no user present in context passed to search operations authz hook")
		return ctx, in, apigwpkg.ErrNoUserInContext
	}
	// get existing operations from context
	operations, _ := apigwpkg.OperationsFromContext(ctx)
	switch obj := in.(type) {
	// check read authorization for sg policy included in policy search request
	case *search.PolicySearchRequest:
		if obj.Tenant == "" {
			// scoping the tenant to be user tenant for policy search request; namespace defaults to "default" if not specified (proto definition)
			obj.Tenant = user.GetTenant()
		}
		resource := authz.NewResource(obj.Tenant, string(apiclient.GroupSecurity), string(security.KindSGPolicy), obj.Namespace, obj.SGPolicy)
		operations = append(operations, authz.NewOperation(resource, auth.Permission_Read.String()))
		resource = authz.NewResource(
			obj.Tenant,
			"",
			auth.Permission_Search.String(),
			"",
			"")
		operations = append(operations, authz.NewOperation(resource, auth.Permission_Read.String()))
	case *search.SearchRequest:
		resource := authz.NewResource(
			user.Tenant,
			"",
			auth.Permission_Search.String(),
			"",
			"")
		operations = append(operations, authz.NewOperation(resource, auth.Permission_Read.String()))
	default:
		return ctx, in, errors.New("invalid input type")
	}
	nctx := apigwpkg.NewContextWithOperations(ctx, operations...)
	return nctx, in, nil
}

// userContext is a pre-call hook to set user and permissions in grpc metadata in outgoing context and to add user tenant to search scope if none is specified in search request
func (e *searchHooks) userContext(ctx context.Context, in interface{}) (context.Context, interface{}, bool, error) {
	e.logger.DebugLog("msg", "APIGw userContext pre-call hook called")
	user, ok := apigwpkg.UserFromContext(ctx)
	if !ok || user == nil {
		e.logger.Errorf("no user present in context passed to userContext pre-call hook")
		return ctx, in, true, apigwpkg.ErrNoUserInContext
	}
	switch obj := in.(type) {
	// check read authorization for sg policy included in policy search request
	case *search.PolicySearchRequest:
		if obj.Tenant == "" {
			// scoping the tenant to be user tenant for policy search request
			obj.Tenant = user.GetTenant()
		}
	case *search.SearchRequest:
		if len(obj.Tenants) == 0 {
			obj.Tenants = append(obj.Tenants, user.Tenant)
		}
	default:
		return ctx, in, true, errors.New("invalid input type")
	}
	perms := e.permissionGetter.GetPermissions(user)
	nctx, err := authzgrpcctx.NewOutgoingContextWithUserPerms(ctx, user, perms)
	if err != nil {
		e.logger.Errorf("error creating outgoing context with user permissions for user [%s|%s]: %v", user.Tenant, user.Name, err)
		return ctx, in, true, err
	}
	return nctx, in, false, nil
}

func registerSearchHooks(svc apigw.APIGatewayService, l log.Logger) error {
	gw := apigwpkg.MustGetAPIGateway()
	grpcaddr := globals.APIServer
	grpcaddr = gw.GetAPIServerAddr(grpcaddr)
	r := &searchHooks{
		permissionGetter: rbac.GetPermissionGetter(globals.APIGw, grpcaddr, gw.GetResolver()),
		logger:           l,
	}
	methods := []string{"Query", "PolicyQuery"}
	for _, method := range methods {
		prof, err := svc.GetServiceProfile(method)
		if err != nil {
			return err
		}
		prof.AddPreAuthZHook(r.operations)
		prof.AddPreCallHook(r.userContext)
	}
	return nil
}

func init() {
	gw := apigwpkg.MustGetAPIGateway()
	gw.RegisterHooksCb("search.SearchV1", registerSearchHooks)
}
