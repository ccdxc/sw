package auth

import (
	"errors"
	"fmt"
	"reflect"
	"testing"

	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/login"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/authn/manager"
	"github.com/pensando/sw/venice/utils/authz/rbac"
	"github.com/pensando/sw/venice/utils/bootstrapper"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestIsBootstrapped(t *testing.T) {
	tests := []struct {
		name       string
		tenant     string
		authGetter manager.AuthGetter
		permGetter rbac.PermissionGetter
		ok         bool
		err        error
	}{
		{
			name:       "non global tenant",
			tenant:     "testTenant",
			authGetter: manager.NewMockAuthGetter(nil, false),
			permGetter: rbac.NewMockPermissionGetter(nil, nil, nil, nil),
			ok:         true,
			err:        nil,
		},
		{
			name:       "no role bindings",
			tenant:     globals.DefaultTenant,
			authGetter: manager.NewMockAuthGetter(nil, false),
			permGetter: rbac.NewMockPermissionGetter(nil, nil, []*auth.Role{login.NewClusterRole(globals.AdminRole)}, nil),
			ok:         false,
			err:        ErrNoAdminRoleBindings,
		},
		{
			name:       "no admin role binding",
			tenant:     globals.DefaultTenant,
			authGetter: manager.NewMockAuthGetter(nil, false),
			permGetter: rbac.NewMockPermissionGetter(nil, nil, []*auth.Role{login.NewClusterRole(globals.AdminRole)},
				[]*auth.RoleBinding{login.NewClusterRoleBinding("AdminRoleBinding", "testRole", "testUser", "")}),
			ok:  false,
			err: ErrNoAdminRoleBindings,
		},
		{
			name:       "already bootstrapped",
			tenant:     globals.DefaultTenant,
			authGetter: manager.NewMockAuthGetter(nil, false),
			permGetter: rbac.NewMockPermissionGetter(nil, nil, []*auth.Role{login.NewClusterRole(globals.AdminRole)},
				[]*auth.RoleBinding{login.NewClusterRoleBinding("AdminRoleBinding", globals.AdminRole, "testUser", "")}),
			ok:  true,
			err: nil,
		},
		{
			name:       "error in fetching auth policy",
			tenant:     globals.DefaultTenant,
			authGetter: manager.NewMockAuthGetter(nil, true),
			permGetter: rbac.NewMockPermissionGetter(nil, nil, []*auth.Role{login.NewClusterRole(globals.AdminRole)},
				[]*auth.RoleBinding{login.NewClusterRoleBinding("AdminRoleBinding", globals.AdminRole, "testUser", "")}),
			ok:  false,
			err: errors.New("authentication policy not found"),
		},
		{
			name:       "no user or group assigned to admin role binding",
			tenant:     globals.DefaultTenant,
			authGetter: manager.NewMockAuthGetter(nil, false),
			permGetter: rbac.NewMockPermissionGetter(nil, nil, []*auth.Role{login.NewClusterRole(globals.AdminRole)},
				[]*auth.RoleBinding{login.NewClusterRoleBinding("AdminRoleBinding", globals.AdminRole, "", "")}),
			ok:  false,
			err: ErrNoAdminRoleBindings,
		},
	}
	logConfig := log.GetDefaultConfig("TestAuthFeature")
	l := log.GetNewLogger(logConfig)
	for _, test := range tests {
		f := &feature{
			logger:     l,
			id:         bootstrapper.Auth,
			authGetter: test.authGetter,
			permGetter: test.permGetter,
		}
		ok, err := f.IsBootstrapped(test.tenant)
		Assert(t, ok == test.ok, fmt.Sprintf("[%s] test failed", test.name))
		Assert(t, reflect.DeepEqual(err, test.err), fmt.Sprintf("[%s] test failed, expected error [%v], got [%v]", test.name, test.err, err))
	}
}
