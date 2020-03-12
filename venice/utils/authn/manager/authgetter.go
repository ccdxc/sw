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
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/authn"
	"github.com/pensando/sw/venice/utils/authn/ldap"
	"github.com/pensando/sw/venice/utils/authn/password"
	"github.com/pensando/sw/venice/utils/authn/radius"
	"github.com/pensando/sw/venice/utils/balancer"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/memdb"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
	"github.com/pensando/sw/venice/utils/watcher"
)

const (
	defaultTokenExpiry = "144h" // default to 6 days
	policyKey          = "AuthenticationPolicy"
)

var (
	// ErrInvalidObjectType is returned when object type in cache is not known
	ErrInvalidObjectType = errors.New("invalid object type in auth cache")
)

// defaultAuthGetter is a singleton that implements AuthGetter interface
var gAuthGetter *defaultAuthGetter
var once sync.Once

type defaultAuthGetter struct {
	sync.RWMutex
	name      string // module name using the watcher
	apiServer string // api server address
	resolver  resolver.Interface
	cache     *memdb.Memdb
	watcher   *watcher.Watcher
	logger    log.Logger
	stopped   bool
}

func (ug *defaultAuthGetter) GetUser(name, tenant string) (*auth.User, bool) {
	objMeta := &api.ObjectMeta{Tenant: tenant, Name: name, Namespace: globals.DefaultNamespace}
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
	retUser := &auth.User{}
	user.Clone(retUser)
	return retUser, ok
}

