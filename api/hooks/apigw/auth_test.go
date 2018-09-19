package impl

import (
	"context"
	"errors"
	"fmt"
	"reflect"
	"sort"
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/login"
	"github.com/pensando/sw/venice/apigw/pkg"
	"github.com/pensando/sw/venice/apigw/pkg/mocks"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/utils/authz"
	"github.com/pensando/sw/venice/utils/log"

	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/authn/manager"
	"github.com/pensando/sw/venice/utils/authz/rbac"
	"github.com/pensando/sw/venice/utils/bootstrapper"
	. "github.com/pensando/sw/venice/utils/testutils"
)

var (
	testNetworkAdminRole = login.NewRole("NetworkAdmin", "testTenant", login.NewPermission(
		"testTenant",
		"Network",
		auth.Permission_Network.String(),
		authz.ResourceNamespaceAll,
		"network1,network2",
		fmt.Sprintf("%s,%s,%s", auth.Permission_CREATE.String(), auth.Permission_UPDATE.String(), auth.Permission_DELETE.String())),
		login.NewPermission(
			"testTenant",
			"Network",
			auth.Permission_LbPolicy.String(),
			authz.ResourceNamespaceAll,
			"",
			fmt.Sprintf("%s,%s,%s", auth.Permission_CREATE.String(), auth.Permission_UPDATE.String(), auth.Permission_DELETE.String())))

	testNetworkAdminRoleBinding = login.NewRoleBinding("NetworkAdminRoleBinding", "testTenant", "NetworkAdmin", "testuser", "")
)

func sortOperations(operations []authz.Operation) {
	sort.Slice(operations, func(i, j int) bool {
		return fmt.Sprintf("%s%s", operations[i].GetResource().GetName(), operations[i].GetAction()) < fmt.Sprintf("%s%s", operations[j].GetResource().GetName(), operations[j].GetAction())
	})
}

func areOperationsEqual(expected []authz.Operation, returned []authz.Operation) bool {
	sortOperations(expected)
	sortOperations(returned)
	return reflect.DeepEqual(expected, returned)
}

