package manager

import "github.com/pensando/sw/api/generated/auth"

//TokenManager is used to create token/session id after successful user login or to validate token/session id
type TokenManager interface {
	// CreateToken returns a token (JWT or session id) depending on chosen session manager implementation. JWT token includes user, user group, tenant and role information.
	// session id is unique id identifying user session
	CreateToken(*auth.User) (string, error)

	// ValidateToken validates token.
	// Returns
	//   true if token/session is valid/not expired.
	ValidateToken(string) (*auth.User, bool, error)
}