func (ug *defaultAuthGetter) GetAuthenticationPolicy() (*auth.AuthenticationPolicy, error) {
	// fetch authentication policy
	objMeta := &api.ObjectMeta{
		Name: policyKey,
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
	retPolicy := &auth.AuthenticationPolicy{}
	policy.Clone(retPolicy)
	return retPolicy, nil
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
	var authenticators []authn.Authenticator
	for _, authenticatorType := range authenticatorOrder {
		switch authenticatorType {
		case auth.Authenticators_LOCAL.String():
			authenticators = append(authenticators, password.NewPasswordAuthenticator(
				ug.name,
				ug.apiServer,
				ug.resolver,
				policy.Spec.Authenticators.GetLocal(), ug.logger))
		case auth.Authenticators_LDAP.String():
			authenticators = append(authenticators, ldap.NewLdapAuthenticator(policy.Spec.Authenticators.Ldap))
		case auth.Authenticators_RADIUS.String():
			authenticators = append(authenticators, radius.NewRadiusAuthenticator(policy.Spec.Authenticators.Radius))
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
		ug.logger.Errorf("Cluster [%+v] not found, Err: %v", objMeta, err)
		return false, err
	}

	clusterObj, ok := val.(*cluster.Cluster)
	if !ok {
		ug.logger.Errorf("Invalid cluster type found in auth cache: %#v", clusterObj)
		ug.cache.DeleteObject(&cluster.Cluster{TypeMeta: api.TypeMeta{Kind: "Cluster"}, ObjectMeta: *objMeta})
		return false, ErrInvalidObjectType
	}
	return clusterObj.Status.AuthBootstrapped, nil
}

func (ug *defaultAuthGetter) Stop() {
	defer ug.Unlock()
	ug.Lock()
	ug.watcher.Stop()
	ug.stopped = true
}

func (ug *defaultAuthGetter) start(name, apiServer string, rslver resolver.Interface) {
	defer ug.Unlock()
	ug.Lock()
	if ug.stopped {
		ug.name = name
		ug.apiServer = apiServer
		ug.resolver = rslver
		ug.watcher.Start(ug.name, ug.apiServer, ug.resolver)
		ug.stopped = false
	}
}

func (ug *defaultAuthGetter) Start() {
	ug.start(ug.name, ug.apiServer, ug.resolver)
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
		policy, err := apicl.AuthV1().AuthenticationPolicy().Get(context.Background(), objMeta)
		if err != nil {
			ug.logger.Errorf("Error getting authentication policy [%s] from API server: %v", objMeta.Name, err)
			return nil, err
		}
		policy.Name = policyKey
		val = policy
	}
	err = ug.cache.AddObject(val)
	if err != nil {
		ug.logger.Errorf("Error adding object [%s|%s] of kind [%v] to AuthGetter cache: %v", objMeta.Tenant, objMeta.Tenant, kind, err)
		return nil, err
	}
	ug.logger.Infof("Updated kind [%v] [%#v] in AuthGetter cache", kind, val.GetObjectMeta())
	return val, err
}

func (ug *defaultAuthGetter) processUserEvent(evt *kvstore.WatchEvent, user *auth.User) {
	// update cache
	switch evt.Type {
	case kvstore.Created, kvstore.Updated:
		ug.cache.AddObject(user)
		ug.logger.Infof("Updated User [%#v] in AuthGetter cache", user.ObjectMeta)
	case kvstore.Deleted:
		ug.cache.DeleteObject(user)
		ug.logger.Infof("Deleted User [%#v] in AuthGetter cache", user.ObjectMeta)
	}
}

func (ug *defaultAuthGetter) processPolicyEvent(evt *kvstore.WatchEvent, policy *auth.AuthenticationPolicy) {
	// update cache
	policy.Name = "AuthenticationPolicy" // it is a singleton
	switch evt.Type {
	case kvstore.Created, kvstore.Updated:
		ug.cache.AddObject(policy)
		ug.logger.Infof("Updated AuthenticationPolicy [%#v] in AuthGetter cache", policy.ObjectMeta)
	case kvstore.Deleted:
		ug.cache.DeleteObject(policy)
		ug.logger.Infof("Deleted AuthenticationPolicy [%#v] in AuthGetter cache", policy.ObjectMeta)
	}
}

func (ug *defaultAuthGetter) processClusterEvent(evt *kvstore.WatchEvent, clusterObj *cluster.Cluster) {
	clusterObj.Name = "Cluster" // it is a singleton
	switch evt.Type {
	case kvstore.Created, kvstore.Updated:
		ug.cache.AddObject(clusterObj)
		ug.logger.Infof("Updated Cluster [%#v] in AuthGetter cache", clusterObj.ObjectMeta)
	case kvstore.Deleted:
		ug.cache.DeleteObject(clusterObj)
		ug.logger.Infof("Deleted Cluster [%#v] in AuthGetter cache", clusterObj.ObjectMeta)
	}
}

func (ug *defaultAuthGetter) processEventCb(evt *kvstore.WatchEvent) error {
	switch tp := evt.Object.(type) {
	case *auth.User:
		ug.processUserEvent(evt, tp)
	case *auth.AuthenticationPolicy:
		ug.processPolicyEvent(evt, tp)
	case *cluster.Cluster:
		ug.processClusterEvent(evt, tp)
	default:
		ug.logger.Errorf("watcher found object of invalid type: %+v", tp)
	}
	return nil
}

func (ug *defaultAuthGetter) resetCacheCb() {
	users := ug.cache.ListObjects("User", nil)
	for _, user := range users {
		ug.cache.DeleteObject(user)
	}
}

// GetAuthGetter returns a singleton implementation of AuthGetter
func GetAuthGetter(name, apiServer string, rslver resolver.Interface, logger log.Logger) AuthGetter {
	module := name + authn.ModuleSuffix
	if gAuthGetter != nil {
		gAuthGetter.start(module, apiServer, rslver)
	}
	once.Do(func() {
		if logger == nil {
			logger = log.GetNewLogger(log.GetDefaultConfig(module))
		}
		cache := memdb.NewMemdb()
		gAuthGetter = &defaultAuthGetter{
			name:      module,
			apiServer: apiServer,
			resolver:  rslver,
			cache:     cache,
			logger:    logger,
			stopped:   false,
		}
		// start watcher
		// Use a custom TLS client identity so that secret field TLS key is not zeroized by ApiServer on watch
		gAuthGetter.watcher = watcher.NewWatcher(module, apiServer, rslver, logger, gAuthGetter.resetCacheCb, gAuthGetter.processEventCb,
			[]rpckit.Option{rpckit.WithTLSClientIdentity("auth-watcher")},
			&watcher.KindOptions{
				Kind:    string(auth.KindUser),
				Options: &api.ListWatchOptions{},
			},
			&watcher.KindOptions{
				Kind:    string(auth.KindAuthenticationPolicy),
				Options: &api.ListWatchOptions{},
			},
			&watcher.KindOptions{
				Kind:    string(cluster.KindCluster),
				Options: &api.ListWatchOptions{},
			})
	})

	return gAuthGetter
}