func TestPrivilegeEscalationCheck(t *testing.T) {
	tests := []struct {
		name               string
		in                 interface{}
		expectedOperations []authz.Operation
		out                interface{}
		err                bool
	}{
		{
			name: "when creating role",
			in:   testNetworkAdminRole,
			expectedOperations: []authz.Operation{
				authz.NewOperation(authz.NewResource("testTenant",
					"Network", auth.Permission_Network.String(),
					authz.ResourceNamespaceAll, "network1"),
					auth.Permission_CREATE.String()),
				authz.NewOperation(authz.NewResource("testTenant",
					"Network", auth.Permission_Network.String(),
					authz.ResourceNamespaceAll, "network2"),
					auth.Permission_CREATE.String()),
				authz.NewOperation(authz.NewResource("testTenant",
					"Network", auth.Permission_Network.String(),
					authz.ResourceNamespaceAll, "network1"),
					auth.Permission_UPDATE.String()),
				authz.NewOperation(authz.NewResource("testTenant",
					"Network", auth.Permission_Network.String(),
					authz.ResourceNamespaceAll, "network2"),
					auth.Permission_UPDATE.String()),
				authz.NewOperation(authz.NewResource("testTenant",
					"Network", auth.Permission_Network.String(),
					authz.ResourceNamespaceAll, "network1"),
					auth.Permission_DELETE.String()),
				authz.NewOperation(authz.NewResource("testTenant",
					"Network", auth.Permission_Network.String(),
					authz.ResourceNamespaceAll, "network2"),
					auth.Permission_DELETE.String()),
				authz.NewOperation(authz.NewResource("testTenant",
					"Network", auth.Permission_LbPolicy.String(),
					authz.ResourceNamespaceAll, ""),
					auth.Permission_CREATE.String()),
				authz.NewOperation(authz.NewResource("testTenant",
					"Network", auth.Permission_LbPolicy.String(),
					authz.ResourceNamespaceAll, ""),
					auth.Permission_UPDATE.String()),
				authz.NewOperation(authz.NewResource("testTenant",
					"Network", auth.Permission_LbPolicy.String(),
					authz.ResourceNamespaceAll, ""),
					auth.Permission_DELETE.String()),
			},
			out: testNetworkAdminRole,
			err: false,
		},
		{
			name: "when creating role binding",
			in:   testNetworkAdminRoleBinding,
			expectedOperations: []authz.Operation{
				authz.NewOperation(authz.NewResource("testTenant",
					"Network", auth.Permission_Network.String(),
					authz.ResourceNamespaceAll, "network1"),
					auth.Permission_CREATE.String()),
				authz.NewOperation(authz.NewResource("testTenant",
					"Network", auth.Permission_Network.String(),
					authz.ResourceNamespaceAll, "network2"),
					auth.Permission_CREATE.String()),
				authz.NewOperation(authz.NewResource("testTenant",
					"Network", auth.Permission_Network.String(),
					authz.ResourceNamespaceAll, "network1"),
					auth.Permission_UPDATE.String()),
				authz.NewOperation(authz.NewResource("testTenant",
					"Network", auth.Permission_Network.String(),
					authz.ResourceNamespaceAll, "network2"),
					auth.Permission_UPDATE.String()),
				authz.NewOperation(authz.NewResource("testTenant",
					"Network", auth.Permission_Network.String(),
					authz.ResourceNamespaceAll, "network1"),
					auth.Permission_DELETE.String()),
				authz.NewOperation(authz.NewResource("testTenant",
					"Network", auth.Permission_Network.String(),
					authz.ResourceNamespaceAll, "network2"),
					auth.Permission_DELETE.String()),
				authz.NewOperation(authz.NewResource("testTenant",
					"Network", auth.Permission_LbPolicy.String(),
					authz.ResourceNamespaceAll, ""),
					auth.Permission_CREATE.String()),
				authz.NewOperation(authz.NewResource("testTenant",
					"Network", auth.Permission_LbPolicy.String(),
					authz.ResourceNamespaceAll, ""),
					auth.Permission_UPDATE.String()),
				authz.NewOperation(authz.NewResource("testTenant",
					"Network", auth.Permission_LbPolicy.String(),
					authz.ResourceNamespaceAll, ""),
					auth.Permission_DELETE.String()),
			},
			out: testNetworkAdminRoleBinding,
			err: false,
		},
		{
			name:               "incorrect object type",
			in:                 struct{ name string }{"testing"},
			expectedOperations: nil,
			out:                struct{ name string }{"testing"},
			err:                true,
		},
		{
			name:               "incorrect role name in role binding",
			in:                 login.NewRoleBinding("IncorrectRoleBinding", "testTenant", "incorrectRoleName", "testUser", ""),
			expectedOperations: nil,
			out:                login.NewRoleBinding("IncorrectRoleBinding", "testTenant", "incorrectRoleName", "testUser", ""),
			err:                true,
		},
	}
	r := &authHooks{}
	logConfig := log.GetDefaultConfig("TestAPIGwAuthHooks")
	r.logger = log.GetNewLogger(logConfig)
	r.permissionGetter = rbac.NewMockPermissionGetter([]*auth.Role{testNetworkAdminRole}, []*auth.RoleBinding{testNetworkAdminRoleBinding}, nil, nil)
	for _, test := range tests {
		nctx, out, err := r.privilegeEscalationCheck(context.TODO(), test.in)
		Assert(t, test.err == (err != nil), fmt.Sprintf("got error [%v], [%s] test failed", err, test.name))
		operations, _ := apigwpkg.OperationsFromContext(nctx)
		Assert(t, areOperationsEqual(test.expectedOperations, operations),
			fmt.Sprintf("unexpected operations, [%s] test failed", test.name))
		Assert(t, reflect.DeepEqual(test.out, out),
			fmt.Sprintf("expected returned object [%v], got [%v], [%s] test failed", test.out, out, test.name))
	}
}

