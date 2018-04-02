package manager

import (
	"errors"

	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/venice/utils/authn"
)

const (
	/* standard claims */

	// SubClaim identifies the principal that is the subject of the token
	SubClaim = "sub"
	// IssuerClaim identifies the principal that issued the token
	IssuerClaim = "iss"
	// ExpClaim identifies the expiration time on or after which the token MUST NOT be accepted for processing
	ExpClaim = "exp"
	// IssuedAtClaim identifies the principal that issued the token
	IssuedAtClaim = "iat"
	// NotBeforeClaim identifies the time before which the token MUST NOT be accepted for processing
	NotBeforeClaim = "nbf"
	// AudienceClaim identifies the recipients that the token is intended for
	AudienceClaim = "aud"
	// IDClaim provides a unique identifier for the token
	IDClaim = "jti"

	/* private claims */

	// CsrfClaim has CSRF synchronizer token
	CsrfClaim = "csrf"
	// TenantClaim has user tenant
	TenantClaim = "tenant"
	// RolesClaim has user roles
	RolesClaim = "roles"
)

var (
	// ErrMissingUserInfo error is returned when no user information is provided while creating token
	ErrMissingUserInfo = errors.New("user information is required to create token")
)

// TokenManager is used to create token/session after successful user login or to validate token/session
type TokenManager interface {
	// CreateToken returns a token (JWT or session id) with provided key value map stored in JWT or session.
	// value is assumed to be JSON serializable.
	// Returns
	//   JWT token or unique session id
	CreateToken(*auth.User, map[string]interface{}) (string, error)

	// ValidateToken validates token.
	// Returns
	//   information stored for user in JWT/session
	//   true if token/session is valid/not expired.
	//   error
	ValidateToken(token string) (map[string]interface{}, bool, error)

	// Get returns object stored in session for the given key
	// Returns
	//   stored object
	//   true if object exists otherwise false
	//   error
	Get(token, key string) (interface{}, bool, error)
}

// AuthGetter abstracts out retrieval of authentication objects
type AuthGetter interface {
	// GetUser returns user given name and tenant
	GetUser(name, tenant string) (*auth.User, bool)

	// GetAuthenticationPolicy returns authentication policy
	GetAuthenticationPolicy() (*auth.AuthenticationPolicy, error)

	// GetAuthenticators returns authenticators based on authentication policy
	GetAuthenticators() ([]authn.Authenticator, error)

	// GetTokenManager returns an instance of TokenManager based on authentication policy
	GetTokenManager() (TokenManager, error)

	// Stop un-initializes AuthGetter
	Stop()

	// Start re-initializes AuthGetter. It blocks if AuthGetter has not been un-initialized through Stop()
	Start()
}
