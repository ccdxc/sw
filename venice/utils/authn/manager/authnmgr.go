package manager

import (
	"errors"
	"time"

	k8serrors "k8s.io/apimachinery/pkg/util/errors"

	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/venice/utils/authn"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
)

var (
	// ErrUserNotFound error is returned when no user is found when validating token
	ErrUserNotFound = errors.New("user not found")
)

// AuthenticationManager authenticates and returns user information
type AuthenticationManager struct {
	AuthGetter AuthGetter
	logger     log.Logger
}

// WithAuthGetter returns an instance of authentication manager
func WithAuthGetter(authGetter AuthGetter, logger log.Logger) (*AuthenticationManager, error) {
	return &AuthenticationManager{
		AuthGetter: authGetter,
		logger:     logger,
	}, nil
}

// NewAuthenticationManager returns an instance of AuthenticationManager
func NewAuthenticationManager(name, apiServer string, rslver resolver.Interface, logger log.Logger) (*AuthenticationManager, error) {
	if logger == nil {
		logger = log.GetNewLogger(log.GetDefaultConfig(name))
	}
	return &AuthenticationManager{
		AuthGetter: GetAuthGetter(name, apiServer, rslver, logger), // get singleton user cache
		logger:     logger,
	}, nil
}

// Authenticate authenticates user using authenticators in the order defined in AuthenticationPolicy. If any authenticator succeeds, it doesn't try the remaining authenticators.
func (authnmgr *AuthenticationManager) Authenticate(credential authn.Credential) (*auth.User, bool, error) {
	authenticators, err := authnmgr.AuthGetter.GetAuthenticators()
	if err != nil {
		return nil, false, err
	}
	var errlist []error
	for _, authenticator := range authenticators {
		user, ok, err := authenticator.Authenticate(credential)
		if ok {
			return user, ok, err
		}
		if err != nil {
			errlist = append(errlist, err)
		}
	}
	return nil, false, k8serrors.NewAggregate(errlist)
}

// CreateToken creates session token. It should be called only after successful authentication. It saves passed in objects in the session.
// Objects should support JSON serialization.
func (authnmgr *AuthenticationManager) CreateToken(user *auth.User, objects map[string]interface{}) (string, time.Time, error) {
	tokenManager, err := authnmgr.AuthGetter.GetTokenManager()
	if err != nil {
		return "", time.Time{}, err
	}
	return tokenManager.CreateToken(user, objects)
}

// ValidateToken validates session token and checks if it has not expired.
// Upon successful validation
// Returns
//  user information
//  true if token/session is valid/not expired
//  CSRF synchronizer token
func (authnmgr *AuthenticationManager) ValidateToken(token string) (*auth.User, bool, string, error) {
	tokenManager, err := authnmgr.AuthGetter.GetTokenManager()
	if err != nil {
		return nil, false, "", err
	}
	tokenInfo, ok, err := tokenManager.ValidateToken(token)
	if !ok {
		return nil, ok, "", err
	}
	// get username
	username, ok := tokenInfo[SubClaim].(string)
	if !ok {
		authnmgr.logger.Errorf("username is not of type string: {%+v}", username)
		return nil, ok, "", ErrUserNotFound
	}
	// get tenant if present
	tenant, _ := tokenInfo[TenantClaim].(string)

	// get user from cache
	user, ok := authnmgr.AuthGetter.GetUser(username, tenant)
	if !ok {
		authnmgr.logger.Errorf("User [%s] in tenant [%s] not found in cache", username, tenant)
		return nil, ok, "", ErrUserNotFound
	}
	// get csrf token if present
	csrfToken, _ := tokenInfo[CsrfClaim].(string)
	return user, ok, csrfToken, nil
}

// GetAuthGetter returns AuthGetter
func (authnmgr *AuthenticationManager) GetAuthGetter() AuthGetter {
	return authnmgr.AuthGetter
}

// ParseErrors returns a list of errors from aggregate error returned by Authenticate call
func (authnmgr *AuthenticationManager) ParseErrors(err error) []error {
	aggregate, ok := err.(k8serrors.Aggregate)
	if !ok {
		return []error{err}
	}
	return aggregate.Errors()
}