func TestAuthzRegistration(t *testing.T) {
	logConfig := log.GetDefaultConfig("TestAPIGwAuthHooks")
	l := log.GetNewLogger(logConfig)
	svc := mocks.NewFakeAPIGwService(l, false)
	r := &authHooks{}
	r.logger = l
	err := r.registerPrivilegeEscalationHook(svc)
	AssertOk(t, err, "privilege escalation hook registration failed")
	// test authz hooks
	ids := []serviceID{
		{"Role", apiserver.CreateOper},
		{"Role", apiserver.UpdateOper},
		{"RoleBinding", apiserver.CreateOper},
		{"RoleBinding", apiserver.UpdateOper},
	}
	for _, id := range ids {
		prof, err := svc.GetCrudServiceProfile(id.kind, id.action)
		AssertOk(t, err, fmt.Sprintf("error getting service profile for [%s] [%s]", id.kind, id.action))
		Assert(t, len(prof.PreAuthZHooks()) == 1, fmt.Sprintf("unexpected number of pre authz hooks [%d] for [%s] [%s] profile", len(prof.PreAuthZHooks()), id.kind, id.action))
	}
	// test error
	svc = mocks.NewFakeAPIGwService(l, true)
	err = r.registerPrivilegeEscalationHook(svc)
	Assert(t, err != nil, "expected error in privilege escalation hook registration")
}

func TestAuthnRegistration(t *testing.T) {
	logConfig := log.GetDefaultConfig("TestAPIGwAuthHooks")
	l := log.GetNewLogger(logConfig)
	svc := mocks.NewFakeAPIGwService(l, false)
	r := &authHooks{}
	r.logger = l
	err := r.registerAuthBootstrapHook(svc)
	AssertOk(t, err, "authBootstrap hook registration failed")

	// test authn hooks
	ids := []serviceID{
		{"AuthenticationPolicy", apiserver.CreateOper},
		{"User", apiserver.CreateOper},
		{"RoleBinding", apiserver.CreateOper},
	}
	for _, id := range ids {
		prof, err := svc.GetCrudServiceProfile(id.kind, id.action)
		AssertOk(t, err, "error getting service profile for [%s] [%s]", id.kind, id.action)
		Assert(t, len(prof.PreAuthNHooks()) == 1, fmt.Sprintf("unexpected number of pre authn hooks [%d] for [%s] [%s] profile", len(prof.PreAuthNHooks()), id.kind, id.action))
	}
	// test error
	svc = mocks.NewFakeAPIGwService(l, true)
	err = r.registerAuthBootstrapHook(svc)
	Assert(t, err != nil, "expected error in authBootstrap hook registration")
}

func TestRemovePassword(t *testing.T) {
	tests := []struct {
		name string
		in   interface{}
		err  bool
	}{
		{
			name: "User object",
			in: &auth.User{
				TypeMeta: api.TypeMeta{Kind: "User"},
				ObjectMeta: api.ObjectMeta{
					Tenant: "testTenant",
					Name:   "testuser",
				},
				Spec: auth.UserSpec{
					Fullname: "Test User",
					Password: "password",
					Email:    "testuser@pensandio.io",
					Type:     auth.UserSpec_LOCAL.String(),
				},
			},
			err: false,
		},
		{
			name: "User list",
			in: &auth.UserList{
				Items: []*auth.User{
					{
						TypeMeta: api.TypeMeta{Kind: "User"},
						ObjectMeta: api.ObjectMeta{
							Tenant: "testTenant",
							Name:   "testuser",
						},
						Spec: auth.UserSpec{
							Fullname: "Test User",
							Password: "password",
							Email:    "testuser@pensandio.io",
							Type:     auth.UserSpec_LOCAL.String(),
						},
					},
				},
			},
			err: false,
		},
		{
			name: "invalid object",
			in:   &struct{ name string }{name: "invalid object type"},
			err:  true,
		},
	}
	logConfig := log.GetDefaultConfig("TestAPIGwAuthHooks")
	l := log.GetNewLogger(logConfig)
	r := &authHooks{}
	r.logger = l
	for _, test := range tests {
		_, out, err := r.removePassword(context.TODO(), test.in)
		Assert(t, test.err == (err != nil), fmt.Sprintf("got error [%v], [%s] test failed", err, test.name))
		switch obj := out.(type) {
		case *auth.User:
			Assert(t, obj.Spec.Password == "", "non empty password")
		case *auth.UserList:
			for _, user := range obj.GetItems() {
				Assert(t, user.Spec.Password == "", "non empty password in user list")
			}
		}

	}
}

