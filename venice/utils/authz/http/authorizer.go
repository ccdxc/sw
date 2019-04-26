package http

import (
	"context"
	"errors"
	"net/http"

	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/venice/utils/authz"
	"github.com/pensando/sw/venice/utils/authz/rbac"
)

type authorizer struct {
	ctx context.Context
	req *http.Request
	// user perms based authorizer
	authorizer authz.Authorizer
	// user from context
	user *auth.User
}

func (a *authorizer) IsAuthorized(user *auth.User, operations ...authz.Operation) (bool, error) {
	if a.user.Name != user.Name || a.user.Tenant != user.Tenant {
		return false, nil
	}
	return a.authorizer.IsAuthorized(a.user, operations...)
}

func (a *authorizer) Stop() {}

func (a *authorizer) AuthorizedOperations(user *auth.User, tenant, namespace string, actionTypes ...auth.Permission_ActionType) []authz.Operation {
	if a.user.Name != user.Name || a.user.Tenant != user.Tenant {
		return nil
	}
	return a.authorizer.AuthorizedOperations(user, tenant, namespace, actionTypes...)
}

// NewAuthorizer returns an authorizer to be used by http backend to check authorization based on permissions passed in http request
func NewAuthorizer(ctx context.Context, request *http.Request) (authz.Authorizer, error) {
	userMeta, ok := UserMetaFromRequest(request)
	if !ok || userMeta == nil {
		return nil, errors.New("no user in context")
	}
	user := &auth.User{
		ObjectMeta: *userMeta,
	}
	perms, ok, err := PermsFromRequest(request)
	if err != nil {
		return nil, err
	}
	return &authorizer{ctx: ctx, req: request, authorizer: rbac.NewUserAuthorizer(perms), user: user}, nil
}
