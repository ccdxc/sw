package impl

import (
	"context"
	"errors"

	apigwpkg "github.com/pensando/sw/venice/apigw/pkg"
	"github.com/pensando/sw/venice/utils/authz"

	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/venice/globals"
	authzgrpcctx "github.com/pensando/sw/venice/utils/authz/grpc/context"
	"github.com/pensando/sw/venice/utils/authz/rbac"
	"github.com/pensando/sw/venice/utils/log"
)

func newContextWithUserPerms(ctx context.Context, permGetter rbac.PermissionGetter, logger log.Logger) (context.Context, error) {
	user, ok := apigwpkg.UserFromContext(ctx)
	if !ok || user == nil {
		logger.ErrorLog("method", "setUserContext", "msg", "no user present in context")
		return ctx, apigwpkg.ErrNoUserInContext
	}
	perms := permGetter.GetPermissions(user)
	isAdmin := isGlobalAdmin(user, permGetter)

	nctx, err := authzgrpcctx.NewOutgoingContextWithUserPerms(ctx, user, isAdmin, perms)
	if err != nil {
		logger.ErrorLog("method", "setUserContext", "msg", "error creating outgoing context with user permissions", "user", user.Name, "tenant", user.Tenant, "err", err)
		return ctx, err
	}
	return nctx, nil
}

func isGlobalAdmin(user *auth.User, permGetter rbac.PermissionGetter) bool {
	if user.Tenant == globals.DefaultTenant {
		if roles := permGetter.GetRolesForUser(user); len(roles) > 0 {
			for _, role := range roles {
				if role.Name == globals.AdminRole {
					return true
				}
			}
		}
	}
	return false
}

func addOwnerToContext(ctx context.Context, user *auth.User, logger log.Logger) (context.Context, error) {
	// get existing operations from context
	operations, ok := apigwpkg.OperationsFromContext(ctx)

	if !ok || operations == nil {
		logger.ErrorLog("method", "addOwner", "msg", "addOwnerToContext failed, operation not present in context")
		return ctx, errors.New("internal error")
	}

	for _, operation := range operations {
		if !authz.IsValidOperationValue(operation) {
			logger.ErrorLog("method", "addOwner", "msg", "addOwnerToContext failed, invalid operation: %v", operation)
			return ctx, errors.New("internal error")
		}
		resource := operation.GetResource()
		resource.SetOwner(user)
	}
	nctx := apigwpkg.NewContextWithOperations(ctx, operations...)
	return nctx, nil
}