func TestAddRoles(t *testing.T) {
	tests := []struct {
		name string
		in   interface{}
		err  bool
	}{
		{
			name: "User object",
			in: &auth.User{
				TypeMeta: api.TypeMeta{Kind: "User"},
				ObjectMeta: api.ObjectMeta{
					Tenant: "testTenant",
					Name:   "testuser",
				},
				Spec: auth.UserSpec{
					Fullname: "Test User",
					Password: "password",
					Email:    "testuser@pensandio.io",
					Type:     auth.UserSpec_LOCAL.String(),
				},
			},
			err: false,
		},
		{
			name: "User list",
			in: &auth.UserList{
				Items: []*auth.User{
					{
						TypeMeta: api.TypeMeta{Kind: "User"},
						ObjectMeta: api.ObjectMeta{
							Tenant: "testTenant",
							Name:   "testuser",
						},
						Spec: auth.UserSpec{
							Fullname: "Test User",
							Password: "password",
							Email:    "testuser@pensandio.io",
							Type:     auth.UserSpec_LOCAL.String(),
						},
					},
				},
			},
			err: false,
		},
		{
			name: "invalid object",
			in:   &struct{ name string }{name: "invalid object type"},
			err:  true,
		},
	}
	logConfig := log.GetDefaultConfig("TestAPIGwAuthHooks")
	l := log.GetNewLogger(logConfig)
	r := &authHooks{}
	r.logger = l
	r.permissionGetter = rbac.NewMockPermissionGetter([]*auth.Role{testNetworkAdminRole}, []*auth.RoleBinding{testNetworkAdminRoleBinding}, nil, nil)
	for _, test := range tests {
		_, out, err := r.addRoles(context.TODO(), test.in)
		Assert(t, test.err == (err != nil), fmt.Sprintf("got error [%v], [%s] test failed", err, test.name))
		switch obj := out.(type) {
		case *auth.User:
			Assert(t, len(obj.Status.Roles) == 1 && obj.Status.Roles[0] == "NetworkAdmin", "user should have network admin role")
		case *auth.UserList:
			for _, user := range obj.GetItems() {
				Assert(t, len(user.Status.Roles) == 1 && user.Status.Roles[0] == "NetworkAdmin", "user should have network admin role")
			}
		}

	}
}

func TestRemoveSecret(t *testing.T) {
	tests := []struct {
		name string
		in   interface{}
		err  bool
	}{
		{
			name: "auth policy",
			in: &auth.AuthenticationPolicy{
				TypeMeta: api.TypeMeta{Kind: "AuthenticationPolicy"},
				ObjectMeta: api.ObjectMeta{
					Name: "AuthenticationPolicy",
				},
				Spec: auth.AuthenticationPolicySpec{
					Authenticators: auth.Authenticators{
						Ldap: &auth.Ldap{
							Enabled: false,
						},
						Local: &auth.Local{
							Enabled: true,
						},
						AuthenticatorOrder: []string{auth.Authenticators_LDAP.String(), auth.Authenticators_LOCAL.String()},
					},
					Secret: []byte("secret"),
				},
			},
			err: false,
		},
		{
			name: "invalid object",
			in:   &struct{ name string }{name: "invalid object type"},
			err:  true,
		},
	}
	logConfig := log.GetDefaultConfig("TestAPIGwAuthHooks")
	l := log.GetNewLogger(logConfig)
	r := &authHooks{}
	r.logger = l
	for _, test := range tests {
		_, out, err := r.removeSecret(context.TODO(), test.in)
		Assert(t, test.err == (err != nil), fmt.Sprintf("got error [%v], [%s] test failed", err, test.name))
		switch obj := out.(type) {
		case *auth.AuthenticationPolicy:
			Assert(t, obj.Spec.Secret == nil, "secret not nil")
		}
	}
}

