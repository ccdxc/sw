package authn

import (
	"errors"

	"github.com/pensando/sw/api/generated/auth"
)

var (
	// ErrInvalidCredentialType error is returned when an incorrect credential type is passed to an authenticator
	ErrInvalidCredentialType = errors.New("incorrect credential type")
	// ErrNoGroupMembership error is returned when user does not belong to any group
	ErrNoGroupMembership = errors.New("user is not a member of any group")
)

// Credential is a marker interface representing credential of a user.
type Credential interface{}

// Authenticator represents an authentication strategy/mechanism used to authenticate user. For example, LDAP authenticator, local user/password authenticator
type Authenticator interface {
	// Authenticate authenticates user
	// Returns
	//  *auth.User user information
	//  bool true if authentication is successful
	//  error if there is any error authenticating user
	Authenticate(credential Credential) (*auth.User, bool, error)
}
