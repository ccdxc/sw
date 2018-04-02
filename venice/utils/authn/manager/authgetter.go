package manager

import (
	"errors"
	"fmt"
	"sync"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/venice/utils/authn"
	"github.com/pensando/sw/venice/utils/authn/ldap"
	"github.com/pensando/sw/venice/utils/authn/password"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/memdb"
	"github.com/pensando/sw/venice/utils/resolver"
)

var (
	// ErrInvalidObjectType is returned when object type in cache is not known
	ErrInvalidObjectType = errors.New("invalid object type in auth cache")
)

// defaultAuthGetter is a singleton that implements AuthGetter interface
var gAuthGetter *defaultAuthGetter
var once sync.Once

type defaultAuthGetter struct {
	name            string // module name using the watcher
	apiServer       string // api server address
	resolver        resolver.Interface
	tokenExpiration time.Duration
	cache           *memdb.Memdb
	watcher         *watcher
}

func (ug *defaultAuthGetter) GetUser(name, tenant string) (*auth.User, bool) {
	objMeta := &api.ObjectMeta{Tenant: tenant, Name: name}
	val, err := ug.cache.FindObject("User", objMeta)
	if err != nil {
		log.Errorf("User [%+v] not found, Err: %v", objMeta, err)
		return nil, false
	}

	user, ok := val.(*auth.User)
	if !ok {
		log.Errorf("Invalid user type found in auth cache: %+v", user)
		ug.cache.DeleteObject(&auth.User{TypeMeta: api.TypeMeta{Kind: "User"}, ObjectMeta: *objMeta})
		return nil, ok
	}
	return user, ok
}

func (ug *defaultAuthGetter) GetAuthenticationPolicy() (*auth.AuthenticationPolicy, error) {
	// fetch authentication policy
	objMeta := &api.ObjectMeta{
		Name: "AuthenticationPolicy",
	}
	val, err := ug.cache.FindObject("AuthenticationPolicy", objMeta)
	if err != nil {
		log.Errorf("AuthenticationPolicy [%+v] not found, Err: %v", objMeta, err)
		return nil, err
	}

	policy, ok := val.(*auth.AuthenticationPolicy)
	if !ok {
		log.Errorf("Invalid policy type found in auth cache: %+v", policy)
		ug.cache.DeleteObject(&auth.AuthenticationPolicy{TypeMeta: api.TypeMeta{Kind: "AuthenticationPolicy"}, ObjectMeta: *objMeta})
		return nil, ErrInvalidObjectType
	}
	return policy, nil
}

func (ug *defaultAuthGetter) GetTokenManager() (TokenManager, error) {
	// get authentication policy
	policy, err := ug.GetAuthenticationPolicy()
	if err != nil {
		log.Errorf("Error fetching authentication policy: Err: %v", err)
		return nil, err
	}
	// instantiate token manager
	tokenManager, err := NewJWTManager(policy.Spec.GetSecret(), ug.tokenExpiration)
	if err != nil {
		log.Errorf("Error creating TokenManager: Err: %v", err)
		return nil, err
	}
	return tokenManager, nil
}

func (ug *defaultAuthGetter) GetAuthenticators() ([]authn.Authenticator, error) {
	// get authentication policy
	policy, err := ug.GetAuthenticationPolicy()
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
			authenticators[i] = password.NewPasswordAuthenticator(
				ug.name,
				ug.apiServer,
				ug.resolver,
				policy.Spec.Authenticators.GetLocal())
		case auth.Authenticators_LDAP.String():
			authenticators[i] = ldap.NewLdapAuthenticator(
				ug.name,
				ug.apiServer,
				ug.resolver,
				policy.Spec.Authenticators.GetLdap())
		case auth.Authenticators_RADIUS.String():
			return nil, fmt.Errorf("[%s] Authenticator not yet implemented", authenticatorType)
		}
	}
	return authenticators, nil

}

func (ug *defaultAuthGetter) Stop() {
	ug.watcher.stop()
}

func (ug *defaultAuthGetter) Start() {
	ug.watcher.start()
}

// GetAuthGetter returns a singleton implementation of AuthGetter
func GetAuthGetter(name, apiServer string, rslver resolver.Interface, tokenExpiration time.Duration) AuthGetter {
	once.Do(func() {
		cache := memdb.NewMemdb()
		// start the watcher on api server
		watcher := newWatcher(cache, name, apiServer, rslver)
		gAuthGetter = &defaultAuthGetter{
			name:            name,
			apiServer:       apiServer,
			resolver:        rslver,
			tokenExpiration: tokenExpiration,
			cache:           cache,
			watcher:         watcher,
		}
	})

	return gAuthGetter
}
