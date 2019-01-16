package impl

import (
	"context"

	"github.com/pensando/sw/venice/apigw/pkg"
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
	nctx, err := authzgrpcctx.NewOutgoingContextWithUserPerms(ctx, user, perms)
	if err != nil {
		logger.ErrorLog("method", "setUserContext", "msg", "error creating outgoing context with user permissions", "user", user.Name, "tenant", user.Tenant, "error", err)
		return ctx, err
	}
	return nctx, nil
}
