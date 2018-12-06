package manager

import (
	"context"
	"errors"
	"sync"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/venice/utils/authn"
	"github.com/pensando/sw/venice/utils/authn/ldap"
	"github.com/pensando/sw/venice/utils/authn/password"
	"github.com/pensando/sw/venice/utils/authn/radius"
	"github.com/pensando/sw/venice/utils/balancer"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/memdb"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
)

const (
	defaultTokenExpiry = "144h" // default to 6 days
)

var (
	// ErrInvalidObjectType is returned when object type in cache is not known
	ErrInvalidObjectType = errors.New("invalid object type in auth cache")
)

// defaultAuthGetter is a singleton that implements AuthGetter interface
var gAuthGetter *defaultAuthGetter
var once sync.Once

type defaultAuthGetter struct {
	name      string // module name using the watcher
	apiServer string // api server address
	resolver  resolver.Interface
	cache     *memdb.Memdb
	watcher   *watcher
	logger    log.Logger
	stopped   bool
}

func (ug *defaultAuthGetter) GetUser(name, tenant string) (*auth.User, bool) {
	objMeta := &api.ObjectMeta{Tenant: tenant, Name: name}
	val, err := ug.cache.FindObject("User", objMeta)
	if err != nil {
		ug.logger.Errorf("User [%+v] not found in AuthGetter cache, Err: %v", objMeta, err)
		val, err = ug.addObj(auth.KindUser, objMeta)
		if err != nil {
			return nil, false
		}
	}

	user, ok := val.(*auth.User)
	if !ok {
		ug.logger.Errorf("Invalid user type found in AuthGetter cache: %+v", user)
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
		ug.logger.Errorf("AuthenticationPolicy [%+v] not found in AuthGetter cache, Err: %v", objMeta, err)
		val, err = ug.addObj(auth.KindAuthenticationPolicy, objMeta)
		if err != nil {
			return nil, err
		}
	}

	policy, ok := val.(*auth.AuthenticationPolicy)
	if !ok {
		ug.logger.Errorf("Invalid policy type found in AuthGetter cache: %+v", policy)
		ug.cache.DeleteObject(&auth.AuthenticationPolicy{TypeMeta: api.TypeMeta{Kind: "AuthenticationPolicy"}, ObjectMeta: *objMeta})
		return nil, ErrInvalidObjectType
	}
	return policy, nil
}

func (ug *defaultAuthGetter) GetTokenManager() (TokenManager, error) {
	// get authentication policy
	policy, err := ug.GetAuthenticationPolicy()
	if err != nil {
		ug.logger.Errorf("Error fetching authentication policy: %v", err)
		return nil, err
	}
	if policy.Spec.TokenExpiry == "" {
		policy.Spec.TokenExpiry = defaultTokenExpiry // default to 6 days
	}
	exp, err := time.ParseDuration(policy.Spec.TokenExpiry)
	if err != nil {
		ug.logger.Errorf("invalid token expiration value: %v", err)
		return nil, err
	}
	// instantiate token manager
	tokenManager, err := NewJWTManager(policy.Spec.GetSecret(), exp)
	if err != nil {
		ug.logger.Errorf("Error creating TokenManager: %v", err)
		return nil, err
	}
	return tokenManager, nil
}

func (ug *defaultAuthGetter) GetAuthenticators() ([]authn.Authenticator, error) {
	// get authentication policy
	policy, err := ug.GetAuthenticationPolicy()
	if err != nil {
		ug.logger.Errorf("Error fetching authentication policy: %v", err)
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
			authenticators[i] = ldap.NewLdapAuthenticator(policy.Spec.Authenticators.GetLdap())
		case auth.Authenticators_RADIUS.String():
			authenticators[i] = radius.NewRadiusAuthenticator(policy.Spec.Authenticators.GetRadius())
		}
	}
	return authenticators, nil

}

func (ug *defaultAuthGetter) IsAuthBootstrapped() (bool, error) {
	// fetch cluster
	objMeta := &api.ObjectMeta{
		Name: "Cluster",
	}
	val, err := ug.cache.FindObject("Cluster", objMeta)
	if err != nil {
		log.Errorf("Cluster [%+v] not found, Err: %v", objMeta, err)
		return false, err
	}

	clusterObj, ok := val.(*cluster.Cluster)
	if !ok {
		log.Errorf("Invalid cluster type found in auth cache: %#v", clusterObj)
		ug.cache.DeleteObject(&cluster.Cluster{TypeMeta: api.TypeMeta{Kind: "Cluster"}, ObjectMeta: *objMeta})
		return false, ErrInvalidObjectType
	}
	return clusterObj.Status.AuthBootstrapped, nil
}

func (ug *defaultAuthGetter) Stop() {
	ug.watcher.stop()
	ug.stopped = true
}

func (ug *defaultAuthGetter) Start() {
	ug.stopped = false
	ug.watcher.start()
}

func (ug *defaultAuthGetter) addObj(kind auth.ObjKind, objMeta *api.ObjectMeta) (memdb.Object, error) {
	if ug.stopped {
		ug.logger.Errorf("Ignoring add of object %v as AuthGetter is in stopped state", *objMeta)
	}
	b := balancer.New(ug.resolver)
	apicl, err := apiclient.NewGrpcAPIClient(ug.name, ug.apiServer, ug.logger, rpckit.WithBalancer(b))
	if err != nil {
		ug.logger.Errorf("Error connecting to gRPC server [%s]: %v", ug.apiServer, err)
		return nil, err
	}
	defer apicl.Close()
	var val memdb.Object
	switch kind {
	case auth.KindUser:
		val, err = apicl.AuthV1().User().Get(context.Background(), objMeta)
		if err != nil {
			ug.logger.Errorf("Error getting user [%s|%s] from API server: %v", objMeta.Tenant, objMeta.Name, err)
			return nil, err
		}
	case auth.KindAuthenticationPolicy:
		val, err = apicl.AuthV1().AuthenticationPolicy().Get(context.Background(), objMeta)
		if err != nil {
			ug.logger.Errorf("Error getting authentication policy [%s] from API server: %v", objMeta.Name, err)
			return nil, err
		}
	}
	err = ug.cache.AddObject(val)
	if err != nil {
		ug.logger.Errorf("Error adding object [%s|%s] of kind [%v] to AuthGetter cache: %v", objMeta.Tenant, objMeta.Tenant, kind, err)
		return nil, err
	}
	ug.logger.Infof("Updated kind [%v] [%#v] in AuthGetter cache", kind, val.GetObjectMeta())
	return val, err
}

// GetAuthGetter returns a singleton implementation of AuthGetter
func GetAuthGetter(name, apiServer string, rslver resolver.Interface) AuthGetter {
	if gAuthGetter != nil && gAuthGetter.stopped {
		gAuthGetter.resolver = rslver
		gAuthGetter.watcher.resolver = rslver
		gAuthGetter.apiServer = apiServer
		gAuthGetter.Start()
	}
	once.Do(func() {
		module := name + authn.ModuleSuffix
		// create logger
		config := log.GetDefaultConfig(module)
		l := log.GetNewLogger(config)

		cache := memdb.NewMemdb()
		// start the watcher on api server
		watcher := newWatcher(cache, module, apiServer, rslver)
		gAuthGetter = &defaultAuthGetter{
			name:      module,
			apiServer: apiServer,
			resolver:  rslver,
			cache:     cache,
			watcher:   watcher,
			logger:    l,
			stopped:   false,
		}
	})

	return gAuthGetter
}
