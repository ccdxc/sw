package manager

import (
	"context"
	"fmt"
	"strings"
	"time"

	k8serrors "k8s.io/apimachinery/pkg/util/errors"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/venice/utils/authn"
	"github.com/pensando/sw/venice/utils/authn/ldap"
	"github.com/pensando/sw/venice/utils/authn/password"
	"github.com/pensando/sw/venice/utils/balancer"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
)

// AuthenticationManager authenticates and returns user information
type AuthenticationManager struct {
	authenticators []authn.Authenticator
	tokenManager   TokenManager
	apicl          apiclient.Services
}

// NewAuthenticationManager returns an instance of AuthenticationManager
func NewAuthenticationManager(apiServer string, resolverUrls string, tokenExpiration time.Duration) (*AuthenticationManager, error) {
	l := log.WithContext("Pkg", "authn")
	// create a resolver
	r := resolver.New(&resolver.Config{Name: "authn", Servers: strings.Split(resolverUrls, ",")})
	apicl, err := apiclient.NewGrpcAPIClient(apiServer, l, rpckit.WithBalancer(balancer.New(r)))
	if err != nil {
		log.Errorf("Failed to connect to API server [%s]\n", apiServer)
		return nil, err
	}

	// fetch authentication policy
	objMeta := &api.ObjectMeta{
		Name: "AuthenticationPolicy",
	}
	policy, err := apicl.AuthV1().AuthenticationPolicy().Get(context.Background(), objMeta)
	if err != nil {
		log.Errorf("Error fetching authentication policy: Err: %v", err)
		return nil, err
	}

	// instantiate authenticators
	authenticatorOrder := policy.Spec.Authenticators.GetAuthenticatorOrder()
	authenticators := make([]authn.Authenticator, len(authenticatorOrder))
	for i, authenticatorType := range authenticatorOrder {
		switch authenticatorType {
		case auth.Authenticators_LOCAL.String():
			authenticators[i] = password.NewPasswordAuthenticator(apicl, policy.Spec.Authenticators.GetLocal())
		case auth.Authenticators_LDAP.String():
			authenticators[i] = ldap.NewLdapAuthenticator(apicl, policy.Spec.Authenticators.GetLdap())
		case auth.Authenticators_RADIUS.String():
			return nil, fmt.Errorf("[%s] Authenticator not yet implemented", authenticatorType)
		}
	}

	// instantiate token manager
	tokenManager, err := NewJWTManager(policy.Spec.GetSecret(), tokenExpiration)
	if err != nil {
		log.Errorf("Error creating TokenManager: Err: %v", err)
		return nil, err
	}

	return &AuthenticationManager{
		authenticators: authenticators,
		tokenManager:   tokenManager,
		apicl:          apicl,
	}, nil
}

// Authenticate authenticates user using authenticators in the order defined in AuthenticationPolicy. If any authenticator succeeds, it doesn't try the remaining authenticators.
func (authnmgr *AuthenticationManager) Authenticate(credential authn.Credential) (*auth.User, bool, error) {
	var errlist []error
	for _, authenticator := range authnmgr.authenticators {
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

// CreateToken creates session token. It should be called only after successful authentication.
func (authnmgr *AuthenticationManager) CreateToken(user *auth.User) (string, error) {
	return authnmgr.tokenManager.CreateToken(user)
}

// ValidateToken validates session token and checks if it has not expired.
func (authnmgr *AuthenticationManager) ValidateToken(token string) (*auth.User, bool, error) {
	return authnmgr.tokenManager.ValidateToken(token)
}