func TestRemovePasswordHookRegistration(t *testing.T) {
	logConfig := log.GetDefaultConfig("TestAPIGwAuthHooks")
	l := log.GetNewLogger(logConfig)
	svc := mocks.NewFakeAPIGwService(l, false)
	r := &authHooks{}
	r.logger = l
	err := r.registerRemovePasswordHook(svc)
	AssertOk(t, err, "removePassword hook registration failed")

	opers := []apiserver.APIOperType{apiserver.CreateOper, apiserver.UpdateOper, apiserver.DeleteOper, apiserver.GetOper, apiserver.ListOper}
	for _, oper := range opers {
		prof, err := svc.GetCrudServiceProfile("User", oper)
		AssertOk(t, err, fmt.Sprintf("error getting service profile for oper :%v", oper))
		Assert(t, len(prof.PostCallHooks()) == 1, fmt.Sprintf("unexpected number of post call hooks [%d] for User operation [%v]", len(prof.PostCallHooks()), oper))
	}
	// test err
	svc = mocks.NewFakeAPIGwService(l, true)
	err = r.registerRemovePasswordHook(svc)
	Assert(t, err != nil, "expected error in removePassword hook registration")
}

func TestAddRolesHookRegistration(t *testing.T) {
	logConfig := log.GetDefaultConfig("TestAPIGwAuthHooks")
	l := log.GetNewLogger(logConfig)
	svc := mocks.NewFakeAPIGwService(l, false)
	r := &authHooks{}
	r.logger = l
	err := r.registerAddRolesHook(svc)
	AssertOk(t, err, "addRoles hook registration failed")

	opers := []apiserver.APIOperType{apiserver.CreateOper, apiserver.UpdateOper, apiserver.DeleteOper, apiserver.GetOper, apiserver.ListOper}
	for _, oper := range opers {
		prof, err := svc.GetCrudServiceProfile("User", oper)
		AssertOk(t, err, fmt.Sprintf("error getting service profile for oper :%v", oper))
		Assert(t, len(prof.PostCallHooks()) == 1, fmt.Sprintf("unexpected number of post call hooks [%d] for User operation [%v]", len(prof.PostCallHooks()), oper))
	}
	// test err
	svc = mocks.NewFakeAPIGwService(l, true)
	err = r.registerAddRolesHook(svc)
	Assert(t, err != nil, "expected error in addRoles hook registration")
}

func TestRemoveSecretHookRegistration(t *testing.T) {
	logConfig := log.GetDefaultConfig("TestAPIGwAuthHooks")
	l := log.GetNewLogger(logConfig)
	svc := mocks.NewFakeAPIGwService(l, false)
	r := &authHooks{}
	r.logger = l
	err := r.registerRemoveSecretHook(svc)
	AssertOk(t, err, "removeSecret hook registration failed")

	opers := []apiserver.APIOperType{apiserver.CreateOper, apiserver.UpdateOper, apiserver.GetOper}
	for _, oper := range opers {
		prof, err := svc.GetCrudServiceProfile("AuthenticationPolicy", oper)
		AssertOk(t, err, fmt.Sprintf("error getting service profile for oper :%v", oper))
		Assert(t, len(prof.PostCallHooks()) == 1, fmt.Sprintf("unexpected number of post call hooks [%d] for AuthenticationPolicy operation [%v]", len(prof.PostCallHooks()), oper))
	}
	// test err
	svc = mocks.NewFakeAPIGwService(l, true)
	err = r.registerRemoveSecretHook(svc)
	Assert(t, err != nil, "expected error in removeSecret hook registration")
}

