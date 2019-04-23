package grpc

import (
	"context"
	"errors"

	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/venice/utils/authz"
	grpcctx "github.com/pensando/sw/venice/utils/authz/grpc/context"
	"github.com/pensando/sw/venice/utils/authz/rbac"
)

type authorizer struct {
	ctx context.Context
	// user perms based authorizer
	authorizer authz.Authorizer
	// user from context
	user *auth.User
}

func (a *authorizer) IsAuthorized(user *auth.User, operations ...authz.Operation) (bool, error) {
	return a.authorizer.IsAuthorized(a.user, operations...)
}

func (a *authorizer) Stop() {}

func (a *authorizer) AuthorizedOperations(user *auth.User, tenant, namespace string, actionTypes ...auth.Permission_ActionType) []authz.Operation {
	return a.authorizer.AuthorizedOperations(user, tenant, namespace, actionTypes...)
}

// NewAuthorizer returns an authorizer to be used by grpc backend to check authorization based on permissions passed in grpc metadata
func NewAuthorizer(ctx context.Context) (authz.Authorizer, error) {
	userMeta, ok := grpcctx.UserMetaFromIncomingContext(ctx)
	if !ok || userMeta == nil {
		return nil, errors.New("no user in context")
	}
	user := &auth.User{
		ObjectMeta: *userMeta,
	}
	perms, ok, err := grpcctx.PermsFromIncomingContext(ctx)
	if err != nil {
		return nil, err
	}
	return &authorizer{ctx: ctx, authorizer: rbac.NewUserAuthorizer(perms), user: user}, nil
}
