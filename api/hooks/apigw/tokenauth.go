// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package impl

import (
	"context"
	"errors"

	"github.com/pensando/sw/api/generated/audit"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/tokenauth"
	"github.com/pensando/sw/venice/apigw"
	apigwpkg "github.com/pensando/sw/venice/apigw/pkg"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/authz"
	"github.com/pensando/sw/venice/utils/authz/rbac"
	"github.com/pensando/sw/venice/utils/log"
)

type tokenAuthHooks struct {
	permissionGetter rbac.PermissionGetter
	logger           log.Logger
}

// operations is a pre authz hook to determine authz.Operation
func (e *tokenAuthHooks) operations(ctx context.Context, in interface{}) (context.Context, interface{}, error) {
	e.logger.Debugf("APIGw tokenAuth operations hook called for obj [%#v]", in)
	user, ok := apigwpkg.UserFromContext(ctx)
	if !ok || user == nil {
		e.logger.Errorf("no user present in context passed to TokenAuth operations authz hook")
		return ctx, in, apigwpkg.ErrNoUserInContext
	}
	var resource authz.Resource
	switch in.(type) {
	case *tokenauth.NodeTokenRequest:
		resource = authz.NewResource(
			user.Tenant,
			"",
			auth.Permission_TokenAuth.String(),
			globals.DefaultNamespace,
			"")
	default:
		return ctx, in, errors.New("invalid input type")
	}
	// get existing operations from context
	operations, _ := apigwpkg.OperationsFromContext(ctx)
	// append requested operation
	operations = append(operations, authz.NewOperation(resource, auth.Permission_Read.String()))

	// By default we no user has this permission, which means that only Global Admin can access the API
	// However, Global Admin can grant the permission to other users if needed.

	nctx := apigwpkg.NewContextWithOperations(ctx, operations...)
	return nctx, in, nil
}

func (e *tokenAuthHooks) registerTokenAuthHooks(svc apigw.APIGatewayService) error {
	methods := []string{"GenerateNodeToken"}
	for _, method := range methods {
		prof, err := svc.GetServiceProfile(method)
		if err != nil {
			return err
		}
		prof.AddPreAuthZHook(e.operations)
		prof.SetAuditLevel(audit.Level_Request.String()) // not recording response as it has private key
	}
	return nil
}

func registerTokenAuthHooks(svc apigw.APIGatewayService, l log.Logger) error {
	gw := apigwpkg.MustGetAPIGateway()
	grpcaddr := gw.GetAPIServerAddr(globals.APIServer)
	r := &tokenAuthHooks{
		permissionGetter: rbac.GetPermissionGetter(globals.APIGw, grpcaddr, gw.GetResolver()),
		logger:           l,
	}
	return r.registerTokenAuthHooks(svc)
}

func init() {
	gw := apigwpkg.MustGetAPIGateway()
	gw.RegisterHooksCb("tokenauth.TokenAuthV1", registerTokenAuthHooks)
}
