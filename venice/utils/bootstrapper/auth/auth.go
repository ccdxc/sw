package auth

import (
	"github.com/pensando/sw/venice/apigw"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/authn/manager"
	"github.com/pensando/sw/venice/utils/authz/rbac"
	"github.com/pensando/sw/venice/utils/bootstrapper"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
)

type feature struct {
	logger     log.Logger
	id         bootstrapper.FeatureID
	name       string
	apiServer  string
	rslvr      resolver.Interface
	authGetter manager.AuthGetter
	permGetter rbac.PermissionGetter
}

func (f *feature) IsBootstrapped(tenant string) bool {
	// auth feature gets in bootstrap mode only for default tenant
	if tenant != globals.DefaultTenant {
		return true
	}
	// check if authentication policy exists
	if _, err := f.authGetter.GetAuthenticationPolicy(); err != nil {
		f.logger.Infof("AuthenticationPolicy not found, auth is not bootstrapped, Err: %v", err)
		return false
	}
	// check if admin role binding exists for "default" tenant
	rbs := f.permGetter.GetRoleBindings(tenant)
	if len(rbs) == 0 {
		f.logger.Infof("no role bindings found in default tenant, auth is not bootstrapped")
		return false
	}
	for _, rb := range rbs {
		if rb.Spec.Role == globals.AdminRole {
			return true
		}
	}
	f.logger.Infof("no role bindings with [%s] role found in default tenant, auth is not bootstrapped", globals.AdminRole)
	return false
}

func (f *feature) IsFlagSet(tenant string) (bool, error) {
	return f.authGetter.IsAuthBootstrapped()
}

func (f *feature) ID() bootstrapper.FeatureID {
	return f.id
}

func registerAuthFeature(name, apiServer string, rslvr resolver.Interface, l log.Logger) (bootstrapper.Feature, error) {
	f := &feature{
		logger:     l,
		name:       name,
		apiServer:  apiServer,
		rslvr:      rslvr,
		id:         bootstrapper.Auth,
		authGetter: manager.GetAuthGetter(name, apiServer, rslvr, apigw.TokenExpInDays*24*60*60),
		permGetter: rbac.GetPermissionGetter(name, apiServer, rslvr),
	}
	return f, nil
}

func init() {
	bs := bootstrapper.GetBootstrapper()
	bs.RegisterFeatureCb(bootstrapper.Auth, registerAuthFeature)
}