func TestAdminRoleCheck(t *testing.T) {
	tests := []struct {
		name     string
		in       interface{}
		out      interface{}
		skipCall bool
		err      error
	}{
		{
			name: "super admin role",
			in: login.NewClusterRole(globals.AdminRole, login.NewPermission(
				authz.ResourceTenantAll,
				authz.ResourceGroupAll,
				auth.Permission_AllResourceKinds.String(),
				authz.ResourceNamespaceAll,
				"",
				auth.Permission_ALL_ACTIONS.String())),
			out: login.NewClusterRole(globals.AdminRole, login.NewPermission(
				authz.ResourceTenantAll,
				authz.ResourceGroupAll,
				auth.Permission_AllResourceKinds.String(),
				authz.ResourceNamespaceAll,
				"",
				auth.Permission_ALL_ACTIONS.String())),
			skipCall: true,
			err:      errors.New("admin role create, update or delete is not allowed"),
		},
		{
			name:     "network admin role",
			in:       testNetworkAdminRole,
			out:      testNetworkAdminRole,
			skipCall: false,
			err:      nil,
		},
		{
			name:     "incorrect object type",
			in:       struct{ name string }{"testing"},
			out:      struct{ name string }{"testing"},
			skipCall: true,
			err:      errors.New("invalid input type"),
		},
	}
	logConfig := log.GetDefaultConfig("TestAPIGwAuthHooks")
	l := log.GetNewLogger(logConfig)
	r := &authHooks{}
	r.logger = l
	for _, test := range tests {
		_, out, skipCall, err := r.adminRoleCheck(context.TODO(), test.in)
		Assert(t, reflect.DeepEqual(err, test.err), fmt.Sprintf("[%s] test failed", test.name))
		Assert(t, skipCall == test.skipCall, fmt.Sprintf("[%s] test failed", test.name))
		Assert(t, reflect.DeepEqual(out, test.out), fmt.Sprintf("[%s] test failed", test.name))
	}
}

func TestAdminRoleCheckHookRegistration(t *testing.T) {
	logConfig := log.GetDefaultConfig("TestAPIGwAuthHooks")
	l := log.GetNewLogger(logConfig)
	svc := mocks.NewFakeAPIGwService(l, false)
	r := &authHooks{}
	r.logger = l
	err := r.registerAdminRoleCheckHook(svc)
	AssertOk(t, err, "adminRoleCheck hook registration failed")

	ids := []serviceID{
		{"Role", apiserver.CreateOper},
		{"Role", apiserver.UpdateOper},
		{"Role", apiserver.DeleteOper},
	}
	for _, id := range ids {
		prof, err := svc.GetCrudServiceProfile(id.kind, id.action)
		AssertOk(t, err, "error getting service profile for [%s] [%s]", id.kind, id.action)
		Assert(t, len(prof.PreCallHooks()) == 1, fmt.Sprintf("unexpected number of pre-call hooks [%d] for [%s] [%s] profile", len(prof.PreCallHooks()), id.kind, id.action))
	}
	// test error
	svc = mocks.NewFakeAPIGwService(l, true)
	err = r.registerAdminRoleCheckHook(svc)
	Assert(t, err != nil, "expected error in adminRoleCheck hook registration")
}

func TestUserCreateCheck(t *testing.T) {
	tests := []struct {
		name       string
		authGetter manager.AuthGetter
		skipCall   bool
		err        error
	}{
		{
			name:       "error in fetching auth policy",
			authGetter: manager.NewMockAuthGetter(nil, true),
			skipCall:   true,
			err:        errors.New("authentication policy not found"),
		},
		{
			name: "local auth disabled",
			authGetter: manager.NewMockAuthGetter(&auth.AuthenticationPolicy{
				TypeMeta: api.TypeMeta{Kind: "AuthenticationPolicy"},
				ObjectMeta: api.ObjectMeta{
					Name: "AuthenticationPolicy",
				},
				Spec: auth.AuthenticationPolicySpec{
					Authenticators: auth.Authenticators{
						Ldap: &auth.Ldap{
							Enabled: true,
						},
						Local: &auth.Local{
							Enabled: false,
						},
						AuthenticatorOrder: []string{auth.Authenticators_LDAP.String(), auth.Authenticators_LOCAL.String()},
					},
				},
			}, false),
			skipCall: true,
			err:      errors.New("local authentication not enabled"),
		},
		{
			name: "local auth enabled",
			authGetter: manager.NewMockAuthGetter(&auth.AuthenticationPolicy{
				TypeMeta: api.TypeMeta{Kind: "AuthenticationPolicy"},
				ObjectMeta: api.ObjectMeta{
					Name: "AuthenticationPolicy",
				},
				Spec: auth.AuthenticationPolicySpec{
					Authenticators: auth.Authenticators{
						Ldap: &auth.Ldap{
							Enabled: true,
						},
						Local: &auth.Local{
							Enabled: true,
						},
						AuthenticatorOrder: []string{auth.Authenticators_LDAP.String(), auth.Authenticators_LOCAL.String()},
					},
				},
			}, false),
			skipCall: false,
			err:      nil,
		},
	}
	logConfig := log.GetDefaultConfig("TestAPIGwAuthHooks")
	l := log.GetNewLogger(logConfig)
	r := &authHooks{}
	r.logger = l
	for _, test := range tests {
		r.authGetter = test.authGetter
		_, _, skipCall, err := r.userCreateCheck(context.TODO(), &auth.User{})
		Assert(t, skipCall == test.skipCall, fmt.Sprintf("expected skipCall [%v], got [%v], [%s] test failed", test.skipCall, skipCall, test.name))
		Assert(t, reflect.DeepEqual(err, test.err), fmt.Sprintf("expected err [%v], got [%v], %s] test failed", test.err, err, test.name))
	}
}

