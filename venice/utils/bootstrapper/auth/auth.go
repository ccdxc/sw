package auth

import (
	"fmt"

	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/authn/manager"
	"github.com/pensando/sw/venice/utils/authz/rbac"
	"github.com/pensando/sw/venice/utils/bootstrapper"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
)

var (
	// ErrNoAdminRoleBindings is returned when there is no AdminRoleBinding with a user or group assigned
	ErrNoAdminRoleBindings = fmt.Errorf("no admin role binding with assigned user or group in %s tenant", globals.DefaultTenant)
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

func (f *feature) IsBootstrapped(tenant string) (bool, error) {
	// auth feature gets in bootstrap mode only for default tenant
	if tenant != globals.DefaultTenant {
		return true, nil
	}
	// check if authentication policy exists
	if _, err := f.authGetter.GetAuthenticationPolicy(); err != nil {
		f.logger.InfoLog("method", "IsBootstrapped", "msg", "AuthenticationPolicy not found, auth is not bootstrapped", "error", err)
		return false, err
	}
	// check if admin role binding exists for "default" tenant
	rbs := f.permGetter.GetRoleBindings(tenant)
	for _, rb := range rbs {
		if (rb.Spec.Role == globals.AdminRole) && (len(rb.Spec.Users) > 0 || len(rb.Spec.UserGroups) > 0) {
			return true, nil
		}
	}
	f.logger.InfoLog("method", "IsBootstrapped", "msg", fmt.Sprintf("no role bindings [%#v] with [%s] role found in default tenant, auth is not bootstrapped", rbs, globals.AdminRole))
	return false, ErrNoAdminRoleBindings
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
		authGetter: manager.GetAuthGetter(name, apiServer, rslvr, l),
		permGetter: rbac.GetPermissionGetter(name, apiServer, rslvr),
	}
	return f, nil
}

func init() {
	bs := bootstrapper.GetBootstrapper()
	bs.RegisterFeatureCb(bootstrapper.Auth, registerAuthFeature)
}
