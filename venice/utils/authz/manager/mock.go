package manager

import (
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/venice/utils/authz"
)

// alwaysAllowAuthorizer implements Authorizer interface. It always allows an operation.
type alwaysAllowAuthorizer struct{}

func (a *alwaysAllowAuthorizer) IsAuthorized(user *auth.User, operations ...authz.Operation) (bool, error) {
	return true, nil
}

// NewAlwaysAllowAuthorizer returns an Authorizer that always allows an operation
func NewAlwaysAllowAuthorizer() authz.Authorizer {
	return &alwaysAllowAuthorizer{}
}

// alwaysDenyAuthorizer implements Authorizer interface. It always denies an operation.
type alwaysDenyAuthorizer struct{}

func (a *alwaysDenyAuthorizer) IsAuthorized(user *auth.User, operations ...authz.Operation) (bool, error) {
	return false, nil
}

// NewAlwaysDenyAuthorizer returns an Authorizer that always denies an operation
func NewAlwaysDenyAuthorizer() authz.Authorizer {
	return &alwaysDenyAuthorizer{}
}