func TestUserCreateCheckHookRegistration(t *testing.T) {
	logConfig := log.GetDefaultConfig("TestAPIGwAuthHooks")
	l := log.GetNewLogger(logConfig)
	svc := mocks.NewFakeAPIGwService(l, false)
	r := &authHooks{}
	r.logger = l
	err := r.registerUserCreateCheckHook(svc)
	AssertOk(t, err, "userCreateCheck hook registration failed")

	ids := []serviceID{
		{"User", apiserver.CreateOper},
	}
	for _, id := range ids {
		prof, err := svc.GetCrudServiceProfile(id.kind, id.action)
		AssertOk(t, err, "error getting service profile for [%s] [%s]", id.kind, id.action)
		Assert(t, len(prof.PreCallHooks()) == 1, fmt.Sprintf("unexpected number of pre-call hooks [%d] for [%s] [%s] profile", len(prof.PreCallHooks()), id.kind, id.action))
	}
	// test error
	svc = mocks.NewFakeAPIGwService(l, true)
	err = r.registerUserCreateCheckHook(svc)
	Assert(t, err != nil, "expected error in userCreateCheck hook registration")
}

func TestAuthBootstrap(t *testing.T) {
	tests := []struct {
		name         string
		in           interface{}
		bootstrapper bootstrapper.Bootstrapper
		skipAuth     bool
		err          error
	}{
		{
			name: "non default user tenant",
			in: &auth.User{
				TypeMeta: api.TypeMeta{Kind: "User"},
				ObjectMeta: api.ObjectMeta{
					Tenant: "testTenant",
					Name:   "testuser",
				},
			},
			bootstrapper: bootstrapper.NewMockBootstrapper(false),
			skipAuth:     false,
			err:          nil,
		},
		{
			name: "non default role binding tenant",
			in: &auth.RoleBinding{
				TypeMeta: api.TypeMeta{Kind: "RoleBinding"},
				ObjectMeta: api.ObjectMeta{
					Tenant: "testTenant",
					Name:   "AdminRoleBinding",
				},
			},
			bootstrapper: bootstrapper.NewMockBootstrapper(false),
			skipAuth:     false,
			err:          nil,
		},
		{
			name:         "invalid input type",
			in:           struct{ name string }{"testing"},
			bootstrapper: bootstrapper.NewMockBootstrapper(false),
			skipAuth:     false,
			err:          errors.New("invalid input type"),
		},
		{
			name: "auth policy",
			in: &auth.AuthenticationPolicy{
				TypeMeta: api.TypeMeta{Kind: "AuthenticationPolicy"},
				ObjectMeta: api.ObjectMeta{
					Name: "AuthenticationPolicy",
				},
			},
			bootstrapper: bootstrapper.NewMockBootstrapper(false),
			skipAuth:     false,
			err:          nil,
		},
	}
	logConfig := log.GetDefaultConfig("TestAPIGwAuthHooks")
	l := log.GetNewLogger(logConfig)
	r := &authHooks{}
	r.logger = l
	for _, test := range tests {
		r.bootstrapper = test.bootstrapper
		_, _, skipAuth, err := r.authBootstrap(context.TODO(), test.in)
		Assert(t, skipAuth == test.skipAuth, fmt.Sprintf("[%s] test failed, expected skipAuth [%v], got [%v]", test.name, test.skipAuth, skipAuth))
		Assert(t, reflect.DeepEqual(err, test.err), fmt.Sprintf("[%s] test failed, expected err [%v], got [%v]", test.name, test.err, err))
	}

}
