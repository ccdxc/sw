package owner

import (
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/venice/utils/authz"
)

type authorizer struct {
	authz.AbstractAuthorizer
}

// NewOwnerAuthorizer authorizes an operation on the resource if user is the owner of the resource
func NewOwnerAuthorizer() authz.Authorizer {
	ownerAuthorizer := &authorizer{}
	ownerAuthorizer.AbstractAuthorizer.Authorizer = ownerAuthorizer
	return ownerAuthorizer
}

func (a *authorizer) IsAuthorized(user *auth.User, operations ...authz.Operation) (bool, error) {
	if user == nil || operations == nil {
		return false, nil
	}
	for _, operation := range operations {
		if !authz.IsValidOperationValue(operation) {
			return false, nil
		}
		owner := operation.GetResource().GetOwner()
		if owner == nil || owner.Tenant != user.Tenant || owner.Name != user.Name {
			return false, nil
		}
	}
	return true, nil
}
